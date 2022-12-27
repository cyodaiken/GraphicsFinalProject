// ==================================================================
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 aOffset;

out vec2 v_texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{

  mat4 MVP = projection * view * model;

  gl_Position = MVP * (vec4(aPos + aOffset, 1.0f));

  v_texCoord = texCoord;
}
// ==================================================================
