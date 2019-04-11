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

    //TODO: add bbox/bbox intersection tests
    //TODO: add bbox/segment intersection tests
    //TODO: add bbox/ray intersection tests
}
