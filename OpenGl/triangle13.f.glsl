#version 120

varying vec3 f_color;
//uniform float fade; cut out so that we can see the cube at all times.
void main(void) 
{
	gl_FragColor = vec4(f_color.x, f_color.y, f_color.z, 1.0);
}
