/* Unit tests for mat3 (util library).
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
#include <obvi/util/mat3.hpp>

using obvi::vec3;
using obvi::mat3;
using namespace Catch::literals; // To get "_a" UDL for approximate floating-point values.

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


TEMPLATE_TEST_CASE("mat3 set and get", "[mat3]", float, double) {
    mat3<TestType> m;

    MAT3_EQUAL(m, 0,0,0, 0,0,0, 0,0,0);

    SECTION( "index and exact element access are the same" ) {
        m.rows[0].set(1,2,3);
        m.rows[1].set(4,5,6);
        m.rows[2].set(7,8,9);

        MAT3_EQUAL(m, 1,2,3, 4,5,6, 7,8,9);
    }

    SECTION( "set method works" ) {
        m.set(1,2,3, 4,5,6, 7,8,9);
        MAT3_EQUAL(m, 1,2,3, 4,5,6, 7,8,9);

        m.set(vec3<TestType>(10,11,12), vec3<TestType>(13,14,15), vec3<TestType>(16,17,18));
        MAT3_EQUAL(m, 10,11,12, 13,14,15, 16,17,18);
    }

    SECTION( "initialization from different type works" ) {
        mat3<long double> n(1.0L,2.0L,3.0L, 4.0L,5.0L,6.0L, 7.0L,8.0L,9.0L);
        m = mat3<TestType>(n);

        MAT3_EQUAL(m, 1,2,3, 4,5,6, 7,8,9);
    }

    SECTION( "column access works" ) {
        m.set(1,2,3, 4,5,6, 7,8,9);

        VEC3_EQUAL(m.col(0), 1,4,7);
        VEC3_EQUAL(m.col(1), 2,5,8);
        VEC3_EQUAL(m.col(2), 3,6,9);
    }

    SECTION( "special matrices" ) {
        m = mat3<TestType>::identity();
        MAT3_EQUAL(m, 1,0,0, 0,1,0, 0,0,1);

        m = mat3<TestType>::xrot(TestType(0.4));
        MAT3_EQUAL(m, 1.0_a, 0.0_a,        0.0_a,
                     0.0_a, 0.9210610_a, -0.3894183_a,
                     0.0_a, 0.3894183_a,  0.9210610_a);

        m = mat3<TestType>::yrot(TestType(0.4));
        MAT3_EQUAL(m,  0.9210610_a, 0.0_a, 0.3894183_a,
                      0.0_a,       1.0_a, 0.0_a,
                     -0.3894183_a, 0.0_a, 0.9210610_a);

        m = mat3<TestType>::zrot(TestType(0.4));
        MAT3_EQUAL(m, 0.9210610_a, -0.3894183_a, 0.0_a,
                     0.3894183_a,  0.9210610_a, 0.0_a,
                     0.0_a,        0.0_a,       1.0_a);
    }
}


TEMPLATE_TEST_CASE("mat3 math", "[mat3]", float, double) {
    mat3<TestType> m(1, 2, 3,  4, 5, 6,  7, 8, 9);
    mat3<TestType> n(10,11,12, 13,14,15, 16,17,18);
    vec3<TestType> v(2,3,4);
    TestType       s = 2.5;

    SECTION( "add" ) {
        SECTION( "matrix in-place" ) {
            m += n;
            MAT3_EQUAL(m, 11.0_a,13.0_a,15.0_a, 17.0_a,19.0_a,21.0_a, 23.0_a,25.0_a,27.0_a);
        }
        SECTION( "matrix separate" ) {
            m = m + n;
            MAT3_EQUAL(m, 11.0_a,13.0_a,15.0_a, 17.0_a,19.0_a,21.0_a, 23.0_a,25.0_a,27.0_a);
        }
        SECTION( "scalar in-place" ) {
            m += s;
            MAT3_EQUAL(m, 3.5_a,4.5_a,5.5_a, 6.5_a,7.5_a,8.5_a, 9.5_a,10.5_a,11.5_a);
        }
        SECTION( "scalar separate" ) {
            m = m + s;
            MAT3_EQUAL(m, 3.5_a,4.5_a,5.5_a, 6.5_a,7.5_a,8.5_a, 9.5_a,10.5_a,11.5_a);
        }
    }

    SECTION( "add" ) {
        SECTION( "matrix in-place" ) {
            m += n;
            MAT3_EQUAL(m, 11.0_a,13.0_a,15.0_a, 17.0_a,19.0_a,21.0_a, 23.0_a,25.0_a,27.0_a);
        }
        SECTION( "matrix separate" ) {
            m = m + n;
            MAT3_EQUAL(m, 11.0_a,13.0_a,15.0_a, 17.0_a,19.0_a,21.0_a, 23.0_a,25.0_a,27.0_a);
        }
        SECTION( "scalar in-place" ) {
            m += s;
            MAT3_EQUAL(m, 3.5_a,4.5_a,5.5_a, 6.5_a,7.5_a,8.5_a, 9.5_a,10.5_a,11.5_a);
        }
        SECTION( "scalar separate" ) {
            m = m + s;
            MAT3_EQUAL(m, 3.5_a,4.5_a,5.5_a, 6.5_a,7.5_a,8.5_a, 9.5_a,10.5_a,11.5_a);
        }
    }

    SECTION( "subtract" ) {
        SECTION( "matrix in-place" ) {
            m -= n;
            MAT3_EQUAL(m, -9.0_a,-9.0_a,-9.0_a, -9.0_a,-9.0_a,-9.0_a, -9.0_a,-9.0_a,-9.0_a);
        }
        SECTION( "matrix separate" ) {
            m = m - n;
            MAT3_EQUAL(m, -9.0_a,-9.0_a,-9.0_a, -9.0_a,-9.0_a,-9.0_a, -9.0_a,-9.0_a,-9.0_a);
        }
        SECTION( "scalar in-place" ) {
            m -= s;
            MAT3_EQUAL(m, -1.5_a,-0.5_a,0.5_a, 1.5_a,2.5_a,3.5_a, 4.5_a,5.5_a,6.5_a);
        }
        SECTION( "scalar separate" ) {
            m = m - s;
            MAT3_EQUAL(m, -1.5_a,-0.5_a,0.5_a, 1.5_a,2.5_a,3.5_a, 4.5_a,5.5_a,6.5_a);
        }
    }

    SECTION( "multiply" ) {
        SECTION( "matrix in-place" ) {
            m *= n;
            MAT3_EQUAL(m, 84.0_a,90.0_a,96.0_a, 201.0_a,216.0_a,231.0_a, 318.0_a,342.0_a,366.0_a);
        }
        SECTION( "matrix separate" ) {
            m = m * n;
            MAT3_EQUAL(m, 84.0_a,90.0_a,96.0_a, 201.0_a,216.0_a,231.0_a, 318.0_a,342.0_a,366.0_a);
        }
        SECTION( "matrix-vector" ) {
            v = m * v;
            VEC3_EQUAL(v, 20.0_a, 47.0_a, 74.0_a);
        }
        SECTION( "scalar in-place" ) {
            m *= s;
            MAT3_EQUAL(m, 2.5_a,5.0_a,7.5_a, 10.0_a,12.5_a,15.0_a, 17.5_a,20.0_a,22.5_a);
        }
        SECTION( "scalar separate" ) {
            m = m * s;
            MAT3_EQUAL(m, 2.5_a,5.0_a,7.5_a, 10.0_a,12.5_a,15.0_a, 17.5_a,20.0_a,22.5_a);
        }
    }

    SECTION( "divide" ) {
        SECTION( "scalar in-place" ) {
            m /= s;
            MAT3_EQUAL(m, 0.4_a,0.8_a,1.2_a, 1.6_a,2.0_a,2.4_a, 2.8_a,3.2_a,3.6_a);
        }
        SECTION( "scalar separate" ) {
            m = m / s;
            MAT3_EQUAL(m, 0.4_a,0.8_a,1.2_a, 1.6_a,2.0_a,2.4_a, 2.8_a,3.2_a,3.6_a);
        }
    }

    SECTION( "diagonal" ) {
        v = m.diag();
        VEC3_EQUAL(v, 1, 5, 9);

        v = diag(m);
        VEC3_EQUAL(v, 1, 5, 9);
    }

    SECTION( "determinant" ) {
        s = m.det();
        REQUIRE( s == 0.0_a );
        s = det(m);
        REQUIRE( s == 0.0_a );

        m.set(7,-1,2, 5,1,3, 9,8,7);
        s = m.det();
        REQUIRE( s == -49.0_a );
        s = det(m);
        REQUIRE( s == -49.0_a );
    }

    SECTION( "transpose" ) {
        n = m.trans();
        MAT3_EQUAL(n, 1,4,7, 2,5,8, 3,6,9);

        n = trans(m);
        MAT3_EQUAL(n, 1,4,7, 2,5,8, 3,6,9);
    }
}
