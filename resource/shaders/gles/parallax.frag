precision lowp float;
varying vec2 textureVarying;
uniform vec2 repeatSize;

uniform vec2 cameraPos;
uniform float parallaxFactor;
uniform float texScale;
uniform float repeats;
uniform float xShift;

uniform sampler2D texture;
uniform vec2 texPortion;

uniform vec2 origPixel;
uniform float cascadeTop;
uniform float cascadeBottom;

void main()
{
  vec2 texCoord = textureVarying;
  /*
  float cascade = mix(cascadeTop, cascadeBottom, texCoord.y);
   */
  float slope = cascadeBottom - cascadeTop;
  float snapY = texCoord.y - mod(texCoord.y, origPixel.y);
  float cascade = cascadeTop + slope * snapY;
  
  
  texCoord.x = (cameraPos.x * parallaxFactor * cascade * texScale) +
    (xShift * texScale) - 0.618 + texCoord.x;
  texCoord.x = mod(texCoord.x, repeatSize.x) * repeatSize.y;
  
  vec2 realCoord = texCoord * texPortion;
  vec4 texColor = texture2D(texture, realCoord);
  gl_FragColor = texColor;
}
