#version 110
uniform int hasTexture;
void main()
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_FrontColor = gl_Color;
  gl_Position = ftransform();
}
