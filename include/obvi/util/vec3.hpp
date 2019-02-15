#ifndef OBVI_VEC3_HPP
#define OBVI_VEC3_HPP

#include <cmath>

namespace obvi {

template<typename real>
struct vec3
{
    real pt[3];
    
    vec3(real x=0, real y=0, real z=0) : pt{x,y,z} {}
    
    template<typename T>
    vec3(const vec3<T> v) : pt{(real)v.pt[0], (real)v.pt[1], (real)v.pt[2]} {}
    
    real& x() { return pt[0]; }
    const real& x() const { return pt[0]; }

    real& y() { return pt[1]; }
    const real& y() const { return pt[1]; }

    real& z() { return pt[2]; }
    const real& z() const { return pt[2]; }

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
    
    vec3 cross(const vec3& rhs) const {
        return vec3(
            y()*rhs.z() - z()*rhs.y(),
            z()*rhs.x() - x()*rhs.z(),
            x()*rhs.y() - y()*rhs.x()
        );
    }
    
    real normsqd() const {
        return pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2];
    }
    
    vec3& normalize() {
        return *this /= sqrt(normsqd());
    }
    vec3 normalized() const {
        return *this / sqrt(normsqd());
    }
};

using vec3f = vec3<float>;
using vec3d = vec3<double>;

} // END namespace obvi
#endif //OBVI_VEC3_HPP
