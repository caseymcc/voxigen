#version 330 core

in vec3 position;
in vec3 normal;
in vec3 texCoords;
in vec3 vertexColor;
in vec3 cubePos;

out vec4 color;

uniform vec3 lightPos;
uniform vec3 statusColor;
uniform float lineWidth=1.0;

void main()
{
//   float value=1.0f;
//   vec3 lightColor=vec3(1.0f, 1.0f, 1.0f);
   vec3 distance=(min(cubePos, vec3(64.0, 64.0, 16.0)-cubePos)/vec3(64.0, 64.0, 16.0));
//   vec3 distanceVec=min(cubePos, vec3(64.0, 64.0, 16.0)-cubePos);
//   float distance = min(min(distanceVec.x, distanceVec.y), distanceVec.z);
   vec3 distanceDelta=lineWidth*fwidth(distance);

//   vec3 edgeDistance=distanceDelta;
   vec3 edgeDistance=smoothstep(distanceDelta*0.5, 1.5f*distanceDelta, distance);
//   distance=distance/distanceDelta;
//   float distance=smoothstep(distanceDelta, 2.0f*distanceDelta, distance);
//   float ambientStrength=0.5; 
//   vec3 ambient=ambientStrength * lightColor;
   
   float value=1.0;
   int count=0;

//   if(distance.x<=edgeDistance.x)
   if(edgeDistance.x<1.0f)
   {
       value=min(edgeDistance.x, value);
       count++;
   }
   if(edgeDistance.y<1.0f)
   {
       value=min(edgeDistance.y, value);
       count++;
   }
   if(edgeDistance.z<1.0f)
   {
       value=min(edgeDistance.z, value);
       count++;
   }

   if(count<2)
        discard;

//   color=vec4(distance.x, distance.y, distance.z, 1.0f);
//   color=vec4(edgeDistance.x, edgeDistance.y, edgeDistance.z, 1.0f);
   color=vec4(value, 0.0f, 0.0f, 1.0f);
   
//   if(count<2)
//       discard;

//   value=value/float(count);
//   float edgeIntensity=smoothstep(1.0f, 2.0f, value);
   
//   if(count<2)// || (value<1.0f))
//       discard;

   // diffuse 
//   vec3 lightDir=normalize(lightPos-position); 
//   float diff=max(dot(normal, lightDir), 0.0); 
//   vec3 diffuse=diff*lightColor; 
//   color=vec4(statusColor*(ambientStrength+diff), 1.0f);

   color=vec4(statusColor, 1.0f);
//   color=vec4(value/2.0f, 0.0f, statusColor.x, 1.0f);

   color=vec4(0.0f, 0.0f, 0.0f, 0.0f)*value+(1.0f-value)*color;
//   color.a=smoothstep(lineWidth, lineWidth+distanceDelta, distance);
}