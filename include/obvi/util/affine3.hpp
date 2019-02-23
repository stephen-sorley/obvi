/* Header-only class that implements an affine transformation in 3D space.
 *
 * This class only supports affine transformations that represent some combination of
 * rotations, translations, and uniform scaling (i.e., scaling that's the same on every
 * axis). Shear and non-uniform scaling are not supported for two reasons:
 *   (1) These operations aren't useful when displaying 3D models - they distort the image.
 *   (2) Omitting these operations makes the transform trivially invertible (no determinant
 *       or division operations are needed), so inverses can be performed quickly and without
 *       greatly increasing the floating-point error (no determinant or float division).
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
#ifndef OBVI_AFFINE3_HPP
#define OBVI_AFFINE3_HPP

#include <cmath>
#include <iostream>

#include <obvi/util/vec3.hpp>
#include <obvi/util/mat3.hpp>

namespace obvi {

template<typename real>
struct affine3
{
    // General affine transform on vector 'x' to get new vector 'y' is of this form:
    //    y = Ax + b, where A is a 3x3 matrix and b is a 3x1 vector.
    //
    // For this class, that translates to the following:
    //    y = (rot * (scale * x)) + b

    affine3() {}

    // Intentionally didn't make this explicit - allows you to pre or post multiply a rotation
    // matrix directly, instead of wrapping with affine3(). Just a convenient shorthand.
    affine3(const mat3<real>& rotation) : rot(rotation) {}

    explicit affine3(const vec3<real>& translation) : tr(translation) {}

    explicit affine3(const real& scale) : uscale(scale) {}

    affine3& operator*=(const affine3& rhs) {
        uscale *= rhs.scale;
    }

    // Combine two affine transformations into a single affine transform.
    friend const affine3& operator*(const affine3& lhs, const affine3& rhs) {
        affine3 ret = lhs;

        lhs.tr     += lhs.rot * rhs.tr;
        lhs.rot    *= rhs.rot;
        lhs.uscale *= rhs.uscale;

        return ret;
    }

    // Transform the given vector.
    friend vec3<real> operator*(const affine3& aff, const vec3<real>& vec) {
        return aff.rot * (vec * aff.uscale) + aff.tr;
    }

    affine3& inv_inplace() {
        rot.trans_inplace();
        tr = -(rot * tr);
        uscale = real(1) / uscale;
        return *this;
    }
    friend affine3& inv_inplace(affine3& aff) {
        return aff.inv_inplace();
    }

    affine3 inv() const {
        affine3 ret = *this;
        return ret.inv_inplace();
    }
    friend affine3 inv(const affine3& aff) {
        return aff.inv();
    }


private:
    // rotation part of transformation (NOT A GENERAL 3x3, this invertible by transpose)
    mat3<real> rot = mat3<real>::identity();
    // translation part of transformation
    vec3<real> tr;
    // uniform scaling part of transformation
    real       uscale = 1;
};

using affine3f = affine3<float>;
using affine3d = affine3<double>;

} // END namespace obvi
#endif //OBVI_AFFINE3_HPP
