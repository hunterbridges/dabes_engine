precision mediump float;
attribute vec4 position;
attribute vec4 color;
attribute vec4 alpha;
attribute vec4 texture;
attribute mat4 modelView;
varying vec4 colorVarying;
varying vec4 alphaVarying;
varying vec4 textureVarying;
uniform mat4 projection;
uniform int hasTexture;

void main()
{
  gl_Position = projection * (modelView * position);
  colorVarying = color;
  alphaVarying = alpha;
  textureVarying = texture;
}
