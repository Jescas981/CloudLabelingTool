#type vertex
#version 450 core

layout(location = 0) in vec2 a_Position;

void main() {
    // Already in NDC — convert from screen coords in CPU side
    gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_BorderColor;
uniform vec4 u_FillColor;
uniform int  u_IsFill;

void main() {
    o_Color = u_IsFill == 1 ? u_FillColor : u_BorderColor;
}