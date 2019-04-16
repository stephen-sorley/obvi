/* Implementation of main GUI window.
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
#include "main_window.hpp"

#include <QDebug>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OpenGL rendering callbacks.
void obvi::main_window::initializeGL() {
    initializeOpenGLFunctions(); // from parent QOpenGLFunctions
    print_context_info(); // for debugging purposes only

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void obvi::main_window::resizeGL(int width, int height) {
    (void)width; (void)height; //TODO: implement this function
}

void obvi::main_window::paintGL() {
    // Clear previous contents of buffer by setting every pixel to the clear color.
    glClear(GL_COLOR_BUFFER_BIT);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Private helper functions.
void obvi::main_window::print_context_info() {
    // Code taken from here: https://www.trentreed.net/blog/qt5-opengl-part-0-creating-a-window/

    // Get version info.
    const char* glType    = (context()->isOpenGLES())? "OpenGL ES" : "OpenGL";
    const char* glVersion = (const char *)glGetString(GL_VERSION);

    const char* glProfile = "";
    switch(format().profile()) {
        case QSurfaceFormat::NoProfile:            glProfile = "(NoProfile)"; break;
        case QSurfaceFormat::CompatibilityProfile: glProfile = "(CompatibilityProfile)"; break;
        case QSurfaceFormat::CoreProfile:          glProfile = "(CoreProfile)"; break;
    }

    // Print out as debug info.
    qDebug() << glType << glVersion << glProfile;
}
