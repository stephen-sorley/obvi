#include <catch2/catch.hpp>
#include <obvi/util/vec3.hpp>

using obvi::vec3;
using namespace Catch::literals; // To get "_a" UDL for approximate floating-point values.

TEMPLATE_TEST_CASE("vec3 set and get", "[vec3]", float, double) {
    vec3<TestType> v;

    REQUIRE( v.x() == TestType(0) );
    REQUIRE( v.y() == TestType(0) );
    REQUIRE( v.z() == TestType(0) );

    SECTION( "array and named element access are the same" ) {
        v.pt[0] = TestType(1.1);
        v.pt[1] = TestType(2.2);
        v.pt[2] = TestType(3.3);

        REQUIRE( v.pt[0] == v.x() );
        REQUIRE( v.pt[1] == v.y() );
        REQUIRE( v.pt[2] == v.z() );
    }

    SECTION( "initialization from different type works" ) {
        vec3<long double> w(1,2,3.6);
        v = vec3<TestType>(w);

        REQUIRE( v.x() == TestType(w.x()) );
        REQUIRE( v.y() == TestType(w.y()) );
        REQUIRE( v.z() == TestType(w.z()) );
    }
}


TEMPLATE_TEST_CASE("vec3 math", "[vec3]", float, double) {
    vec3<TestType> v(1,2,3);
    vec3<TestType> w(4,5,6);
    TestType       s = 2.5;

    SECTION( "add" ) {
        SECTION( "vector in-place" ) {
            v += w;
            REQUIRE( v.x() == 5.0_a );
            REQUIRE( v.y() == 7.0_a );
            REQUIRE( v.z() == 9.0_a );
        }

        SECTION( "vector separate" ) {
            v = v + w;
            REQUIRE( v.x() == 5.0_a );
            REQUIRE( v.y() == 7.0_a );
            REQUIRE( v.z() == 9.0_a );
        }

        SECTION( "scalar in-place" ) {
            v += s;
            REQUIRE( v.x() == 3.5_a );
            REQUIRE( v.y() == 4.5_a );
            REQUIRE( v.z() == 5.5_a );
        }

        SECTION( "scalar separate" ) {
            v = v + s;
            REQUIRE( v.x() == 3.5_a );
            REQUIRE( v.y() == 4.5_a );
            REQUIRE( v.z() == 5.5_a );
        }
    }

    SECTION( "subtract" ) {
        SECTION( "vector in-place" ) {
            v -= w;
            REQUIRE( v.x() == -3.0_a );
            REQUIRE( v.y() == -3.0_a );
            REQUIRE( v.z() == -3.0_a );
        }

        SECTION( "vector separate" ) {
            v = v - w;
            REQUIRE( v.x() == -3.0_a );
            REQUIRE( v.y() == -3.0_a );
            REQUIRE( v.z() == -3.0_a );
        }

        SECTION( "scalar in-place" ) {
            v -= s;
            REQUIRE( v.x() == -1.5_a );
            REQUIRE( v.y() == -0.5_a );
            REQUIRE( v.z() ==  0.5_a );
        }

        SECTION( "scalar separate" ) {
            v = v - s;
            REQUIRE( v.x() == -1.5_a );
            REQUIRE( v.y() == -0.5_a );
            REQUIRE( v.z() ==  0.5_a );
        }
    }

    SECTION( "multiply" ) {
        SECTION( "scalar in-place" ) {
            v *= s;
            REQUIRE( v.x() == 2.5_a );
            REQUIRE( v.y() == 5.0_a );
            REQUIRE( v.z() == 7.5_a );
        }

        SECTION( "scalar separate" ) {
            v = v * s;
            REQUIRE( v.x() == 2.5_a );
            REQUIRE( v.y() == 5.0_a );
            REQUIRE( v.z() == 7.5_a );
        }
    }

    SECTION( "divide" ) {
        SECTION( "scalar in-place" ) {
            v /= s;
            REQUIRE( v.x() == 0.4_a );
            REQUIRE( v.y() == 0.8_a );
            REQUIRE( v.z() == 1.2_a );
        }

        SECTION( "scalar separate" ) {
            v = v / s;
            REQUIRE( v.x() == 0.4_a );
            REQUIRE( v.y() == 0.8_a );
            REQUIRE( v.z() == 1.2_a );
        }
    }

    SECTION( "dot product" ) {
        REQUIRE( v.dot(w) == 32.0_a );
    }

    SECTION( "cross product" ) {
        v = v.cross(w);
        REQUIRE( v.x() == -3.0_a );
        REQUIRE( v.y() ==  6.0_a );
        REQUIRE( v.z() == -3.0_a );
    }

    SECTION( "2-norm" ) {
        REQUIRE( v.normsqd() == 14.0_a );

        w = v.normalized();
        REQUIRE( w.x() == 0.2672612_a );
        REQUIRE( w.y() == 0.5345225_a );
        REQUIRE( w.z() == 0.8017837_a );
    }
}
