/* Unit tests for vec3 (util library).
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
#include <obvi/util/vec3.hpp>

using obvi::vec3;
using namespace Catch::literals; // To get "_a" UDL for approximate floating-point values.

#define VEC3_EQUAL(v, v0, v1, v2) \
    REQUIRE( v.pt[0] == v0 ); \
    REQUIRE( v.pt[1] == v1 ); \
    REQUIRE( v.pt[2] == v2 );


TEMPLATE_TEST_CASE("vec3 set and get", "[vec3]", float, double) {
    vec3<TestType> v;

    VEC3_EQUAL(v, 0, 0, 0);

    SECTION( "array and named element access are the same" ) {
        v.pt[0] = TestType(1.1);
        v.pt[1] = TestType(2.2);
        v.pt[2] = TestType(3.3);
        VEC3_EQUAL(v, v.x(), v.y(), v.z());
    }

    SECTION( "set method works" ) {
        v.set(TestType(4.4), TestType(5.5), TestType(6.6));
        VEC3_EQUAL(v, TestType(4.4), TestType(5.5), TestType(6.6));
    }

    SECTION( "initialization from different type works" ) {
        vec3<long double> w(1.0L,2.0L,3.6L);
        v = vec3<TestType>(w);
        VEC3_EQUAL(v, TestType(w.x()), TestType(w.y()), TestType(w.z()))
    }
}


TEMPLATE_TEST_CASE("vec3 math", "[vec3]", float, double) {
    vec3<TestType> v(1,2,3);
    vec3<TestType> w(4,5,6);
    TestType       s = 2.5;

    SECTION( "add" ) {
        SECTION( "vector in-place" ) {
            v += w;
            VEC3_EQUAL(v, 5.0_a, 7.0_a, 9.0_a);
        }

        SECTION( "vector separate" ) {
            v = v + w;
            VEC3_EQUAL(v, 5.0_a, 7.0_a, 9.0_a);
        }

        SECTION( "scalar in-place" ) {
            v += s;
            VEC3_EQUAL(v, 3.5_a, 4.5_a, 5.5_a);
        }

        SECTION( "scalar separate" ) {
            v = v + s;
            VEC3_EQUAL(v, 3.5_a, 4.5_a, 5.5_a);
        }
    }

    SECTION( "subtract" ) {
        SECTION( "vector in-place" ) {
            v -= w;
            VEC3_EQUAL(v, -3.0_a, -3.0_a, -3.0_a);
        }

        SECTION( "vector separate" ) {
            v = v - w;
            VEC3_EQUAL(v, -3.0_a, -3.0_a, -3.0_a);
        }

        SECTION( "scalar in-place" ) {
            v -= s;
            VEC3_EQUAL(v, -1.5_a, -0.5_a, 0.5_a);
        }

        SECTION( "scalar separate" ) {
            v = v - s;
            VEC3_EQUAL(v, -1.5_a, -0.5_a, 0.5_a);
        }
    }

    SECTION( "multiply" ) {
        SECTION( "scalar in-place" ) {
            v *= s;
            VEC3_EQUAL(v, 2.5_a, 5.0_a, 7.5_a);
        }

        SECTION( "scalar separate" ) {
            v = v * s;
            VEC3_EQUAL(v, 2.5_a, 5.0_a, 7.5_a);
        }
    }

    SECTION( "divide" ) {
        SECTION( "scalar in-place" ) {
            v /= s;
            VEC3_EQUAL(v, 0.4_a, 0.8_a, 1.2_a);
        }

        SECTION( "scalar separate" ) {
            v = v / s;
            VEC3_EQUAL(v, 0.4_a, 0.8_a, 1.2_a);
        }
    }

    SECTION( "dot product" ) {
        REQUIRE( v.dot(w) == 32.0_a );
        REQUIRE( v.dot(w.x(), w.y(), w.z()) == 32.0_a );
    }

    SECTION( "cross product" ) {
        v = v.cross(w);
        VEC3_EQUAL(v, -3.0_a, 6.0_a, -3.0_a);
    }

    SECTION( "2-norm" ) {
        REQUIRE( v.normsqd() == 14.0_a );

        w = v.normalized();
        VEC3_EQUAL(w, 0.2672612_a, 0.5345225_a, 0.8017837_a);
    }
}
