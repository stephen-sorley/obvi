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
    using vec3r = vec3<real>;

    vec3r min_pt;
    vec3r max_pt;

    // Bbox is initialized "empty", which means that no intersection tests will succeed
    // against it. This is indicated by setting min_pt.x > max_pt.x.
    bbox() : min_pt(1,0,0), max_pt(-1,0,0) {}

    bbox(const real& xmin, const real& ymin, const real& zmin,
         const real& xmax, const real& ymax, const real& zmax)
        : min_pt(xmin,ymin,zmin), max_pt(xmax,ymax,zmax) {}

    explicit bbox(const vec3r& pt) : min_pt(pt), max_pt(pt) {}

    bool is_empty() const {
        return min_pt.x() > max_pt.x();
    }

    void clear() {
        min_pt.set(1,0,0);
        max_pt.set(-1,0,0);
    }

    // Calculate the center of the bounding box.
    vec3r center() const {
        return (min_pt + max_pt) * real(0.5);
    }

    // Expand the bounding box to include the given point.
    void expand(const vec3r& pt) {
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
    bool intersects_point(const vec3r& pt) const {
        return pt.x() >= min_pt.x() && pt.x() <= max_pt.x()
            && pt.y() >= min_pt.y() && pt.y() <= max_pt.y()
            && pt.z() >= min_pt.z() && pt.z() <= max_pt.z();
    }

    // bbox <-> bbox intersection
    bool intersects_box(const bbox& box) const {
        if(is_empty() || box.is_empty()) {
            return false;
        }
        return (min_pt.x() <= box.max_pt.x() && max_pt.x() >= box.min_pt.x())
            && (min_pt.y() <= box.max_pt.y() && max_pt.y() >= box.min_pt.y())
            && (min_pt.z() <= box.max_pt.z() && max_pt.z() >= box.min_pt.z());
    }

    // segment <-> bbox intersection
    //
    // Works using the Separating Axis Theorum (SAT), see the following for details:
    // https://www.gamedev.net/forums/topic/338987-aabb---line-segment-intersection-test/?do=findComment&comment=3209917
    bool intersects_segment(const vec3r& seg_a, const vec3r& seg_b) const {
        if(is_empty()) {
            return false;
        }
        vec3r d = (seg_b - seg_a) * real(0.5);
        return intersects_segment_precalc(d, seg_a + d, d.abs());
    }
    bool intersects_segment_precalc(const vec3r &d, const vec3r &seg_a_d, const vec3r &ad) const {
        vec3r e = (max_pt - min_pt) * real(0.5);
        vec3r c = seg_a_d - (max_pt + min_pt) * real(0.5);

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
    bool intersects_ray(const vec3r& origin, const vec3r& inv_norm_dir) const {
        if(is_empty()) {
            return false;
        }

        // Naive algorithm, adapted from the following source (Andrew Kensler, public domain):
        //   psgraphics.blogspot.com/2016/02/new-simple-ray-box-test-from-andrew.html
        //
        // Kinda slow, but is definitely correct in all edge cases.
        //
        // TODO: figure out how to make this faster, while still detecting intersections with
        //       infinitely thin planes.
        real tmax = std::numeric_limits<real>::infinity();
        real tmin = -tmax;
        for(size_t i=0; i<3; ++i) {
            if(!std::isinf(inv_norm_dir[i])) { // Avoid NaN's from 0 * INF
                real t0 = (min_pt[i] - origin[i]) * inv_norm_dir[i];
                real t1 = (max_pt[i] - origin[i]) * inv_norm_dir[i];

                tmin = std::max(tmin, std::min(t0, t1));
                tmax = std::min(tmax, std::max(t0, t1));
            } else if (origin[i] < min_pt[i] || origin[i] > max_pt[i]) {
                return false;
            }
        }
        //static constexpr real epsfac = real(1) + real(2) * std::numeric_limits<real>::epsilon();
        //tmax *= epsfac;
        return tmax >= tmin && tmax >= real(0);
    }
};

using bboxf = bbox<float>;
using bboxd = bbox<double>;

} // END namespace obvi
#endif // OBVI_BBOX_HPP
