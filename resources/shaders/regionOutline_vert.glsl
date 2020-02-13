#version 330 core

layout (location = 0) in vec3 inputVertex;
layout (location = 1) in vec3 inputNormal;
layout (location = 2) in vec2 inputTexCoord;

uniform mat4 projectionView;
uniform vec3 regionOffset;

out vec3 position;

void main()
{
   position=inputVertex;
   gl_Position=projectionView*vec4(position+regionOffset, 1.0);
}