/* Main implementation of class that implements a bounding-volume hierarchy (BVH).
 *
 * Large portions of the BVH generation routine were originally based on public-domain NVIDIA code
 * found here:
 *   https://devblogs.nvidia.com/thinking-parallel-part-iii-tree-construction-gpu/
 *
 * OpenMP 2.0 parallel radix sort implementation was adapted from public-domain code posted here:
 *   https://haichuanwang.wordpress.com/2014/05/26/a-faster-openmp-radix-sort-implementation
 *
 * * * * * * * * * * * *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Stephen Sorley
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * * * * * * * * * * * *
 */
#include <obvi/util/bvh.hpp>
#include <obvi/util/compat_omp.hpp>
#include <obvi/util/math.hpp>

using obvi::bboxf;
using obvi::vec3f;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Helper code - only visible inside this file.
namespace {
    struct obj {
        uint32_t code = 0;
        uint32_t idx  = 0;
    };

    // Parallel radix sort code adapted from here:
    //    https://haichuanwang.wordpress.com/2014/05/26/a-faster-openmp-radix-sort-implementation
    //
    // TODO: generalize the parallel_radix_sort function so we can pass in a comparator, and
    //       template it to allow use with 64-bit values.
    uint32_t digits(const uint32_t v, const int shift, const uint32_t mask) {
        return (v >> shift) & mask;
    }
    void parallel_radix_sort(std::vector<obj>& objs) {
        constexpr int      base_bits = 8;
        constexpr int      base      = 1 << base_bits;
        constexpr uint32_t mask      = base - 1;

        std::vector<obj> buffer;

        int total_digits = (int)sizeof(uint32_t)*8;
        int nobjs        = (int)objs.size();
        int i; //loop indices must be 'int' for old OpenMP (v2.5) in Visual Studio.

        //Each thread use local_bucket to move data
        for(int shift = 0; shift < total_digits; shift+=base_bits) {
            size_t bucket[base] = {0};

            size_t local_bucket[base] = {0}; // size needed in each bucket/thread
            //1st pass, scan whole and check the count
#               pragma omp parallel firstprivate(local_bucket)
            {
#               pragma omp for schedule(static) nowait
                for(i = 0; i < nobjs; i++){
                    local_bucket[digits(objs[(size_t)i].code, shift, mask)]++;
                }
#               pragma omp critical
                for(i = 0; i < base; i++) {
                    bucket[i] += local_bucket[i];
                }
#               pragma omp barrier
#               pragma omp single
                for(i = 1; i < base; i++) {
                    bucket[i] += bucket[i - 1];
                }
                int nthreads = omp_get_num_threads();
                int tid = omp_get_thread_num();
                for(int cur_t = nthreads - 1; cur_t >= 0; cur_t--) {
                    if(cur_t == tid) {
                        for(i = 0; i < base; i++) {
                            bucket[i] -= local_bucket[i];
                            local_bucket[i] = bucket[i];
                        }
                    } else { //just do barrier
#                       pragma omp barrier
                    }

                }
#               pragma omp for schedule(static)
                for(i = 0; i < nobjs; i++) { //note here the end condition
                    buffer[local_bucket[digits(objs[(size_t)i].code, shift, mask)]++]
                        = objs[(size_t)i];
                }
            }
            //now move data from buffer back into original vector.
            std::swap(objs, buffer);
        }
    }

    // Make list of morton code and index for each bounding box, then sort it.
    void make_obj_list(std::vector<obj>& objs, const std::vector<bboxf> &boxes, const bboxf& root_box) {
        objs.resize(boxes.size());

        // Multiplier used when converting bbox centroids to lie in range [0,1024) for x, y, and z.
        vec3f mult = 1024.0f / (root_box.max_pt - root_box.min_pt);

        // Compute morton codes for center of each box, store in obj list along with object's index
        // in the boxes array.
#       pragma omp parallel for // loop idx must be 'int' for old OpenMP (v2.5) in Visual Studio.
        for(int i=0; i<(int)boxes.size(); ++i) {
            // Get center of bounding box, transform so that it lies on range [0,1024] for all dims.
            vec3f center = (boxes[(size_t)i].center() - root_box.min_pt) * mult;
            // Set morton code of box center and index, in objs array.
            objs[(size_t)i].code = obvi::morton_encode_30(center.x(), center.y(), center.z());
            objs[(size_t)i].idx  = (uint32_t)i;
        }

        // Sort the objects list in morton-code order.
        //std::sort(objs.begin(), objs.end(), [](const obj& a, const obj& b){ return a.code < b.code; });
        parallel_radix_sort(objs); //should be faster
    }

    // Use binary search to find the index of the object where the highest non-common bit
    // in the morton code changes from 0 to 1.
    //
    // Returns the index of the last element where that bit is zero. split+1 is the first element
    // that has 1 in that bit.
    size_t find_split(const std::vector<obj> &objs, size_t first, size_t last) {
        uint32_t first_code = objs[first].code;
        uint32_t last_code = objs[last].code;

        // Identical Morton codes => split the range in the middle.
        // Remember, Morton codes are buckets in real space - you can have multiple objects
        // in the same bucket.
        if(first_code == last_code) {
            return (first + last) / 2;
        }

        // Calculate the number of highest bits that are the same
        // for all objects, using the count-leading-zeros intrinsic.

        uint32_t common_prefix = obvi::count_leading_zeros(first_code ^ last_code);

        // Use binary search to find where the next bit differs.
        // Specifically, we are looking for the highest object that
        // shares more than common_prefix bits with the first one.

        size_t split = first; // initial guess
        size_t step = last - first;

        do {
            step = (step + 1) / 2; // exponential decrease in step size
            size_t newSplit = split + step; // proposed new position

            if(newSplit < last) {
                uint32_t split_prefix = obvi::count_leading_zeros(first_code ^ objs[newSplit].code);
                if (split_prefix > common_prefix) {
                    split = newSplit; // accept proposal
                }
            }
        } while(step > 1);

        return split;
    }

    // Recursively generate the BVH. Note that we're storing the BVH linearly in memory,
    // in depth-first traversal order. So, we don't have to keep track of what level we're
    // on in the tree when constructing it, we just build it in depth-first order too.
    void generate(std::vector<obvi::bvh::node> &tree,
                  const std::vector<bboxf>& boxes, const std::vector<obj>& objs,
                  bboxf& curr_box, size_t first, size_t last) {
        if(first == last) {
            // Single object => add a new leaf node. Need to set top bit to 1 to mark as leaf.
            tree.push_back({curr_box, (1u<<31) | objs[first].idx});
            return;
        }
        // Multiple objects => add a new internal node. Leave top bit set to 0.
        tree.push_back({curr_box, (uint32_t)(last - first + 1)});

        // Determine where to split the range.
        size_t split = find_split(objs, first, last);

        // Process left children, add nodes to tree.
        curr_box.clear();
        for(size_t i=first; i<=split; ++i) {
            curr_box.expand(boxes[objs[i].idx]);
        }
        generate(tree, boxes, objs, curr_box, first, split);

        // Process right children, add nodes to tree.
        curr_box.clear();
        for(size_t i=split+1; i<=last; ++i) {
            curr_box.expand(boxes[objs[i].idx]);
        }
        generate(tree, boxes, objs, curr_box, split + 1, last);
    }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementations of public API functions.
bool obvi::bvh::generate(const std::vector<bboxf>& boxes) {
    clear();

    if(boxes.size() > max_size) {
        return false;
    }

    if(boxes.size()==0) {
        return true;
    }

    // Preallocate memory for BVH. Number of nodes = 2 * (number of leaves) - 1.
    tree.reserve(2 * boxes.size() - 1);

    // Get bounding box that covers all individual boxes in scene.
    bboxf root_box;
    for(auto &box : boxes) {
        root_box.expand(box);
    }

    // Get sorted list of morton codes and obj indexes for each bounding box.
    std::vector<obj> objs;
    make_obj_list(objs, boxes, root_box);

    // Generate BVH.
    ::generate(tree, boxes, objs, root_box, 0, boxes.size() - 1);

    return true;
}
