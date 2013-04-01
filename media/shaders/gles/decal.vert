precision mediump float;
attribute vec4 position;
attribute vec4 color;
attribute vec4 texture;
varying vec4 colorVarying;
varying vec4 textureVarying;
uniform mat4 projection;
uniform mat4 modelView;
uniform int hasTexture;

void main()
{
  gl_Position = projection * modelView * position;
  colorVarying = color;
  textureVarying = texture;
}
