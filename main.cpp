/* Main executable for Obvi.
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

#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

#include <algorithm>

#include "main_window.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    obvi::main_window mainwin;

    // Set our required OpenGL type and version.
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile); // Leave out old stuff from before OpenGL 3.
    format.setVersion(4,3); // OpenGL 4.3
    format.setSamples(4); // Enable multisampling.
    mainwin.setFormat(format); // MUST be called BEFORE show().

    // Set window size (use fraction of screen geometry, so it is independent of screen resolution).
    QRect screen_size = mainwin.screen()->availableGeometry();
    int   dim         = int(std::min(screen_size.width(), screen_size.height()) * 0.7 + 0.5);
    mainwin.resize(dim, dim);

    // Make the window visible (begins OpenGL rendering), and start the app's main event loop.
    mainwin.show();
    return app.exec();
}
