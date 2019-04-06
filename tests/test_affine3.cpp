/* Unit tests for affine3 (util library).
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
#include <obvi/util/affine3.hpp>
#include <obvi/util/mat3.hpp>
#include <obvi/util/vec3.hpp>
#include <array>

using obvi::affine3;
using obvi::mat3;
using obvi::vec3;
using namespace Catch::literals; // To get "_a" UDL for approvimate floating-point values.

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

#define MAT3_EQUAL(m, m00, m01, m02, m10, m11, m12, m20, m21, m22) \
    REQUIRE( m(0,0) == m00 ); \
    REQUIRE( m(0,1) == m01 ); \
    REQUIRE( m(0,2) == m02 ); \
    REQUIRE( m(1,0) == m10 ); \
    REQUIRE( m(1,1) == m11 ); \
    REQUIRE( m(1,2) == m12 ); \
    REQUIRE( m(2,0) == m20 ); \
    REQUIRE( m(2,1) == m21 ); \
    REQUIRE( m(2,2) == m22 );

#define VEC3_EQUAL(v, v0, v1, v2) \
    REQUIRE( v.pt[0] == v0 ); \
    REQUIRE( v.pt[1] == v1 ); \
    REQUIRE( v.pt[2] == v2 );

#define AFFINE3_EQUAL(a, m, v, s) \
    MAT3_EQUAL(a.rotation(), m(0,0), m(0,1), m(0,2), m(1,0), m(1,1), m(1,2), m(2,0), m(2,1), m(2,2)); \
    VEC3_EQUAL(a.translation(), v.x(), v.y(), v.z()); \
    REQUIRE(a.scale() == s);

TEMPLATE_TEST_CASE("affine3 set and get", "[affine3]", float, double) {
    affine3<TestType> aff;
    mat3<TestType>    mat = mat3<TestType>::identity();
    vec3<TestType>    vec = vec3<TestType>(0,0,0);
    TestType          sca = TestType(1);

    AFFINE3_EQUAL(aff, mat, vec, sca); //Tests accessor methods.

    SECTION( "construct from rotation" ) {
        mat = mat3<TestType>::xrot(TestType(0.3));
        aff = mat;
        AFFINE3_EQUAL(aff, mat, vec, sca);
    }

    SECTION( "construct from translation" ) {
        vec.set(1,2,3);
        aff = vec;
        AFFINE3_EQUAL(aff, mat, vec, sca);
    }

    SECTION( "construct from scale" ) {
        sca = TestType(2.5);
        aff = sca;
        AFFINE3_EQUAL(aff, mat, vec, sca);
    }

    SECTION( "set" ) {
        mat = mat3<TestType>::zrot(TestType(0.3));
        vec.set(1,2,3);
        sca = TestType(2.5);
        aff.set(mat, vec, sca);
        AFFINE3_EQUAL(aff, mat, vec, sca);
    }

    SECTION( "output as GL matrix" ) {
        std::array<long double, 16> g;
        g.fill(20);

        mat = mat3<TestType>::zrot(TestType(0.3));
        vec.set(1,2,3);
        sca = TestType(2.5);
        aff.set(mat, vec, sca);
        aff.to_gl(g);

        mat(0,0) *= sca;
        mat(1,1) *= sca;
        mat(2,2) *= sca;
        MAT3_EQUAL(mat, g[0], g[4], g[8], g[1], g[5], g[9], g[2], g[6], g[10]);
        VEC3_EQUAL(vec, g[12], g[13], g[14]);
        REQUIRE( g[3]  == 0.0_a );
        REQUIRE( g[7]  == 0.0_a );
        REQUIRE( g[11] == 0.0_a );
        REQUIRE( g[15] == 1.0_a );
    }
}

TEMPLATE_TEST_CASE("affine3 vector transform", "[affine3]", float, double) {
    affine3<TestType> aff;
    vec3<TestType>    vec;

    SECTION( "rotate then translate" ) {
        aff = vec3<TestType>(TestType(0.1), TestType(-0.2), TestType(0.5));
        aff *= mat3<TestType>::zrot(TestType(M_PI / 2.0));
        vec = aff * vec3<TestType>(TestType(0.5), TestType(0.5), 0);
        VEC3_EQUAL(vec, -0.4_a, 0.3_a, 0.5_a);
    }
}
