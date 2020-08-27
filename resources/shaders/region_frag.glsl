#version 330 core

in vec3 position;
in vec2 texCoords;
flat in uint type;
out vec4 color;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform sampler2D textureSampler;

void main()
{
   vec3 normal = cross(dFdy(position), dFdx(position));
   normal=normalize(normal);

   // ambient
   float ambientStrength=0.5;
   vec3 ambient=ambientStrength * lightColor;
   
   // diffuse 
   vec3 lightDir=normalize(lightPos-position); 
   float diff=max(dot(normal, lightDir), 0.0); 
   vec3 diffuse=diff * lightColor; 
   color=texelFetch(textureSampler, ivec2(texCoords), 0);
}