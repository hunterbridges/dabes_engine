#version 110
attribute vec4 position;
attribute vec4 texPos;
uniform vec2 stretch;
uniform mat4 projection;
uniform mat4 modelView;
varying vec2 texPosVarying;

void main()
{
  gl_Position = projection * (modelView * position);
  texPosVarying = texPos.xy * stretch;
}
