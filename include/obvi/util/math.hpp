/* Header containing basic math constants and functions that aren't part of C++ standard.
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
#ifndef OBVI_MATH_HPP
#define OBVI_MATH_HPP

#include <stdint.h> // used this instead of cstdint to make sure type names are in std namespace.
#include <algorithm> // for std::min, std::max

#ifdef _MSC_VER
#  include <intrin.h> // provides _BitScanReverse()
#endif

namespace obvi {

    template<typename real>
    constexpr real pi = real(3.141592653589793238462643383279502884197L);

    template<typename real>
    static inline constexpr real deg2rad(const real& deg) {
        return deg * (pi<real> / real(180));
    }

    template<typename real>
    static inline constexpr real rad2deg(const real& rad) {
        return rad * (real(180) / pi<real>);
    }

    static inline uint32_t count_leading_zeros(uint32_t x) {
#if defined(__GNUC__)
        // GCC or Clang
        return (x > 0)? (uint32_t)__builtin_clz(x) : 32u;
#elif defined(_MSC_VER)
        // Visual Studio
        unsigned long leading_zero = 0;
        if(_BitScanReverse(&leading_zero, x)) {
            return 31u - leading_zero;
        } else {
            return 32u;
        }
#else
        static const uint8_t lut[32] = {
            0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19,
            1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18
        };
        if(x == 0) {
            return 32;
        }
        x |= x>>1;
        x |= x>>2;
        x |= x>>4;
        x |= x>>8;
        x |= x>>16;
        x++;
        return lut[x*0x076be629>>27];
#endif
    }

    /* Expand a 10-bit integer into 30 bits by inserting 2 zeros above each bit.
     *
     * E.g., 1111111111 becomes 001001001001001001001001001001.
     *
     * This is a helper function for morton_3d.
     *
     * See: https://devblogs.nvidia.com/thinking-parallel-part-iii-tree-construction-gpu/
     */
    static inline uint32_t expand_bits_30(uint32_t v)
    {
        v = v & 0x3FF; // mask off everything above first 10 bits.
        v = (v * 0x00010001u) & 0xFF0000FFu;
        v = (v * 0x00000101u) & 0x0F00F00Fu;
        v = (v * 0x00000011u) & 0xC30C30C3u;
        v = (v * 0x00000005u) & 0x49249249u;
        return v;
    }

    const uint32_t morton_30_max = 1 << 10u; // 10 bits per dim

    /* Convert a 3D point into a 30-bit morton code.
     *
     * Note that each value (x,y,z) must lie on the range [0,morton_30_max). Values outside this
     * range will be clamped, so you must map each dimension of your data to this range before you
     * pass it in.
     *
     * See: https://devblogs.nvidia.com/thinking-parallel-part-iii-tree-construction-gpu/
     */
    template<typename real>
    static inline uint32_t morton_encode_30(real x, real y, real z) {
        // Clamp x,y,z to [0,1023]. (Use 1023 as upper limit because we're going to truncate anyway)
        x = std::min(std::max(x, real(0)), real(morton_30_max - 1));
        y = std::min(std::max(y, real(0)), real(morton_30_max - 1));
        z = std::min(std::max(z, real(0)), real(morton_30_max - 1));
        // Truncate value to integer, then pass to expand_bits.
        // The truncation effectively divides each dimension into 1024 buckets of size 1.
        unsigned int xx = expand_bits_30(uint32_t(x));
        unsigned int yy = expand_bits_30(uint32_t(y));
        unsigned int zz = expand_bits_30(uint32_t(z));
        // Interleave bits from the expanded x,y,z values, to form a single 30-bit code.
        return (xx * 4) + (yy * 2) + zz;
    }

} // END namespace obvi
#endif // OBVI_MATH_HPP
