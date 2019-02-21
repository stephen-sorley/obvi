/* The MIT License (MIT)
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
    vec3 operator+(const real& rhs) const {
        vec3 ret = *this;
        ret += rhs;
        return ret;
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

    // Scalar Multiplication
    vec3& operator*=(const real& rhs) {
        pt[0] *= rhs;
        pt[1] *= rhs;
        pt[2] *= rhs;
        return *this;
    }
    vec3 operator*(const real& rhs) const {
        vec3 ret = *this;
        ret *= rhs;
        return ret;
    }

    // Scalar Division
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

    real normsqd() const {
        return this->dot(*this);
    }

    vec3& normalize() {
        return *this /= std::sqrt(normsqd());
    }
    vec3 normalized() const {
        return *this / std::sqrt(normsqd());
    }
};

using vec3f = vec3<float>;
using vec3d = vec3<double>;

} // END namespace obvi
#endif //OBVI_VEC3_HPP
