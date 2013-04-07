#version 110
uniform int hasTexture;
uniform sampler2D tex;
void main()
{
  vec4 texColor = texture2D(tex, gl_TexCoord[0].st);
  vec4 mixed = gl_Color;
  if (hasTexture != 0) {
    mixed = texColor + gl_Color * (1.0 - texColor.a);
  }
  gl_FragColor = mixed;
}