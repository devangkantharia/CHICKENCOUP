#version 300 es

precision mediump float;

uniform bool u_is_lit;

void main()
{
  gl_FragDepth = gl_FragCoord.z;
}