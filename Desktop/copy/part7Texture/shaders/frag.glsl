// Adapted from class code
// ==================================================================
#version 410 core
out vec4 color;

in vec2 v_texCoord;

uniform sampler2D u_Texture;

void main()
{

  vec4 texColor = texture(u_Texture, v_texCoord);

  color = texColor;
}
// ==================================================================
