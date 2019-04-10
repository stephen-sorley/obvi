/* Public header for class that implements a bounding-volume hierarchy (BVH).
 *
 * A BVH is a datastructure used to accelerate various kinds of queries against a 3D object.
 * Use cases are things like finding the closest triangle in a mesh that intersects with the
 * given ray, finding the closest object to a given point, etc.
 *
 * Usage example:
 * \code
 * std::vector<obvi::bboxf> bboxes; // list of bounding boxes, one for each object to place in BVH.
 * // ... fill bboxes ...
 *
 * obvi::bvh bvh;
 * bvh.generate(bboxes);
 *
 * auto pt_query = bvh.make_query(bvh::intersect_point(vec3f(1.0f, 2.5f, 1.2f)));
 * size_t obj_idx;
 * while(pt_query.next(&obj_idx)) {
 *     printf("intersection: box# %zu\n", obj_idx);
 * }
 *
 * // Do another point query on the same bvh, using a different point.
 * pt_query.reset(bvh::intersect_point(vec3f(1.0f, 2.5f, 1.2f)));
 * while(pt_query.next(&obj_idx)) {
 *     printf("intersection: box# %zu\n", obj_idx);
 * }
 * \endcode
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
#ifndef OBVI_BVH_HPP
#define OBVI_BVH_HPP

#include <vector>

#include <obvi/util/math.hpp>
#include <obvi/util/vec3.hpp>
#include <obvi/util/bbox.hpp>

namespace obvi {

struct bvh {
    //max number of objects in BVH is 2^30, because number of BVH nodes is (2*num_leaves-1), and
    //the number of nodes must fit in a 31-bit unsigned integer.
    static constexpr size_t max_size = 1u << 30;

    void clear() {
        tree.clear();
        num_leaves = 0;
    }

    /* Create a new BVH from the given list of object bounding boxes.
     *
     * Any previously-generated tree data will be wiped first.
     *
     * Returns 'false' if there are too many boxes (i.e., resulting tree would exhaust
     * the index space). This won't occur unless you try to make a BVH with more than
     * ~1 billion (2^30) objects in a single tree.
     */
    bool generate(const std::vector<bboxf>& boxes);

    size_t size() const {
        return num_leaves;
    }

    template<typename intersect_func> struct query; //defined at bottom of file

    // Make an intersection query.
    template<typename intersect_func>
    query<intersect_func> make_query(intersect_func ifunc) const {
        return query<intersect_func>(*this, ifunc);
    }

    // internal bvh node.
    struct node {
        bboxf    box;
        uint32_t num; // high bit == 0: 31 low bits are num nodes in subtree with this node as root.
                      // high bit == 1: this is a leaf node, and 31 low bits are index of object.
        // True if node is a leaf node, false if it has children.
        bool is_leaf() const { return num > 0x7FFFFFFF; }
        // Number of nodes in this subtree (this node + all children).
        size_t subtree_size() const { return (is_leaf())? (size_t)1 : (size_t)num; }
    }; // 28 bytes


    // intersection functors.
    struct intersect_point {
        vec3f point;
        intersect_point(const vec3f& pt) : point(pt) {}
        bool operator()(const bboxf& box) {
            return box.intersects_point(point);
        }
    };
    struct intersect_box {
        bboxf qbox;
        intersect_box(const bboxf& bx) : qbox(bx) {}
        bool operator()(const bboxf& box) {
            return box.intersects_box(qbox);
        }
    };
    struct intersect_segment {
        vec3f d;
        vec3f seg_a_d;
        vec3f ad;
        intersect_segment(const vec3f& seg_a, const vec3f& seg_b) {
            d       = (seg_b - seg_a) * 0.5f;
            seg_a_d = seg_a + d;
            ad      = d.abs();
        }
        bool operator()(const bboxf& box) {
            return box.intersects_segment_precalc(d, seg_a_d, ad);
        }
    };
    struct intersect_ray {
        vec3f origin;
        vec3f inv_norm_dir;
        intersect_ray(const vec3f& ray_origin, const vec3f &ray_norm_dir)
            : origin(ray_origin), inv_norm_dir(ray_norm_dir.inv()) {}
        bool operator()(const bboxf& box) {
            return box.intersects_ray(origin, inv_norm_dir);
        }
    };

private:
    std::vector<node> tree; // BVH tree, stored linearly in depth-first-traversal order
    size_t            num_leaves = 0;
};


/* Iterator that conducts a BVH intersection query.
 *
 * Multiple iterators can be used on the same BVH in parallel (thread safe to reads).
 * However, it is not safe to modify the BVH while iterators that point to it are
 * being used.
 *
 * intersect_func:
 *   Functor that accepts a bboxf as an argument, and returns a bool
 *   indicating whether your object intersects that bbox or not.
 *        bool intersect_func(const bboxf& box);
 */
template<typename intersect_func>
struct bvh::query {
    query(const bvh& targ, intersect_func ifunc)
        : next_node(0), tree(targ.tree), intersects(ifunc) {}

    void reset() { next_node = 0; }

    void reset(intersect_func ifunc) { reset(); intersects = ifunc; }

    /* Return the index of the next object whose bounding box was intersected by the query.
     * If no additional objects were found, returns -1.
     *
     * The returned index corresponds to the bounding box's location in the vector of bounding
     * boxes that was passed to generate() when this BVH was created.
     */
    bool next(size_t *out_match) {
        while(next_node < tree.size()) {
            const node& nd = tree[next_node];
            if(intersects(nd.box)) {
                // Set next_node to the next node in a depth-first traversal of the tree.
                next_node++;
                if(nd.is_leaf()) {
                    if(out_match) {
                        *out_match = (size_t)(nd.num & 0x7FFFFFFFu);
                    }
                    return true;
                }
            } else {
                // If we didn't find an intersection with this node, skip past the entire subtree.
                next_node += nd.subtree_size();
            }
        }
        return false;
    }

private:
    size_t                   next_node;
    const std::vector<node>& tree;
    intersect_func           intersects;
};

} // END namespace obvi
#endif // OBVI_BVH_HPP
