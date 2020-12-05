#version 440

#define M_TO_AU       (1.0 / 1.496e+11)
#define MOON_RADIUS     1737100.0 // meters
#define SUN_RADIUS    696340000.0 // meters
#define OUTPUT_RADIUS 0.8 // radius of moon within rendered area
#define DIFFUSE_COLOR vec3(1.0, 1.0, 1.0)
#define AMBIENT_COLOR vec3(0.005, 0.005, 0.005)
#define BG_COLOR      vec3(0.05, 0.05, 0.05)

uniform vec3 baseColor;
uniform vec3 moonPos;
uniform vec3 sunPos;
in vec2 texCoords;
in vec3 ray;
layout(location = 0) out vec4 colorOut;

vec3 blinn(in vec3 rDir, in vec3 norm, in vec3 lPos)
{
  vec3 pos = moonPos + norm*MOON_RADIUS*M_TO_AU; // 3D position of pixel
  vec3 L = normalize(lPos - pos);                // vector from pixel to light source
  float NdotL = dot(L, norm);
  return clamp(NdotL, 0.0, 1.0)*baseColor + AMBIENT_COLOR;
}

void main()
{
  /* vec3 moonDir = normalize(moonPos); */
  vec2 diff = ray.xy;// - moonDir.xy;
  float dist = sqrt(diff.x*diff.x + diff.y*diff.y);
  float theta = atan(ray.y, ray.x);

  float z = -OUTPUT_RADIUS*sqrt(1 - (dist*dist)/(OUTPUT_RADIUS*OUTPUT_RADIUS));
  vec3 norm = normalize(vec3(dist*vec2(cos(theta),sin(theta)), z));

  vec3 col = (dist < OUTPUT_RADIUS ? blinn(ray, norm, sunPos) : BG_COLOR);
  colorOut = vec4(col, 1);
}
