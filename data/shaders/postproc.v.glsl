#version 110

attribute vec2 vCoord;
uniform sampler2D fboTexture;
varying vec2 f_texcoord;

void main(void)
{
  gl_Position = vec4(vCoord, 0.0, 1.0);
  f_texcoord = (vCoord + 1.0) / 2.0;
}