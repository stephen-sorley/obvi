/* Header-only class to manage a camera in 3D space.
 *
 * This includes both the camera's position/orientation in world space, and the projection
 * transformation used to model the camera's lens.
 *
 * Projection matrix math taken from here:
 *   https://www.glprogramming.com/red/appendixf.html (frustum only - the ortho matrix has a typo)
 *   https://en.wikipedia.org/wiki/Orthographic_projection
 *   http://www.songho.ca/opengl/gl_transform.html
 *
 * [point in clip coords] = Projection * View * Model * [point in object coordinates]
 *
 * To go from clip coords (4d vector) to normalized device coordinates (3d vector), divide
 * [x,y,z] components by w (fourth component).
 *
 * Normalized device coordinates (NDC) are just the window coordinates normalized to range [-1, 1].
 * Note that in OpenGL NDC (-1,-1) is the lower-left corner of the screen and (1,1) is the
 * upper-right.
 *
 * This differs from standard convention in windowing systems like Qt, where (0,0) is the upper-left
 * corner of the window, and (width,height) is the lower-right. You'll need to factor that in when
 * converting NDC coords to window coords, or vice-versa.
 *
 * Inverse (convert point in clip coords to a ray in object coords):
 * [point in object coords] = Model^-1 * View^-1 * Projection^-1 * [point in clip coords]
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
#ifndef OBVI_CAMERA3_HPP
#define OBVI_CAMERA3_HPP

#include <cmath>

#include <obvi/util/affine3.hpp>
#include <obvi/util/math.hpp>

namespace obvi {

template<typename real>
struct camera3
{
    enum class type {
        ORTHOGRAPHIC, // Orthographic projection (objects do not get smaller with distance)
        PERSPECTIVE   // Perspective projection (real-world view)
    };

    // Inverse of camera position and orientation (view matrix).
    affine3<real> view;

    void look_at(const vec3<real>& camera_pos, const vec3<real>& target_pos, const vec3<real>& up) {
        // Rotate axes so that -z points from camera to target. Set +x to be perpendicular to up
        // vector and +z, in direction to obey right-hand rule. Set +y to be perpendicular to +x
        // and +z. Store result in 'view' as affine transform.
        mat3<real> rot;
        vec3<real> eye = target_pos - camera_pos;
        rot.rows[0] = (eye.cross(up)).normalized();           // new +x, expressed in old coords
        rot.rows[1] = (rot.rows[0].cross(eye)).normalized();  // new +y, expressed in old coords
        rot.rows[2] = -eye.normalized();                      // new +z, expressed in old coords
        view = rot;

        // Apply translation to move camera back away from object (ends up being applied first).
        view *= affine3<real>(-eye);
    }

    // Camera projection.
    bool set_projection(type proj, real left, real right, real bottom, real top, real near, real far) {
        if(near <= real(0) || far <= real(0) || left == right || bottom == top) {
            return false;
        }

        proj_type = proj;

        mleft = left;
        mright = right;
        mbottom = bottom;
        mtop = top;
        mnear = near;
        mfar = far;

        recalc();
        return true;
    }

    /* Shortcut for common case - perspective projection centered on viewing axis, defined by
       vertical field-of-view angle in radians (fovy_rad) and aspect ratio.

       See: https://stackoverflow.com/a/12943456
     */
    bool set_perspective(real fovy_rad, real aspect_ratio, real near, real far) {
        if(fovy_rad <= real(0) || aspect_ratio <= real(0)) {
            return false;
        }

        real fH = std::tan(fovy_rad * real(0.5)) * near;
        real fW = fH * aspect_ratio;
        return set_frustum(-fW, fW, -fH, fH, near, far);
    }

    /* Construct (projection * view) matrix, save in opengl format. */
    template<typename GLreal>
    void to_gl(std::array<GLreal, 16>& arr) const {
        to_gl_internal(arr, view);
    }

    /* Construct (projection * view * model) matrix, save in opengl format. */
    template<typename GLreal>
    void to_gl(std::array<GLreal, 16>& arr, const affine3<real>& model) const {
        to_gl_internal(arr, view * model);
    }

private:
    type proj_type = type::ORTHOGRAPHIC;

    real mleft   = -1;
    real mright  = 1;
    real mbottom = -1;
    real mtop    = 1;
    real mnear   = 1;
    real mfar    = 1000;

    real p[6]    = {0};
    real invp[6] = {0};

    void recalc() {
        switch(proj_type) {
            case type::ORTHOGRAPHIC:
                recalc_ortho();
            break;
            case type::PERSPECTIVE:
                recalc_persp();
            break;
        }
    }

    void recalc_ortho() {
        real w  = mright - mleft;
        real h  = mtop - mbottom;
        real nd = mnear - mfar;

        /* Orthographic:
         |p[0]  0    0   p[4]|
         | 0   p[1]  0   p[5]|
         | 0    0   p[2] p[3]|
         | 0    0    0    1  |
        */
        p[0] = real(2) / w;
        p[1] = real(2) / h;
        p[2] = real(2) / nd;
        p[3] = (mnear + mfar) / nd;
        p[4] = (mleft + mright) / -w;
        p[5] = (mbottom + mtop) / -h;

        /* Orthographic (Inverse):
         |invp[0]    0       0    invp[4]|
         |   0    invp[1]    0    invp[5]|
         |   0       0    invp[2] invp[3]|
         |   0       0       0       1   |
        */
        invp[0] = w / real(2);
        invp[1] = h / real(2);
        invp[2] = nd / real(2);
        invp[3] = (mnear + mfar) / real(-2);
        invp[4] = (mleft + mright) / real(2);
        invp[5] = (mbottom + mtop) / real(2);
    }

    void recalc_persp() {
        real w   = mright - mleft;
        real h   = mtop - mbottom;
        real nd  = mnear - mfar;
        real n2  = real(2) * mnear;
        real nf2 = n2 * mfar;

        /* Perspective:
         |p[0]  0   p[4]  0  |
         | 0   p[1] p[5]  0  |
         | 0    0   p[2] p[3]|
         | 0    0    -1   0  |
        */
        p[0] = n2 / w;
        p[1] = n2 / h;
        p[2] = (mnear + mfar) / nd;
        p[3] = nf2 / nd;
        p[4] = (mright + mleft) / w;
        p[5] = (mbottom + mtop) / h;

        /* Perspective (Inverse):
         |invp[0]    0       0    invp[4]|
         |   0    invp[1]    0    invp[5]|
         |   0       0       0      -1   |
         |   0       0    invp[2] invp[3]|
        */
        invp[0] = w / n2;
        invp[1] = h / n2;
        invp[2] = nd / nf2;
        invp[3] = (mnear + mfar) / nf2;
        invp[4] = (mright + mleft) / n2;
        invp[5] = (mbottom + mtop) / n2;
    }

    static size_t colmajor(size_t row, size_t col) {
        return col * 4 + row;
    }

    template<typename GLreal>
    void to_gl_internal(std::array<GLreal, 16>& arr, const affine3<real>& aff) const {
        switch(proj_type) {
            case type::ORTHOGRAPHIC:
                to_gl_internal_orthographic(arr, aff);
            break;

            case type::PERSPECTIVE:
                to_gl_internal_perpective(arr, aff);
            break;
        }
    }

    // Compute (ortho projection * aff), store column-wise in arr for export to OpenGL.
    template<typename GLreal>
    void to_gl_internal_orthographic(std::array<GLreal, 16>& arr, const affine3<real>& aff) const {
        const affine3<real>& rot = aff.rotation();
        const vec3<real>& tr     = aff.translation();

        // Initialize with all zeros.
        arr.fill(GLreal(0));

        // Combine scale with rotation matrix, then compute upper-left 3x3 of result.
        real s00 = rot(0,0) * aff.scale();
        real s11 = rot(1,1) * aff.scale();
        real s22 = rot(2,2) * aff.scale();

        arr[colmajor(0,0)] = GLreal( p[0]*s00 );
        arr[colmajor(0,1)] = GLreal( p[0]*rot(0,1) );
        arr[colmajor(0,2)] = GLreal( p[0]*rot(0,2) );

        arr[colmajor(1,0)] = GLreal( p[1]*rot(1,0) );
        arr[colmajor(1,1)] = GLreal( p[1]*s11 );
        arr[colmajor(1,2)] = GLreal( p[1]*rot(1,2) );

        arr[colmajor(2,0)] = GLreal( p[2]*rot(2,0) );
        arr[colmajor(2,1)] = GLreal( p[2]*rot(2,1) );
        arr[colmajor(2,2)] = GLreal( p[2]*s22 );

        // Compute upper-right 3x1 of result.
        arr[colmajor(0,3)] = GLreal( p[0]*tr.x() + p[4] );
        arr[colmajor(1,3)] = GLreal( p[1]*tr.y() + p[5] );
        arr[colmajor(2,3)] = GLreal( p[2]*tr.z() + p[3] );

        arr[colmajor(3,3)] = GLreal( 1 );
    }

    // Compute (perspective projection * aff), store column-wise in arr for export to OpenGL.
    template<typename GLreal>
    void to_gl_internal_perpective(std::array<GLreal, 16>& arr, const affine3<real>& aff) const {
        const affine3<real>& rot = aff.rotation();
        const vec3<real>& tr     = aff.translation();

        // Initialize with all zeros.
        arr.fill(GLreal(0));

        // Combine scale with rotation matrix, then compute upper-left 3x3 of result.
        real s00 = rot(0,0) * aff.scale();
        real s11 = rot(1,1) * aff.scale();
        real s22 = rot(2,2) * aff.scale();

        arr[colmajor(0,0)] = GLreal( p[0]*s00      + p[4]*rot(2,0) );
        arr[colmajor(0,1)] = GLreal( p[0]*rot(0,1) + p[4]*rot(2,1) );
        arr[colmajor(0,2)] = GLreal( p[0]*rot(0,2) + p[4]*s22 );

        arr[colmajor(1,0)] = GLreal( p[1]*rot(1,0) + p[5]*rot(2,0) );
        arr[colmajor(1,1)] = GLreal( p[1]*s11      + p[5]*rot(2,1) );
        arr[colmajor(1,2)] = GLreal( p[1]*rot(1,2) + p[5]*s22 );

        arr[colmajor(2,0)] = GLreal( p[2]*rot(2,0) );
        arr[colmajor(2,1)] = GLreal( p[2]*rot(2,1) );
        arr[colmajor(2,2)] = GLreal( p[2]*s22 );

        // Compute upper-right 3x1 of result.
        arr[colmajor(0,3)] = GLreal( p[0]*tr.x() + p[4]*tr.z() );
        arr[colmajor(1,3)] = GLreal( p[1]*tr.y() + p[5]*tr.z() );
        arr[colmajor(2,3)] = GLreal( p[2]*tr.z() + p[3] );

        // Compute bottom row (1x4) of result.
        arr[colmajor(3,0)] = GLreal( -rot(2,0) );
        arr[colmajor(3,1)] = GLreal( -rot(2,1) );
        arr[colmajor(3,2)] = GLreal( -s22 );
        arr[colmajor(3,3)] = GLreal( -tr.z() );
    }
};

using camera3f = camera3<float>;
using camera3d = camera3<double>;

} // END namespace obvi
#endif // OBVI_CAMERA3_HPP
