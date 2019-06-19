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

#include <QKeyEvent>
#include <QDebug>

#include <obvi/util/bbox.hpp>

// TODO: read data from disk instead of relying on hardcoded vertex data below.
namespace {
    static constexpr GLfloat vertex_data[] = {
    //        Position           Color
         0.00f, 0.75f,0.0f,  1.0f,0.0f,0.0f,  //vertex 0
        -0.75f,-0.75f,0.0f,  0.0f,0.0f,1.0f,  //vertex 1
         0.75f,-0.75f,0.0f,  0.0f,1.0f,0.0f,  //vertex 2
    };

    static constexpr size_t num_vertices = sizeof(vertex_data) / (6 * sizeof(vertex_data[0]));
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
    connect(this, SIGNAL(frameSwapped()), this, SLOT(update())); // continuously redraw (sync'd to refresh rate if Vsync enabled)
    print_context_info(); // for debugging purposes only

    // Point camera at center of object.
    {
        bboxf box;
        for(size_t i = 0; i < num_vertices; ++i) {
            const float *vtx = vertex_data + i * 6;
            box.expand(vec3f(vtx[0],vtx[1],vtx[2]));
        }
        vec3f center = box.center();
        vec3f camera_pos = center - vec3f(0,0,3);
        camera.look_at(camera_pos, center, vec3f(0,1,0));
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Hardcode a triangle in OpenGL state.
    {
        // Compile and link shader code from resource files we bundled inside the executable.
        //   see: shaders/*.{vert,frag}
        program.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/flat.vert");
        program.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/flat.frag");
        program.link();
        program.bind();
        int loc_position     = program.attributeLocation("position");
        int loc_color        = program.attributeLocation("color");

        loc_model            = program.uniformLocation("model");
        loc_view_proj        = program.uniformLocation("view_proj");
        loc_light_dir_world  = program.uniformLocation("light_dir_world");
        loc_camera_pos_world = program.uniformLocation("camera_pos_world");
        int loc_diff_frac    = program.uniformLocation("diff_frac");
        int loc_ambi_frac    = program.uniformLocation("ambi_frac");

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

        program.setUniformValue(loc_diff_frac, 0.7f);
        program.setUniformValue(loc_ambi_frac, 0.3f);

        // Unbind everything we just set.
        v_obj.release();
        v_buffer.release();
        program.release();
    }

    model_moved  = true;
    camera_moved = true;
    lens_changed = true;
}

void obvi::main_window::resizeGL(int width, int height) {
    (void)width; (void)height;
    lens_changed = true;
}

void obvi::main_window::paintGL() {
    // Clear previous contents of buffer by setting every pixel to the clear color.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bind_state();
    {
        // Send new model, view and projection matrices to GPU, if any have changed.
        update_model();
        update_camera();

        // Draw the mesh.
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
    release_state();

    glFinish(); // Minimizes screen tearing when window is resized.
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Event handling.
void obvi::main_window::keyPressEvent(QKeyEvent *ev) {
    if(!ev) {
        return;
    }
    switch(ev->key()) {
        // Toggle animation on/off.
        case Qt::Key_A:
        if(!animate) {
            animate = true;
            tstart = std::chrono::steady_clock::now();
            update();
        } else {
            animate = false;
        }
        break;
    }
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

void obvi::main_window::bind_state() {
    program.bind();
    v_obj.bind();
}

void obvi::main_window::release_state() {
    v_obj.release();
    program.release();
}

void obvi::main_window::update_model() {
    if(animate) {
        static constexpr float two_pi      = 2.0f * pi<float>;
        static constexpr float rot_per_sec = 0.5f;
        auto tend = std::chrono::steady_clock::now();
        std::chrono::duration<float> fsec = tend - tstart;

        model.set(model.rotation() * mat3f::yrot(two_pi * rot_per_sec * fsec.count()),
            model.translation());
        tstart = tend;
        model_moved = true;
    }
    if(model_moved) {
        std::array<float, 16> mat_model;
        model.to_gl(mat_model);
        program.setUniformValue(loc_model, QMatrix4x4(mat_model.data()).transposed());
    }
    model_moved = false;
}

void obvi::main_window::update_camera() {
    if(lens_changed) {
        camera.set_perspective(deg2rad(45.0f), float(width()) / float(height()), 1.0f, 1e5f);
    }
    if(camera_moved) {
        // Update camera position.
        const vec3f& camera_pos = camera.get_position();
        program.setUniformValue(loc_camera_pos_world, camera_pos[0], camera_pos[1], camera_pos[2]);
        // Update light direction (light pointed in same direction as camera).
        vec3f look_dir = camera.get_look_dir();
        program.setUniformValue(loc_light_dir_world, look_dir[0], look_dir[1], look_dir[2]);
    }
    if(lens_changed || camera_moved) {
        std::array<float, 16> mat_view_proj;
        camera.to_gl(mat_view_proj);
        program.setUniformValue(loc_view_proj, QMatrix4x4(mat_view_proj.data()).transposed());
    }
    lens_changed = false;
    camera_moved = false;
}
