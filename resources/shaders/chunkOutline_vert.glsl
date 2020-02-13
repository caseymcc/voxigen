#version 330 core

layout (location = 0) in vec3 inputVertex;
layout (location = 1) in vec3 inputNormal;
layout (location = 2) in vec2 inputTexCoord;
layout (location = 3) in vec4 inputOffset;

out vec3 position;
out vec3 normal;
out vec3 texCoords;
out vec3 cubePos;

uniform mat4 projectionView;
uniform vec3 regionOffset;

void main()
{
//   position=inputOffset.xyz+inputVertex;
   cubePos=inputVertex;
   position=regionOffset+inputOffset.xyz+inputVertex;
   normal=inputNormal;
   gl_Position=projectionView*vec4(position, 1.0);
}