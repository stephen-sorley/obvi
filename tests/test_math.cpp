/* Unit tests for math functions (util library).
 *
 *
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
#include <obvi/util/math.hpp>

using namespace Catch::literals; // Provides "_a" UDL for approximate floating-point values.

using obvi::expand_bits_30;
using obvi::morton_encode_30;

TEST_CASE("expand_bits_30", "[math]") {
    REQUIRE( expand_bits_30(   0b1110110001u) == 0b001001001000001001000000000001u );
    REQUIRE( expand_bits_30(0b1011110110001u) == 0b001001001000001001000000000001u );
}

TEMPLATE_TEST_CASE("morton_encode_30", "[math]", float, double) {
    typedef TestType T;

    // Use all ones for a single dimension, make sure it gets completely full.
    REQUIRE( morton_encode_30<T>(1023,0,0) == 0b100100100100100100100100100100u );
    REQUIRE( morton_encode_30<T>(0,1023,0) == 0b010010010010010010010010010010u );
    REQUIRE( morton_encode_30<T>(0,0,1023) == 0b001001001001001001001001001001u );

    // Make sure order of bits within a single dimension is correct.
    REQUIRE( morton_encode_30<T>(0b1011,0,0) == 0b100000100100u );
    REQUIRE( morton_encode_30<T>(0,0b1011,0) == 0b010000010010u );
    REQUIRE( morton_encode_30<T>(0,0,0b1011) == 0b001000001001u );

    // Make sure inputs are being clamped to proper range.
    REQUIRE( morton_encode_30<T>(1030,0,0) == 0b100100100100100100100100100100u );
    REQUIRE( morton_encode_30<T>(-12,0,0)  == 0 );
}
