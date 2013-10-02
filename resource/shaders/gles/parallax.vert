precision mediump float;
attribute vec4 position;
attribute vec4 texture;
varying vec2 textureVarying;
uniform mat4 projection;
uniform mat4 modelView;

void main()
{
  gl_Position = projection * modelView * position;
  vec2 texCoord = vec2(texture.x, texture.y);
  textureVarying = texCoord;
}
