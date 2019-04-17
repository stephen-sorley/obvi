/* Header for main GUI window.
 *
 * Just a basic OpenGL display with no controls (for now).
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
#ifndef OBVI_MAIN_WINDOW_HPP
#define OBVI_MAIN_WINDOW_HPP

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#include <chrono>

#include <obvi/util/affine3.hpp>
#include <obvi/util/camera3.hpp>

namespace obvi {

struct main_window : public QOpenGLWindow, protected QOpenGLFunctions {
    Q_OBJECT

public:
    ~main_window();

    // Functions from QOpenGLWindow that are called by Qt during rendering.
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

private:
    // Event handling.
    void keyPressEvent(QKeyEvent *ev);

    // Helper functions.
    void print_context_info();

    void bind_state();
    void release_state();
    void update_model();
    void update_camera();

    // OpenGL object state.
    QOpenGLBuffer            v_buffer;
    QOpenGLVertexArrayObject v_obj;
    QOpenGLShaderProgram     program;
    int                      loc_model;
    int                      loc_view_proj;
    int                      loc_light_dir_world;
    int                      loc_camera_pos_world;

    // Other object state.
    obvi::affine3f model;
    obvi::camera3f camera;
    bool           model_moved  = false;
    bool           lens_changed = false;
    bool           camera_moved = false;

    bool           animate      = false;

    std::chrono::steady_clock::time_point tstart;
};

} // END namespace obvi

#endif // OBVI_MAIN_WINDOW_HPP
