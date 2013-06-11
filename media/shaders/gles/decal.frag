precision mediump float;
varying vec4 colorVarying;
varying vec4 textureVarying;
uniform int hasTexture;
uniform sampler2D texture;
uniform vec2 textureOffset;
uniform vec2 textureSize;
uniform vec2 potScale;

void main()
{
  vec2 texCoord = vec2(textureVarying.x, textureVarying.y);
  texCoord = texCoord * textureSize;
  texCoord = texCoord + textureOffset;
  texCoord = texCoord * potScale;
  texCoord = texCoord + texelSize / 4.0;
  vec4 mixed = colorVarying;
  if (hasTexture != 0) {
    vec4 texColor = texture2D(texture, texCoord);
    mixed = texColor + colorVarying * (1.0 - texColor.a);
  }
  gl_FragColor = mixed;
}
