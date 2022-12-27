// ==================================================================
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aOffset;
//layout (location = 2) in vec2 aOffset;

out vec3 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//uniform vec2 offsets[100];

void main()
{

  //vec2 offset = offsets[gl_InstanceID];
  //vec2 pos = aPos * (gl_InstanceID / 100.0);
  //gl_Position = vec4(pos + aOffset, 1.0);

  mat4 MVP = projection * view * model;

  gl_Position = MVP * (vec4(aPos + aOffset, 1.0f));
  fColor = aColor;
}
// ==================================================================
