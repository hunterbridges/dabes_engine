precision highp float;
attribute vec4 position;
attribute vec4 color;
attribute vec4 texPos;
attribute mat4 modelView;
varying vec4 colorVarying;
varying vec4 texPosVarying;
uniform mat4 projection;

void main()
{
  gl_Position = projection * modelView * position;
  colorVarying = color;
  texPosVarying = texPos;
}
