#version 430 core
in vec4 f_color;
in vec4 f_pos_world;
layout(location=0) out vec4 out_color;

uniform vec3  camera_pos_world; //location of camera in world coords
uniform vec3  light_dir_world; //direction that light source is shining into the scene (world coords).
uniform float diff_frac;
uniform float ambi_frac;

void main() {
    // Set initial brightness to ambient component.
    float brightness = ambi_frac;

    // Compute face normal (flat shading), by calculating two perpendicular vectors that
    // are tangent to the surface at this point, then taking their cross-product.
    vec3 xtan = dFdx(f_pos_world.xyz);
    vec3 ytan = dFdy(f_pos_world.xyz);
    vec3 normal = cross(xtan,ytan);

    // Only compute diffuse component if light is shining on the side of the face that the camera
    // can see.
    float light_cosang = dot(light_dir_world, normal);
    if(sign(light_cosang) == sign(dot(f_pos_world.xyz - camera_pos_world, normal))) {
        // Compute cosine of angle between light direction and face - this makes the diffuse
        // component largest when face is pointed directly at light, and smaller when it's at
        // an angle. Note that light_dir is already normalized.
        float cosang = abs(light_cosang) * inversesqrt(dot(normal,normal));
        // Combine ambient and diffuse components to get final color.
        brightness += diff_frac * cosang;
    }

    out_color.rgb = brightness * f_color.rgb;
    out_color.a   = 1.0f; // alpha=1 (fully opaque).
}
