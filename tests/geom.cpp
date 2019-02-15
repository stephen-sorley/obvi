#include <catch2/catch.hpp>
#include <obvi/util/vec3.hpp>

using obvi::vec3;

TEMPLATE_TEST_CASE("vec3 set and get","[vec3]", float, double) {
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
}
