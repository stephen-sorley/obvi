/* Header-only class that implements a 3-element vector.
 *
 * Used for 3D geometry calculations.
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
#ifndef OBVI_VEC3_HPP
#define OBVI_VEC3_HPP

#include <cmath>
#include <iostream>

namespace obvi {

template<typename real>
struct vec3
{
    real pt[3];

    vec3(real x=0, real y=0, real z=0) : pt{x,y,z} {}

    template<typename T>
    vec3(const vec3<T> &v) : pt{real(v.pt[0]), real(v.pt[1]), real(v.pt[2])} {}

    void set(real x, real y, real z) {
        pt[0] = x;
        pt[1] = y;
        pt[2] = z;
    }

    real& x() { return pt[0]; }
    const real& x() const { return pt[0]; }

    real& y() { return pt[1]; }
    const real& y() const { return pt[1]; }

    real& z() { return pt[2]; }
    const real& z() const { return pt[2]; }

    real& operator[](size_t i) {
        return pt[i];
    }
    const real& operator[](size_t i) const {
        return pt[i];
    }


    friend std::ostream& operator<<(std::ostream& os, const vec3& v) {
        os << '[' << v.x() << ", " << v.y() << ", " << v.z() << ']';
        return os;
    }

    // Negation
    vec3 operator-() const {
        vec3 ret;
        ret.p[0] = -pt[0];
        ret.p[1] = -pt[1];
        ret.p[2] = -pt[2];
        return ret;
    }

    // Addition
    vec3& operator+=(const vec3& rhs) {
        pt[0] += rhs.pt[0];
        pt[1] += rhs.pt[1];
        pt[2] += rhs.pt[2];
        return *this;
    }
    vec3 operator+(const vec3& rhs) const {
        vec3 ret = *this;
        ret += rhs;
        return ret;
    }
    vec3& operator+=(const real& rhs) {
        pt[0] += rhs;
        pt[1] += rhs;
        pt[2] += rhs;
        return *this;
    }
    friend vec3 operator+(const vec3& vec, const real& scalar) {
        vec3 ret = vec;
        return ret += scalar;
    }
    friend vec3 operator+(const real& scalar, const vec3& vec) {
        return vec + scalar;
    }

    // Subtraction
    vec3& operator-=(const vec3& rhs) {
        pt[0] -= rhs.pt[0];
        pt[1] -= rhs.pt[1];
        pt[2] -= rhs.pt[2];
        return *this;
    }
    vec3 operator-(const vec3& rhs) const {
        vec3 ret = *this;
        ret -= rhs;
        return ret;
    }
    vec3& operator-=(const real& rhs) {
        pt[0] -= rhs;
        pt[1] -= rhs;
        pt[2] -= rhs;
        return *this;
    }
    vec3 operator-(const real& rhs) const {
        vec3 ret = *this;
        ret -= rhs;
        return ret;
    }

    // Multiplication (element-wise)
    vec3& operator*=(const real& rhs) {
        pt[0] *= rhs;
        pt[1] *= rhs;
        pt[2] *= rhs;
        return *this;
    }
    friend vec3 operator*(const vec3& vec, const real& scalar) {
        vec3 ret = vec;
        ret *= scalar;
        return ret;
    }
    friend vec3 operator*(const real& scalar, const vec3& vec) {
        return vec * scalar;
    }

    vec3& operator*=(const vec3& rhs) {
        pt[0] *= rhs[0];
        pt[1] *= rhs[1];
        pt[2] *= rhs[2];
        return *this;
    }
    vec3 operator*(const vec3& rhs) const {
        vec3 ret = *this;
        return ret *= rhs;
    }

    // Division (element-wise)
    vec3& operator/=(const real& rhs) {
        pt[0] /= rhs;
        pt[1] /= rhs;
        pt[2] /= rhs;
        return *this;
    }
    vec3 operator/(const real& rhs) const {
        vec3 ret = *this;
        ret /= rhs;
        return ret;
    }
    friend vec3 operator/(const real& scalar, const vec3& vec) {
        return vec3(scalar / vec.pt[0], scalar / vec.pt[1], scalar / vec.pt[2]);
    }

    vec3& operator/=(const vec3& rhs) {
        pt[0] /= rhs.pt[0];
        pt[1] /= rhs.pt[1];
        pt[2] /= rhs.pt[2];
        return *this;
    }
    vec3 operator/(const vec3& rhs) const {
        vec3 ret = *this;
        return ret /= rhs;
    }

    // Other vector operations
    real dot(const vec3& rhs) const {
        return pt[0]*rhs.pt[0] + pt[1]*rhs.pt[1] + pt[2]*rhs.pt[2];
    }

    real dot(const real& x, const real& y, const real& z) const {
        return pt[0]*x + pt[1]*y + pt[2]*z;
    }

    vec3 cross(const vec3& rhs) const {
        return vec3(
            y()*rhs.z() - z()*rhs.y(),
            z()*rhs.x() - x()*rhs.z(),
            x()*rhs.y() - y()*rhs.x()
        );
    }

    vec3 abs() const {
        return vec3(std::abs(x()), std::abs(y()), std::abs(z()));
    }

    real normsqd() const {
        return this->dot(*this);
    }

    vec3& normalize() {
        return *this /= std::sqrt(normsqd());
    }
    vec3 normalized() const {
        return *this / std::sqrt(normsqd());
    }

    vec3 inv() const {
        // invert each element, return result in new vector.
        return vec3(real(1)/x(), real(1)/y(), real(1)/z());
    }
    vec3 norm_inv() const {
        // normalize the vector, then invert each element, then return result in new vector.
        real norm = std::sqrt(normsqd());
        return vec3(norm/x(), norm/y(), norm/z());
    }

};

using vec3f = vec3<float>;
using vec3d = vec3<double>;

} // END namespace obvi
#endif //OBVI_VEC3_HPP
