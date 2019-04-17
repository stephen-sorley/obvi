#version 430 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
out vec4 f_color;
out vec4 f_pos_world;

uniform mat4 model;     // model matrix: transforms from object coords to world coords
uniform mat4 view_proj; // projection * view matrix: transforms from world coords to clip coords

void main() {
    f_color = vec4(color, 1.0f);

    f_pos_world = model * vec4(position, 1.0f);
    gl_Position = view_proj * f_pos_world;
}
