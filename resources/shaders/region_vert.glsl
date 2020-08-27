#version 330 core
layout (location = 0) in uvec3 packedPosition;
layout (location = 1) in ivec2 vTexCoords;
layout (location = 2) in uint data;

out vec3 position;
out vec2 texCoords;
flat out uint type;

uniform mat4 projectionView;
uniform vec3 regionOffset;

void main()
{
   vec3 decodedPosition=packedPosition;
   decodedPosition=decodedPosition;
   position=regionOffset+decodedPosition;
   texCoords=vec2(vTexCoords.x, vTexCoords.y);
   type=data;
   gl_Position=projectionView*vec4(position, 1.0);

}