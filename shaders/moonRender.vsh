#version 440

layout (location = 0) in vec2 posAttr;
layout (location = 1) in vec2 texAttr;
out vec2 texCoords;
out vec3 ray;

void main()
{
  gl_Position = vec4(posAttr.x, -posAttr.y,0,1);  
  texCoords = texAttr;
  ray = vec3((texAttr*2-1), 1);
}
