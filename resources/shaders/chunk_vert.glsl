#version 330 core
//layout (location = 0) in vec3 blockvertex;
//layout (location = 1) in vec3 blockNormal;
//layout (location = 2) in vec2 blockTexCoord;
//layout (location = 3) in vec4 blockOffset;
layout (location = 0) in uvec4 packedPosition;
layout (location = 1) in ivec3 packedNormal;
layout (location = 2) in ivec2 vTexCoords;
layout (location = 3) in uint data;

out vec3 position;
out vec3 normal;
out vec2 texCoords;
out vec3 barycentric;
flat out uint type;

//layout (std140) uniform pos
//{
//   vec4 cameraPos;
//   vec4 lightPos;
//   vec4 lightColor;
//}
uniform mat4 projectionView;
uniform vec3 regionOffset;

void main()
{
//   gl_Position=vec4(blockOffset.xyz+blockvertex, 1.0);
//   position=regionOffset+blockOffset.xyz+blockvertex;
    vec3 decodedPosition=packedPosition.xyz;
    decodedPosition=decodedPosition;
    position=regionOffset+decodedPosition;
//   normal=blockNormal;
    normal=packedNormal;
    normal=normal;
    texCoords=vec2(vTexCoords.x, vTexCoords.y);
//   texCoords=vec3(0.0, 0.0, data);
    type=data;
//   vec2 normTexCoords=mod(texCoords, 4.0f);
//   vec2 normTexCoords=mod(texCoords, 32.0f)*32.0f;
//    vec2 normTexCoords=(texCoords/32.0f);

//    normTexCoords=normTexCoords-floor(normTexCoords);
//   normTexCoords=normTexCoords*100;
//   barycentric=vec3(normTexCoords.x, normTexCoords.y, (normTexCoords.x==normTexCoords.y)?1.0f:0.0f);
    if((packedPosition.w == 1u) || (packedPosition.w == 3u))
        barycentric=vec3(0.0f, 0.0f, 1.0f);
    else if(packedPosition.w == 0u)
        barycentric=vec3(1.0f, 0.0f, 0.0f);
    else if(packedPosition.w == 2u)
        barycentric=vec3(0.0f, 1.0f, 0.0f);

    gl_Position=projectionView*vec4(position, 1.0f);
}
