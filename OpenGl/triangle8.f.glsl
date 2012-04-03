#version 120

varying vec3 f_color;
uniform float fade;
//here it is. this will be the same for all pixels. we can apply it here to the alpha chanel and
//the triangle will uniformaly fade. in the next itteration, we'll make it fade in and out
//uniformly. enjoy
void main(void) 
{
	gl_FragColor = vec4(f_color.x, f_color.y, f_color.z, fade);
}
