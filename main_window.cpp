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

// TODO: read data from disk instead of relying on hardcoded vertex data below.
namespace {
    static constexpr GLfloat vertex_data[] = {
    //        Position           Color
         0.00f, 0.75f,1.0f,  1.0f,0.0f,0.0f,  //vertex 0
        -0.75f,-0.75f,1.0f,  0.0f,0.0f,1.0f,  //vertex 1
         0.75f,-0.75f,1.0f,  0.0f,1.0f,0.0f,  //vertex 2
    };

    static constexpr size_t num_vertices = sizeof(vertex_data) / (6 * sizeof(vertex_data[0]));

    static constexpr GLfloat mat4_identity[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

obvi::main_window::~main_window() {
    // Clean up OpenGL objects.
    makeCurrent();
    v_obj.destroy();
    v_buffer.destroy();
    program.removeAllShaders();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OpenGL rendering callbacks.
void obvi::main_window::initializeGL() {
    initializeOpenGLFunctions(); // from parent QOpenGLFunctions
    print_context_info(); // for debugging purposes only

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Hardcode a triangle in OpenGL state.
    {
        // Compile and link shader code from resource files we bundled inside the executable.
        //   see: shaders/*.{vert,frag}
        program.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/flat.vert");
        program.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/flat.frag");
        program.link();
        program.bind();
        int loc_position   = program.attributeLocation("position");
        int loc_color      = program.attributeLocation("color");
        int loc_modelview  = program.uniformLocation("mv");
        int loc_projection = program.uniformLocation("mvp");
        int loc_light_dir  = program.uniformLocation("light_dir");
        int loc_diff_frac  = program.uniformLocation("diff_frac");
        int loc_ambi_frac  = program.uniformLocation("ambi_frac");

        // Create buffer to store vertex data in, and fill it with hardcoded values.
        v_buffer.create();
        v_buffer.bind();
        v_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        v_buffer.allocate(vertex_data, int(sizeof(vertex_data)));

        // Create Vertex Array Object, store state values we'll use to draw an object.
        v_obj.create();
        v_obj.bind();

        program.enableAttributeArray(loc_position);
        program.enableAttributeArray(loc_color);
        program.setAttributeBuffer(loc_position, GL_FLOAT, 0, 3, sizeof(float)*6);
        program.setAttributeBuffer(loc_color, GL_FLOAT, sizeof(float)*3, 3, sizeof(float)*6);

        program.setUniformValue(loc_modelview, QMatrix4x4(mat4_identity).transposed());
        program.setUniformValue(loc_projection, QMatrix4x4(mat4_identity).transposed());

        program.setUniformValue(loc_light_dir, 0.0f, 0.0f, 1.0f); //light coming straight from camera
        program.setUniformValue(loc_diff_frac, 0.7f);
        program.setUniformValue(loc_ambi_frac, 0.3f);

        // Unbind everything we just set.
        v_obj.release();
        v_buffer.release();
        program.release();
    }
}

void obvi::main_window::resizeGL(int width, int height) {
    (void)width; (void)height; //TODO: implement this function
}

void obvi::main_window::paintGL() {
    // Clear previous contents of buffer by setting every pixel to the clear color.
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw.
    program.bind();
    {
        v_obj.bind();
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        v_obj.release();
    }
    program.release();
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
