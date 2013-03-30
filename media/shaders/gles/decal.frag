precision lowp float;
varying vec4 colorVarying;
varying vec4 textureVarying;
uniform int hasTexture;
uniform sampler2D texture;

void main()
{
  vec2 texCoord = vec2(textureVarying.x, textureVarying.y);
  vec4 mixed = colorVarying;
  if (hasTexture != 0) {
    vec4 texColor = texture2D(texture, texCoord);
    mixed = texColor + colorVarying * (1.0 - texColor.a);
  }
  gl_FragColor = mixed;
}
