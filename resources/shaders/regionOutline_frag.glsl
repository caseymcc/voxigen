#version 330 core

in vec3 position;

out vec4 color;

uniform vec3 statusColor;
uniform float lineWidth=0.2;

void main()
{
   vec3 distance=min(position, vec3(1024.0, 1024.0, 256.0)-position);
   float ambientStrength=0.5; 
   
   int count=0;
   if(distance.x < lineWidth)
       count++;
   if(distance.y < lineWidth)
       count++;
   if(distance.z < lineWidth)
       count++;
   if(count<2)
       discard;
   color=vec4(statusColor, 1.0f);
}