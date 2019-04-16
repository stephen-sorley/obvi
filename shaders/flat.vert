#version 430 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
out vec4 f_color;
out vec4 f_view_pos;

uniform mat4 modelview;  // modelview matrix: transforms from object coords to eye coords
uniform mat4 projection; // projection matrix: transforms from eye coords to clip coords

void main() {
    f_color = vec4(color, 1.0f);
    //f_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);

    //f_view_pos = modelview * vec4(position, 1.0f);
    //gl_Position = projection * f_view_pos;
    f_view_pos = vec4(position, 1.0f);
    gl_Position = f_view_pos;
}
