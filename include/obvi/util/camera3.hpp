/* Header-only class to manage a camera in 3D space.
 *
 * This includes both the camera's position/orientation in world space, and the projection
 * transformation used to model the camera's lens.
 *
 * All projection transformations are on-axis, meaning that we're assuming that the viewer of the
 * image is centered on the screen. This makes the inverse of the projection matrix easier to
 * calculate, because the on-axis matrix is block-diagonal. Off-axis projections are really only
 * useful for unusual setups (like those encountered in VR).
 *
 * Projection matrix math taken from here:
 *   http://www.songho.ca/opengl/gl_projectionmatrix.html
 *   http://www.songho.ca/opengl/gl_transform.html
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

namespace obvi {

template<typename real>
struct camera3
{
    enum class type {
        ORTHOGRAPHIC, // Orthographic projection (objects do not get smaller with distance)
        PERSPECTIVE   // Perspective projection (basic camera defined by field-of-view angle)
    };
}

using camera3f = camera3<float>;
using camera3d = camera3<double>;

} // END namespace obvi
#endif // OBVI_CAMERA3_HPP
