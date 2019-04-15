/* Unit tests for bbox (util library).
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

#include <catch2/catch.hpp>
#include <obvi/util/bbox.hpp>
#include <obvi/util/vec3.hpp>

using obvi::vec3;
using obvi::bbox;
using namespace Catch::literals; // To get "_a" UDL for approximate floating-point values.

#define VEC3_EQUAL(v, v0, v1, v2) \
    REQUIRE( v.pt[0] == v0 ); \
    REQUIRE( v.pt[1] == v1 ); \
    REQUIRE( v.pt[2] == v2 );


TEMPLATE_TEST_CASE("bbox creation", "[bbox]", float, double) {
    using T     = TestType;
    using vec3t = vec3<T>;
    using bboxt = bbox<T>;

    bboxt box, other;

    REQUIRE( box.is_empty() );

    SECTION( "construct from point" ) {
        box = bboxt(vec3t(1,2,3));
        VEC3_EQUAL(box.min_pt, 1,2,3);
        VEC3_EQUAL(box.max_pt, 1,2,3);
        REQUIRE_FALSE( box.is_empty() );
    }

    SECTION( "construct from components" ) {
        box = bboxt(1,2,3,4,5,6);
        VEC3_EQUAL(box.min_pt, 1,2,3);
        VEC3_EQUAL(box.max_pt, 4,5,6);
        REQUIRE_FALSE( box.is_empty() );
    }

    SECTION( "clear" ) {
        box.min_pt.set(1,2,3);
        box.max_pt.set(4,5,6);
        REQUIRE_FALSE( box.is_empty() );

        box.clear();
        REQUIRE( box.is_empty() );
    }

    SECTION( "expand by point" ) {
        box.expand(vec3t(1,2,3));
        VEC3_EQUAL(box.min_pt, 1,2,3);
        VEC3_EQUAL(box.max_pt, 1,2,3);
        REQUIRE_FALSE( box.is_empty() );

        box.expand(vec3t(4,5,6));
        VEC3_EQUAL(box.min_pt, 1,2,3);
        VEC3_EQUAL(box.max_pt, 4,5,6);
        REQUIRE_FALSE( box.is_empty() );

        box.expand(vec3t(-2,-3,-4));
        VEC3_EQUAL(box.min_pt, -2,-3,-4);
        VEC3_EQUAL(box.max_pt, 4,5,6);

        box.expand(vec3t(2,3,4));
        VEC3_EQUAL(box.min_pt, -2,-3,-4);
        VEC3_EQUAL(box.max_pt, 4,5,6);

        box.expand(vec3t(-2,-3,-4));
        VEC3_EQUAL(box.min_pt, -2,-3,-4);
        VEC3_EQUAL(box.max_pt, 4,5,6);

        box.expand(vec3t(4,5,6));
        VEC3_EQUAL(box.min_pt, -2,-3,-4);
        VEC3_EQUAL(box.max_pt, 4,5,6);
    }

    SECTION( "expand by box" ) {
        // Expand by box from empty.
        box.expand(bboxt(1,2,3, 4,5,6));
        VEC3_EQUAL(box.min_pt, 1,2,3);
        VEC3_EQUAL(box.max_pt, 4,5,6);

        // Expand by box from point.
        box = bboxt(vec3t(7,8,9));
        box.expand(bboxt(1,2,3, 4,5,6));
        VEC3_EQUAL(box.min_pt, 1,2,3);
        VEC3_EQUAL(box.max_pt, 7,8,9);

        // Expand by box from box.
        box.expand(bboxt(2,3,4, 10,11,12));
        VEC3_EQUAL(box.min_pt, 1,2,3);
        VEC3_EQUAL(box.max_pt, 10,11,12);

        box.expand(bboxt(-2,-1,0, 9,10,11));
        VEC3_EQUAL(box.min_pt, -2,-1,0);
        VEC3_EQUAL(box.max_pt, 10,11,12);

        box.expand(bboxt(-3,-4,-5, 15,18,16));
        VEC3_EQUAL(box.min_pt, -3,-4,-5);
        VEC3_EQUAL(box.max_pt, 15,18,16);

        box.expand(bboxt(-1,-2,-3, 13,12,11));
        VEC3_EQUAL(box.min_pt, -3,-4,-5);
        VEC3_EQUAL(box.max_pt, 15,18,16);
    }

    SECTION( "calculate box center" ) {
        vec3<T> center;

        box.min_pt.set(1,2,3);
        box.max_pt.set(1,2,3);
        center = box.center();
        VEC3_EQUAL(center, 1.0_a, 2.0_a, 3.0_a);

        box.max_pt.set(4,6,7);
        center = box.center();
        VEC3_EQUAL(center, 2.5_a, 4.0_a, 5.0_a);
    }
}

TEMPLATE_TEST_CASE("bbox intersection", "[bbox]", float, double) {
    using T     = TestType;
    using vec3t = vec3<T>;
    using bboxt = bbox<T>;

    bboxt box(1,2,3, 4,5,6);

    SECTION( "bbox/point intersection" ) {
        // Corners
        REQUIRE( box.intersects_point(vec3t(1,2,3)) );
        REQUIRE( box.intersects_point(vec3t(4,2,3)) );
        REQUIRE( box.intersects_point(vec3t(1,5,3)) );
        REQUIRE( box.intersects_point(vec3t(4,5,3)) );
        REQUIRE( box.intersects_point(vec3t(1,2,6)) );
        REQUIRE( box.intersects_point(vec3t(4,2,6)) );
        REQUIRE( box.intersects_point(vec3t(1,5,6)) );
        REQUIRE( box.intersects_point(vec3t(4,5,6)) );
        // Center of box
        REQUIRE( box.intersects_point(vec3t(T(2.5),4,4)) );

        // Outside box (X).
        REQUIRE_FALSE( box.intersects_point(vec3t(0,2,3)) );
        REQUIRE_FALSE( box.intersects_point(vec3t(5,2,6)) );
        // Outside box (Y).
        REQUIRE_FALSE( box.intersects_point(vec3t(4,1,6)) );
        REQUIRE_FALSE( box.intersects_point(vec3t(1,6,3)) );
        // Outside box (Z).
        REQUIRE_FALSE( box.intersects_point(vec3t(4,5,2)) );
        REQUIRE_FALSE( box.intersects_point(vec3t(1,2,7)) );
    }

    SECTION( "bbox/bbox intersection" ) {
        // Exact same box.
        REQUIRE( box.intersects_box(box) );

        // Point box at center.
        REQUIRE( box.intersects_box(bboxt(box.center())) );

        // Point-box on each corner.
        REQUIRE( box.intersects_box(bboxt(1,2,3, 1,2,3)));
        REQUIRE( box.intersects_box(bboxt(4,2,3, 4,2,3)));
        REQUIRE( box.intersects_box(bboxt(1,5,3, 1,5,3)));
        REQUIRE( box.intersects_box(bboxt(4,5,3, 4,5,3)));
        REQUIRE( box.intersects_box(bboxt(1,2,6, 1,2,6)));
        REQUIRE( box.intersects_box(bboxt(4,2,6, 4,2,6)));
        REQUIRE( box.intersects_box(bboxt(1,5,6, 1,5,6)));
        REQUIRE( box.intersects_box(bboxt(4,5,6, 4,5,6)));

        // Only share a corner (box with volume).
        REQUIRE( box.intersects_box(bboxt(0,1,2, 1,2,3)) );
        REQUIRE( box.intersects_box(bboxt(4,1,2, 5,2,3)) );
        REQUIRE( box.intersects_box(bboxt(4,5,2, 5,6,3)) );
        REQUIRE( box.intersects_box(bboxt(0,5,2, 1,6,3)) );
        REQUIRE( box.intersects_box(bboxt(0,1,6, 1,2,7)) );
        REQUIRE( box.intersects_box(bboxt(4,1,5, 5,2,6)) );
        REQUIRE( box.intersects_box(bboxt(4,5,6, 5,6,7)) );
        REQUIRE( box.intersects_box(bboxt(0,5,6, 1,6,7)) );

        // Box2 completely surrounds Box1.
        REQUIRE( box.intersects_box(bboxt(0,1,2, 5,6,7)) );

        // Box2 completely inside Box1.
        REQUIRE( box.intersects_box(bboxt(2,3,4, 3,4,5)) );

        // Half inside, half outside.
        REQUIRE( box.intersects_box(bboxt(-0.5,3,4, 2.5,4,5)) ); // -X
        REQUIRE( box.intersects_box(bboxt(2.5,3,4, 5.5,4,5)) );  // +X
        REQUIRE( box.intersects_box(bboxt(2,0.5,4, 3,3.5,5)) );  // -Y
        REQUIRE( box.intersects_box(bboxt(2,3.5,4, 3,6.5,5)) );  // +Y
        REQUIRE( box.intersects_box(bboxt(2,3,1.5, 3,4,4.5)) );  // -Z
        REQUIRE( box.intersects_box(bboxt(2,3,4.5, 3,4,7.5)) );  // +Z

        // Just outside.
        REQUIRE_FALSE( box.intersects_box(bboxt(-3.5,3,4, -0.5,4,5)) ); // -X
        REQUIRE_FALSE( box.intersects_box(bboxt(5.5,3,4, 8.5,4,5)) );   // +X
        REQUIRE_FALSE( box.intersects_box(bboxt(2,-2.5,4, 3,0.5,5)) );  // -Y
        REQUIRE_FALSE( box.intersects_box(bboxt(2,6.5,4, 3,9.5,5)) );   // +Y
        REQUIRE_FALSE( box.intersects_box(bboxt(2,3,-1.5, 3,4,1.5)) );  // -Z
        REQUIRE_FALSE( box.intersects_box(bboxt(2,3,7.5, 3,4,10.5)) );  // +Z

        // Empty box.
        REQUIRE_FALSE( box.intersects_box(bboxt()) );                 // box2 empty
        REQUIRE_FALSE( bboxt().intersects_box(bboxt(1,2,3, 4,5,6)) ); // box1 empty
        REQUIRE_FALSE( bboxt().intersects_box(bboxt()) );             // both empty
    }

    SECTION( "bbox/segment intersection" ) {
        // Point segment at center.
        REQUIRE( box.intersects_segment(box.center(), box.center()) );

        // Point segment outside box.
        REQUIRE_FALSE( box.intersects_segment(vec3t(0,1,2), vec3t(0,1,2)) );

        // Point segment at each corner.
        REQUIRE( box.intersects_segment(vec3t(1,2,3), vec3t(1,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,3), vec3t(4,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,3), vec3t(4,5,3)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,3), vec3t(1,5,3)) );
        REQUIRE( box.intersects_segment(vec3t(1,2,6), vec3t(1,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,6), vec3t(4,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,6), vec3t(4,5,6)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,6), vec3t(1,5,6)) );

        // Exact edges of box.
        REQUIRE( box.intersects_segment(vec3t(1,2,3), vec3t(4,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,3), vec3t(1,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,3), vec3t(4,5,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,3), vec3t(4,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,3), vec3t(1,5,3)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,3), vec3t(4,5,3)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,3), vec3t(1,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(1,2,3), vec3t(1,5,3)) );

        REQUIRE( box.intersects_segment(vec3t(1,2,6), vec3t(4,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,6), vec3t(1,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,6), vec3t(4,5,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,6), vec3t(4,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,6), vec3t(1,5,6)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,6), vec3t(4,5,6)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,6), vec3t(1,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(1,2,6), vec3t(1,5,6)) );

        REQUIRE( box.intersects_segment(vec3t(1,2,3), vec3t(1,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(1,2,6), vec3t(1,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,3), vec3t(4,2,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,2,6), vec3t(4,2,3)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,3), vec3t(4,5,6)) );
        REQUIRE( box.intersects_segment(vec3t(4,5,6), vec3t(4,5,3)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,3), vec3t(1,5,6)) );
        REQUIRE( box.intersects_segment(vec3t(1,5,6), vec3t(1,5,3)) );

        // Segments intersects only at corner.
        REQUIRE( box.intersects_segment(vec3t(0,3,2.5), vec3t(2,1,3.5)) );
        REQUIRE( box.intersects_segment(vec3t(3,1,3.5), vec3t(5,3,2.5)) );
        REQUIRE( box.intersects_segment(vec3t(5,4,2.5), vec3t(3,6,3.5)) );
        REQUIRE( box.intersects_segment(vec3t(2,6,3.5), vec3t(0,4,2.5)) );
        REQUIRE( box.intersects_segment(vec3t(0,3,5.5), vec3t(2,1,6.5)) );
        REQUIRE( box.intersects_segment(vec3t(3,1,6.5), vec3t(5,3,5.5)) );
        REQUIRE( box.intersects_segment(vec3t(5,4,5.5), vec3t(3,6,6.5)) );
        REQUIRE( box.intersects_segment(vec3t(2,6,6.5), vec3t(0,4,5.5)) );

        // Both ends of segment inside box.
        REQUIRE( box.intersects_segment(vec3t(2,4,5), vec3t(3,3,4)) );

        // Both ends of segment outside box.
        REQUIRE( box.intersects_segment(vec3t(0,6,7), vec3t(5,1,2)) );

        // Half in, half out.
        REQUIRE( box.intersects_segment(vec3t(-0.5,3.5,4.5), vec3t(2.5,3.5,4.5)) ); // -X
        REQUIRE( box.intersects_segment(vec3t(2.5,3.5,4.5), vec3t(5.5,3.5,4.5)) );  // +X
        REQUIRE( box.intersects_segment(vec3t(2.5,0.5,4.5), vec3t(2.5,3.5,4.5)) );  // -Y
        REQUIRE( box.intersects_segment(vec3t(2.5,3.5,4.5), vec3t(2.5,6.5,4.5)) );  // +Y
        REQUIRE( box.intersects_segment(vec3t(2.5,3.5,1.5), vec3t(2.5,3.5,4.5)) );  // -Z
        REQUIRE( box.intersects_segment(vec3t(2.5,3.5,4.5), vec3t(2.5,3.5,7.5)) );  // +Z

        // Outside by one dimension.
        REQUIRE_FALSE( box.intersects_segment(vec3t(5,2,3), vec3t(5,5,3)) ); // off by +1 x
        REQUIRE_FALSE( box.intersects_segment(vec3t(1,1,3), vec3t(4,1,3)) ); // off by -1 y
        REQUIRE_FALSE( box.intersects_segment(vec3t(1,2,2), vec3t(4,5,2)) ); // off by -1 z
    }

    SECTION( "bbox/ray intersection" ) {
        static const vec3t xpos = vec3t(1,0,0).inv(), xneg = vec3t(-1,0,0).inv(),
                           ypos = vec3t(0,1,0).inv(), yneg = vec3t(0,-1,0).inv(),
                           zpos = vec3t(0,0,1).inv(), zneg = vec3t(0,0,-1).inv();

        // Origin inside box.
        REQUIRE( box.intersects_ray(box.center(), vec3t(1,2,3).norm_inv()) );

        // Origin outside box, but direction points to center.
        REQUIRE( box.intersects_ray(vec3t(-3,-4,-5), (box.center() - vec3t(-3,-4,-5)).norm_inv()) );

        // Origin outside box, direction points away from center (but would intersect if ray
        // was an infinite line instead).
        REQUIRE_FALSE( box.intersects_ray(vec3t(-3,-4,-5),
            (vec3t(-3,-4,-5).norm_inv() - box.center()).norm_inv()) );

        // Origin outside box, ray pointed at center of each box face.
        REQUIRE( box.intersects_ray(vec3t(0,      T(3.5), T(4.5)), xpos) ); // -X face
        REQUIRE( box.intersects_ray(vec3t(10,     T(3.5), T(4.5)), xneg) ); // +X face
        REQUIRE( box.intersects_ray(vec3t(T(2.5), 0,      T(4.5)), ypos) ); // -Y face
        REQUIRE( box.intersects_ray(vec3t(T(2.5), 10,     T(4.5)), yneg) ); // +Y face
        REQUIRE( box.intersects_ray(vec3t(T(2.5), T(3.5), 0),      zpos) ); // -Z face
        REQUIRE( box.intersects_ray(vec3t(T(2.5), T(3.5), 10),     zneg) ); // +Z face

        // Origin at each box corner, rest of ray doesn't intersect box.
        REQUIRE( box.intersects_ray(vec3t(1,2,3), yneg) );
        REQUIRE( box.intersects_ray(vec3t(4,2,3), xpos) );
        REQUIRE( box.intersects_ray(vec3t(4,5,3), zneg) );
        REQUIRE( box.intersects_ray(vec3t(1,5,3), xneg) );
        REQUIRE( box.intersects_ray(vec3t(1,2,6), zpos) );
        REQUIRE( box.intersects_ray(vec3t(4,2,6), yneg) );
        REQUIRE( box.intersects_ray(vec3t(4,5,6), ypos) );
        REQUIRE( box.intersects_ray(vec3t(1,5,6), zpos) );

        // Origin at each box corner, ray along each connected edge.
        REQUIRE( box.intersects_ray(vec3t(1,2,3), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,2,3), ypos) );
        REQUIRE( box.intersects_ray(vec3t(1,2,3), zpos) );

        REQUIRE( box.intersects_ray(vec3t(4,2,3), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,2,3), ypos) );
        REQUIRE( box.intersects_ray(vec3t(4,2,3), zpos) );

        REQUIRE( box.intersects_ray(vec3t(4,5,3), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,5,3), yneg) );
        REQUIRE( box.intersects_ray(vec3t(4,5,3), zpos) );

        REQUIRE( box.intersects_ray(vec3t(1,5,3), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,5,3), yneg) );
        REQUIRE( box.intersects_ray(vec3t(1,5,3), zpos) );

        REQUIRE( box.intersects_ray(vec3t(1,2,6), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,2,6), ypos) );
        REQUIRE( box.intersects_ray(vec3t(1,2,6), zneg) );

        REQUIRE( box.intersects_ray(vec3t(4,2,6), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,2,6), ypos) );
        REQUIRE( box.intersects_ray(vec3t(4,2,6), zneg) );

        REQUIRE( box.intersects_ray(vec3t(4,5,6), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,5,6), yneg) );
        REQUIRE( box.intersects_ray(vec3t(4,5,6), zneg) );

        REQUIRE( box.intersects_ray(vec3t(1,5,6), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,5,6), yneg) );
        REQUIRE( box.intersects_ray(vec3t(1,5,6), zneg) );

        // Origin not in box, but ray contains a box edge.
        REQUIRE( box.intersects_ray(vec3t(0,2,3), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,1,3), ypos) );
        REQUIRE( box.intersects_ray(vec3t(1,2,2), zpos) );

        REQUIRE( box.intersects_ray(vec3t(5,2,3), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,1,3), ypos) );
        REQUIRE( box.intersects_ray(vec3t(4,2,2), zpos) );

        REQUIRE( box.intersects_ray(vec3t(5,5,3), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,6,3), yneg) );
        REQUIRE( box.intersects_ray(vec3t(4,5,2), zpos) );

        REQUIRE( box.intersects_ray(vec3t(0,5,3), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,6,3), yneg) );
        REQUIRE( box.intersects_ray(vec3t(1,5,2), zpos) );

        REQUIRE( box.intersects_ray(vec3t(0,2,6), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,1,6), ypos) );
        REQUIRE( box.intersects_ray(vec3t(1,2,7), zneg) );

        REQUIRE( box.intersects_ray(vec3t(5,2,6), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,1,6), ypos) );
        REQUIRE( box.intersects_ray(vec3t(4,2,7), zneg) );

        REQUIRE( box.intersects_ray(vec3t(5,5,6), xneg) );
        REQUIRE( box.intersects_ray(vec3t(4,6,6), yneg) );
        REQUIRE( box.intersects_ray(vec3t(4,5,7), zneg) );

        REQUIRE( box.intersects_ray(vec3t(0,5,6), xpos) );
        REQUIRE( box.intersects_ray(vec3t(1,6,6), yneg) );
        REQUIRE( box.intersects_ray(vec3t(1,5,7), zneg) );

        // Origin not in box, ray pointed in wrong direction (but would be on edge if it was right).
        REQUIRE_FALSE( box.intersects_ray(vec3t(0,2,3), xneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,1,3), yneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,2,2), zneg) );

        REQUIRE_FALSE( box.intersects_ray(vec3t(5,2,3), xpos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,1,3), yneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,2,2), zneg) );

        REQUIRE_FALSE( box.intersects_ray(vec3t(5,5,3), xpos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,6,3), ypos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,5,2), zneg) );

        REQUIRE_FALSE( box.intersects_ray(vec3t(0,5,3), xneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,6,3), ypos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,5,2), zneg) );

        REQUIRE_FALSE( box.intersects_ray(vec3t(0,2,6), xneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,1,6), yneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,2,7), zpos) );

        REQUIRE_FALSE( box.intersects_ray(vec3t(5,2,6), xpos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,1,6), yneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,2,7), zpos) );

        REQUIRE_FALSE( box.intersects_ray(vec3t(5,5,6), xpos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,6,6), ypos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(4,5,7), zpos) );

        REQUIRE_FALSE( box.intersects_ray(vec3t(0,5,6), xneg) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,6,6), ypos) );
        REQUIRE_FALSE( box.intersects_ray(vec3t(1,5,7), zpos) );

        // Infinitely thin boxes.
        box = bboxt(1,2,3, 1,5,6); //no width in x direction
        REQUIRE( box.intersects_ray(vec3t(0,T(3.5),T(4.5)),  xpos) );
        REQUIRE( box.intersects_ray(vec3t(10,T(3.5),T(4.5)), xneg) );

        box = bboxt(1,2,3, 4,2,6); //no width in y direction
        REQUIRE( box.intersects_ray(vec3t(T(2.5),0,T(4.5)),  ypos) );
        REQUIRE( box.intersects_ray(vec3t(T(2.5),10,T(4.5)), yneg) );

        box = bboxt(1,2,3, 4,5,3); //no width in z direction
        REQUIRE( box.intersects_ray(vec3t(T(2.5),T(3.5),0),  zpos) );
        REQUIRE( box.intersects_ray(vec3t(T(2.5),T(3.5),10), zneg) );
    }
}
