/* Header-only class that implements a 3x3 matrix.
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
#ifndef OBVI_MAT3_HPP
#define OBVI_MAT3_HPP

#include <cmath>
#include <limits>
#include <iostream>

#include <obvi/util/vec3.hpp>

namespace obvi {

template<typename real>
struct mat3
{
    vec3<real> rows[3];

    mat3() : rows{vec3<real>(), vec3<real>(), vec3<real>()} {}

    mat3(real a11, real a12, real a13, real a21, real a22, real a23, real a31, real a32, real a33)
        : rows{vec3<real>(a11,a12,a13), vec3<real>(a21,a22,a23), vec3<real>(a31,a32,a33)} {}

    mat3(const vec3<real>& r1, const vec3<real>& r2, const vec3<real> &r3)
        : rows{r1,r2,r3} {}

    template<typename T>
    mat3(const mat3<T> &m)
        : rows{vec3<real>(m.rows[0]), vec3<real>(m.rows[1]), vec3<real>(m.rows[2])} {}

    void set(real a11, real a12, real a13,
             real a21, real a22, real a23,
             real a31, real a32, real a33) {
        rows[0].set(a11,a12,a13);
        rows[1].set(a21,a22,a23);
        rows[2].set(a31,a32,a33);
    }

    void set(const vec3<real>& row0, const vec3<real>& row1, const vec3<real>& row2) {
        rows[0] = row0;
        rows[1] = row1;
        rows[2] = row2;
    }

    real& operator()(size_t rowIdx, size_t colIdx) { return rows[rowIdx].pt[colIdx]; }
    const real& operator()(size_t rowIdx, size_t colIdx) const { return rows[rowIdx].pt[colIdx]; }

    vec3<real> col(size_t colIdx) const {
        return vec3<real>(rows[0].pt[colIdx], rows[1].pt[colIdx], rows[2].pt[colIdx]);
    }

    friend std::ostream& operator<<(std::ostream& os, const mat3& m) {
        os << '{' << m.rows[0] << ", " << m.rows[1] << ", " << m.rows[2] << '}';
        return os;
    }

    // Static methods to create and initialize some common matrices.
    static mat3 diagonal(real fill) {
        return mat3(fill,    0,    0,
                       0, fill,    0,
                       0,    0, fill);
    }

    static mat3 diagonal(real d1, real d2, real d3) {
        return mat3(d1,  0,  0,
                     0, d2,  0,
                     0,  0, d3);
    }

    static mat3 identity() {
        return diagonal(1);
    }

    static mat3 xrot(real angle_radians) {
        real s = std::sin(angle_radians);
        real c = std::cos(angle_radians);
        return mat3(1,0,0,
                    0,c,-s,
                    0,s,c);
    }

    static mat3 yrot(real angle_radians) {
        real s = std::sin(angle_radians);
        real c = std::cos(angle_radians);
        return mat3(c,0,s,
                    0,1,0,
                    -s,0,c);
    }

    static mat3 zrot(real angle_radians) {
        real s = std::sin(angle_radians);
        real c = std::cos(angle_radians);
        return mat3(c,-s,0,
                    s,c,0,
                    0,0,1);
    }

    // Negation.
    mat3 operator-() const {
        return mat3(-rows[0], -rows[1], -rows[2]);
    }

    // Addition.
    mat3& operator+=(const mat3& rhs) {
        rows[0] += rhs.rows[0];
        rows[1] += rhs.rows[1];
        rows[2] += rhs.rows[2];
        return *this;
    }
    mat3 operator+(const mat3& rhs) const {
        mat3 ret = *this;
        ret += rhs;
        return ret;
    }
    mat3& operator+=(const real& rhs) {
        rows[0] += rhs;
        rows[1] += rhs;
        rows[2] += rhs;
        return *this;
    }
    friend mat3 operator+(const mat3& mat, const real& scalar) {
        mat3 ret = mat;
        return ret += scalar;
    }
    friend mat3 operator+(const real& scalar, const mat3& mat) {
        return mat + scalar;
    }

    // Subtraction.
    mat3& operator-=(const mat3& rhs) {
        rows[0] -= rhs.rows[0];
        rows[1] -= rhs.rows[1];
        rows[2] -= rhs.rows[2];
        return *this;
    }
    mat3 operator-(const mat3& rhs) const {
        mat3 ret = *this;
        ret -= rhs;
        return ret;
    }
    mat3& operator-=(const real& rhs) {
        rows[0] -= rhs;
        rows[1] -= rhs;
        rows[2] -= rhs;
        return *this;
    }
    mat3 operator-(const real& rhs) const {
        mat3 ret = *this;
        ret -= rhs;
        return ret;
    }

    // Multiplication.
    mat3& operator*=(const mat3& rhs) {
        for(size_t i=0; i<3; ++i) {
            rows[i].set(
                rows[i].dot(rhs(0,0), rhs(1,0), rhs(2,0)),
                rows[i].dot(rhs(0,1), rhs(1,1), rhs(2,1)),
                rows[i].dot(rhs(0,2), rhs(1,2), rhs(2,2))
            );
        }
        return *this;
    }
    mat3 operator*(const mat3& rhs) const {
        mat3 ret = *this;
        ret *= rhs;
        return ret;
    }
    mat3& operator*=(const real& rhs) {
        rows[0] *= rhs;
        rows[1] *= rhs;
        rows[2] *= rhs;
        return *this;
    }
    friend mat3 operator*(const mat3& mat, const real& scalar) {
        mat3 ret = mat;
        return ret *= scalar;
    }
    friend mat3 operator*(const real& scalar, const mat3& mat) {
        return mat * scalar;
    }

    // Division (scalar only).
    mat3& operator/=(const real& rhs) {
        rows[0] /= rhs;
        rows[1] /= rhs;
        rows[2] /= rhs;
        return *this;
    }
    mat3 operator/(const real& rhs) const {
        mat3 ret = *this;
        ret /= rhs;
        return ret;
    }

    // Linear algebra operations.
    vec3<real> operator*(const vec3<real>& rhs) const {
        /* Implements matrix-vector multiplication:
         *  [ a11 a12 a13 ]   [v1]
         *  [ a21 a22 a23 ] * [v2] = ...
         *  [ a31 a32 a33 ]   [v3]
         */
        return vec3<real>(rows[0].dot(rhs), rows[1].dot(rhs), rows[2].dot(rhs));
    }

    vec3<real> diag() const {
        const mat3& m = *this;
        return vec3<real>(m(0,0), m(1,1), m(2,2));
    }
    friend vec3<real> diag(const mat3& m) {
        return m.diag();
    }

    real det() const {
        const mat3& m = *this;
        return   m(0,0) * (m(1,1)*m(2,2) - m(1,2)*m(2,1))
               - m(0,1) * (m(1,0)*m(2,2) - m(1,2)*m(2,0))
               + m(0,2) * (m(1,0)*m(2,1) - m(1,1)*m(2,0));
    }
    friend real det(const mat3& m) {
        return m.det();
    }

    mat3& trans_inplace() {
        mat3& m = *this;
        std::swap(m(1,0), m(0,1));
        std::swap(m(2,0), m(0,2));
        std::swap(m(2,1), m(1,2));
        return m;
    }
    friend mat3& trans_inplace(mat3 &m) {
        return m.trans_inplace();
    }

    mat3 trans() const {
        mat3 ret = *this;
        ret.trans_inplace();
        return ret;
    }
    friend mat3 trans(const mat3& m) {
        return m.trans();
    }

    bool is_orthogonal() const {
        return (*this * trans()).is_identity();
    }
    friend bool is_orthogonal(const mat3& m) {
        return m.is_orthogonal();
    }


private:
    static bool approx_0(const real& val) {
        return std::abs(val) <= std::numeric_limits<real>::epsilon();
    }

    static bool approx_1(const real& val) {
        return std::abs(val - 1) <= std::numeric_limits<real>::epsilon();
    }

    bool is_diag() const {
        const mat3& m = *this;
        return approx_0(m(1,0)) && approx_0(m(2,0)) && approx_0(m(2,1))
            && approx_0(m(0,1)) && approx_0(m(0,2)) && approx_0(m(1,2));
    }

    bool is_identity() const {
        const mat3& m = *this;
        return m.is_diag() && approx_1(m(0,0)) && approx_1(m(1,1)) && approx_1(m(2,2));
    }
};

using mat3f = mat3<float>;
using mat3d = mat3<double>;

} // END namespace obvi
#endif //OBVI_MAT3_HPP
