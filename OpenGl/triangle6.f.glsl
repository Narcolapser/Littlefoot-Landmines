#version 120

varying vec3 f_color;
//a varying variable is vertex shader's output and a fragment shader's input. this is how info is
//passed between the two. the other important bit is that it is interpolated. this means that
//the values in f_color are varried according to a pixel's proximaty to each vertex. when you run
//this, you'll see that amazingly that there is an even color gradient from one vertex to the next.
//where did this happen? you might ask. I kinda did... well it happens on in the step between the
//vertex and the fragment shader. I don't know about you, but I'm kinda amazed.
void main(void) 
{
	gl_FragColor = vec4(f_color.x, f_color.y, f_color.z, 1.0);
}
//you may have also noticed, as i kind of said it right at the begining, there was no change to the
//c file. that is because there is no interface from c code to the varying. varyings are used only
//as communication between vertex and fragment shaders. if you want to pass information to a 
//fragment shader, it must be done in this manner. 
