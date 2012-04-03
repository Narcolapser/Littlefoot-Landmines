#version 120

attribute vec2 coord2d;
attribute vec3 v_color;
//added the attribute here. thats the only change. but from here we will pass it on to the fragment
//shader in the next commentary. go read that one.
void main(void) 
{
	gl_Position = vec4(coord2d, 0.0, 1.0);
}
