#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texCoords;
in vec3 barycentric;
flat in uint type;
out vec4 color;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform sampler2D textureSampler;
uniform int options=0;

void main()
{
   // ambient
   float ambientStrength=0.5;
   vec3 ambient=ambientStrength * lightColor;
   
   // diffuse 
   vec3 lightDir=normalize(lightPos-position); 
   float diff=max(dot(normal, lightDir), 0.0); 
   vec3 diffuse=diff * lightColor; 
   
   color=texelFetch(textureSampler, ivec2(texCoords), 0);

   //wireframe
   if(options==1)
   {
       float edgeDistance=min(min(barycentric.x, barycentric.y), barycentric.z);
       float distDelta=fwidth(edgeDistance);
       float edgeIntensity=smoothstep(distDelta, 2.0f*distDelta, edgeDistance);

       color=color*edgeIntensity+(1.0f-edgeIntensity)*vec4(0.0f, 0.5f, 1.0f, 1.0f);
   }

}