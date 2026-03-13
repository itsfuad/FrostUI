#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec4 frag_color;

layout(push_constant) uniform PushConstants {
    vec2 viewport_size;
} pc;

void main() {
    vec2 ndc = vec2(
        (in_pos.x / pc.viewport_size.x) * 2.0 - 1.0,
        (in_pos.y / pc.viewport_size.y) * 2.0 - 1.0
    );

    gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
    frag_uv = in_uv;
    frag_color = in_color;
}
