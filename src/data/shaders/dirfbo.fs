#version 300 es

precision mediump float;

uniform bool u_is_lit;

void main()
{
  if (!u_is_lit)
    discard;
    
  gl_FragDepth = gl_FragCoord.z;
}