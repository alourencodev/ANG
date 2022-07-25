#version 450

#include <meshCore.vert.glsl>


void main() 
{
    gl_Position = vec4(inPosition, 1.0);
    outFragColor = vec3(0.0, 1.0, 0.0);
}
