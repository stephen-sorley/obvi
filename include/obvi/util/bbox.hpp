/* Header-only class that implements an axis-aligned bounding-box in 3D space.
 *
 * The edges of the bounding box are inclusive - a point lying on the surface of the bounding
 * box is considered to be inside it.
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
#ifndef OBVI_BBOX_HPP
#define OBVI_BBOX_HPP

#include <cmath>
#include <algorithm> // for min and max
#include <limits>
#include <obvi/util/vec3.hpp>

namespace obvi {

template<typename real>
struct bbox {
    vec3<real> min_pt;
    vec3<real> max_pt;

    // Bbox is initialized "empty", which means that no intersection tests will succeed
    // against it. This is indicated by setting min_pt.x > max_pt.x.
    bbox() : min_pt(1,0,0), max_pt(-1,0,0) {}

    explicit bbox(const vec3<real>& pt) : min_pt(pt), max_pt(pt) {}

    bool is_empty() const {
        return min_pt.x() > max_pt.x();
    }

    void clear() {
        min_pt.set(1,0,0);
        max_pt.set(-1,0,0);
    }

    // Calculate the center of the bounding box.
    vec3<real> center() {
        return (min_pt + max_pt) * real(0.5);
    }

    // Expand the bounding box to include the given point.
    void expand(const vec3<real>& pt) {
        if(is_empty()) {
            min_pt = pt;
            max_pt = pt;
        } else {
            for(size_t i=0; i<3; ++i) {
                min_pt[i] = std::min(min_pt[i], pt[i]);
                max_pt[i] = std::max(max_pt[i], pt[i]);
            }
        }
    }

    // Expand the bounding box to include the given bounding box.
    void expand(const bbox& box) {
        if(is_empty()) {
            min_pt = box.min_pt;
            max_pt = box.max_pt;
        } else {
            for(size_t i=0; i<3; ++i) {
                min_pt[i] = std::min(min_pt[i], box.min_pt[i]);
                max_pt[i] = std::max(max_pt[i], box.max_pt[i]);
            }
        }
    }

    // point <-> bbox intersection
    friend bool intersects(const bbox& box, const vec3<real>& pt) {
        return pt.x() >= box.min_pt.x() && pt.x() <= box.max_pt.x()
            && pt.y() >= box.min_pt.y() && pt.y() <= box.max_pt.y()
            && pt.z() >= box.min_pt.z() && pt.z() <= box.max_pt.z();
    }

    // bbox <-> bbox intersection
    friend bool intersects(const bbox& box1, const bbox& box2) {
        if(box1.is_empty() || box2.is_empty()) {
            return false;
        }
        return (box1.min_pt.x() <= box2.max_pt.x() && box1.max_pt.x() >= box2.min_pt.x())
            && (box1.min_pt.y() <= box2.max_pt.y() && box1.max_pt.y() >= box2.min_pt.y())
            && (box1.min_pt.z() <= box2.max_pt.z() && box1.max_pt.z() >= box2.min_pt.z());
    }

    // segment <-> bbox intersection
    //
    // Works using the Separating Axis Theorum (SAT), see the following for details:
    // https://www.gamedev.net/forums/topic/338987-aabb---line-segment-intersection-test/?do=findComment&comment=3209917
    friend bool intersects(const bbox& box, const vec3<real>& segA, const vec3<real>& segB) {
        if(box.is_empty()) {
            return false;
        }

        vec3<real> d  = (segB - segA) * real(0.5);
        vec3<real> e  = (box.max_pt - box.min_pt) * real(0.5);
        vec3<real> c  = segA + d - (box.max_pt + box.min_pt) * real(0.5);
        vec3<real> ad = d.abs();

        if(std::abs(c.x()) > e.x() + ad.x()) {
            return false;
        }
        if(std::abs(c.y()) > e.y() + ad.y()) {
            return false;
        }
        if(std::abs(c.z()) > e.z() + ad.z()) {
            return false;
        }

        real eps = std::numeric_limits<real>::epsilon();
        if(std::abs(d.y()*c.z() - d.z()*c.y()) > e.y()*ad.z() + e.z()*ad.y() + eps) {
            return false;
        }
        if(std::abs(d.z()*c.x() - d.x()*c.z()) > e.z()*ad.x() + e.x()*ad.z() + eps) {
            return false;
        }
        if(std::abs(d.x()*c.y() - d.y()*c.x()) > e.x()*ad.y() + e.y()*ad.x() + eps) {
            return false;
        }

        return true;
    }

    // ray <-> bbox intersection
    //
    // origin: origin of ray
    // inv_dir: element-wise inverse (reciprocal) of normalized ray direction vector
    //
    // Algorithm taken from the following source (Tavian Barnes, 3/23/2015, public domain):
    //   https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
    friend bool intersects(const bbox& box, const vec3<real>& origin, const vec3<real>& inv_dir) {
        if(box.is_empty()) {
            return false;
        }

        real t1 = (box.min_pt[0] - origin[0]) * inv_dir[0];
        real t2 = (box.max_pt[0] - origin[0]) * inv_dir[0];

        real tmin = std::min(t1, t2);
        real tmax = std::max(t1, t2);

        for(int i = 1; i < 3; ++i) {
            t1 = (box.min_pt[i] - origin[i]) * inv_dir[i];
            t2 = (box.max_pt[i] - origin[i]) * inv_dir[i];

            tmin = std::max(tmin, std::min(std::min(t1, t2), tmax));
            tmax = std::min(tmax, std::max(std::max(t1, t2), tmin));
        }

        return tmax > std::max(tmin, real(0));
    }
};

using bboxf = bbox<float>;
using bboxd = bbox<double>;

} // END namespace obvi
#endif // OBVI_BBOX_HPP
