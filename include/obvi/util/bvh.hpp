/* Public header for class that implements a bounding-volume hierarchy (BVH).
 *
 * A BVH is a datastructure used to accelerate various kinds of queries against a 3D object.
 * Use cases are things like finding the closest triangle in a mesh that intersects with the
 * given ray, finding the closest object to a given point, etc.
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
    static constexpr size_t max_size = 1 << 30;

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

    // internal bvh node.
    struct node {
        bboxf    box;
        uint32_t num; // high bit == 0: 31 low bits are num nodes in subtree with this node as root.
                      // high bit == 1: this is a leaf node, and 31 low bits are index of object.
    }; // 28 bytes

private:
    std::vector<node> tree; // BVH tree, stored linearly in depth-first-traversal order
    size_t            num_leaves = 0;
};

} // END namespace obvi
#endif // OBVI_BVH_HPP
