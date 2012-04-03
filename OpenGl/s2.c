/* This next itteration in the code is probably going to be a bit of a jump. There
 * is also another include because I switched the tutorial I was following to this
 * one: http://en.wikibooks.org/wiki/OpenGL_Programming
 * You'll get everything that I'll talk about here from there, But I'll try and 
 * break things down a little more than they did as my first time through it I did
 * not understand it all either. This stuff is hard, its programming in a way that
 * most of us have never had to code before. Take your time, move on when you feel
 * like you know what is going on. The new include, glew.h, adds a couple prototypes
 * that are needed for us to get everything we need. To ubuntu users, add -lGLEW to
 * your compile command and it will work just fine. one other little note, GLEW must
 * be imported before glut. or it will cause errors.
 * as for the addition of stdio and stdlib, we will use a couple functions of theirs
 * and they are handy to have around, so they are included now too.
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>


GLuint program;
GLint attribute_coord2d;
//These are global variables. declared up here they are accessable and editable by
//any part of the program. You'll notice that their type is "GL___" these are types
//that we get from glut or from glew that are easily interfaced over to the gpu. 
//you can use them more or less exactly as you would use a normal unsigned int or
//int or float, etc. obviously they come with extra overhead, so use them only when
//you have to. though i feel like that was a given.
//So, GLuint program. as you my guess this is going to be our reference to our
//program. Java, Python, VB programs should all have a problem with this. it isn't
//of type "GLprogram" it is an unsigned int. how can that be a program? welcome to
//the world of C. In higher level languages it is just hidding from you what is
//actually going on. Objects in Java, python, VB, basically any object orriented 
//programing language, is not actually the object, but a pointer to its location. 
//in this case, we have the same thing. see a pointer is just an unsigned integer
//that points to a location in memory. in this case, it points to a location in 
//the GPU's memory. later in this itteration, we will be creating whats called a
//shader program. this is the program that will be responsible for finding what 
//pixels need to be shaded and how they need to be shaded. 
//Similarly, attribute_coord2d is a pointer to a location on the GPU where an attrib
//can be found. This will also come in to play later in the code, so bear with me for
//now. I hope this all starts to make sense soon.


void free_resources()
{
	glDeleteProgram(program);
}
//2 new methods. free_resources, and init_resources. the tutorial I'm following has
//these set aside as individual function calls. even though free_resources only 
//makes one function call. that could very likely change in the future. you'll 
//notice that this is actually called /after/ the glutMainLoop. "but I thought you 
//said that that glutMainLoop never returns ever?" well, turns out it never returns
//while the program is running. after the window closes, yes, the glutMainLoop
//returns. here it is best practice to do any of the clean up you need to do. it is
//smart to put it all into a single function so that you can keep track and the
//actuall cleaning process and your main method clean and easy to read.


int init_resources()
{
//this method has quite a bit of new information in it. do your best to hang in there
//I'll do my best to explain every little detail.

	GLint compile_ok = GL_FALSE;
	GLint link_ok = GL_FALSE;
	//in openGL, as well as openCL, they don't given you a lot of things unless
	//you explicitly ask for it. this saves time and processor power. but, when
	//you want to get information, like say about whether or not a kernel compiled
	//or something of that nature, you have to have some where to store that info
	//when it is passed back to you. These two GLints are here to hold that info.
	//of course, GL___ means that it is part of the openGL interface. here, incase
	//you didn't know, GL_FALSE is a constant. It is an enumerated type. Because
	//the specification of a boolean never was decided on within C, it isn't 
	//uncommon to see things like this where people have decided that some 
	//arbitrary value is now true or false. we are initializing compile_ok and
	//link_ok here to be false because if all goes well, they will be flipped over
	//to GL_TRUE. but they could also be flipped to one of several error messages.
	//so it is the safe thing to do to start them off at GL_FALSE.

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	//here we actually begin the process of creating our shader program. we start
	//by creating a vertex shader. direct quote from wiki:
//Vertex shaders are run once for each vertex given to the graphics processor. The purpose is to transform each vertex's 3D position in virtual space to the 2D coordinate at which it appears on the screen (as well as a depth value for the Z-buffer). Vertex shaders can manipulate properties such as position, color, and texture coordinate, but cannot create new vertices. The output of the vertex shader goes to the next stage in the pipeline, which is either a geometry shader if present or the rasterizer otherwise.
	//so, inother words. the vertex shader is a per vertex shader. the code in 
	//the vertex shader will be run on each vertex, and many of them in parellel.
	//there are two possible enumerated types you can send to this method.
	//GL_VERTEX_SHADER and GL_FRAGMENT_SHADER. specifics can be found here:
	//http://www.opengl.org/sdk/docs/man/xhtml/glCreateShader.xml

	const char *vs_source =
		"#version 120                           \n"
		"attribute vec2 coord2d;                  "
		"void main(void) {                        "
		"  gl_Position = vec4(coord2d, 0.0, 1.0); "
		"}";
	//the code for these shaders is typically created in run time. this allows
	//the code to be transportable between different graphics cards and even
	//different devices. the same shader code can work the same way on your ipod,
	//android device, and desktop. For now we will just hard code these in place,
	//but it is better, and more common, to have them in seperate files that you
	//read in during run time. but for simplicity sake, we will hard code them.

	glShaderSource(vs, 1, &vs_source, NULL);
	//glShaderSource. this method is used to give or replace source to your
	//shader objects. Its arguments are as follows:
	//	Arg 1: Shader, "vs", GLunit, it is the pointer to the shader you want
	//		the operation to happen on.
	//	Arg 2: count, "1", GLsizei, this specifies the number of elements in the next two args.
	//		here it is 1 because there is only one string being passed.
	//	Arg 3: string, "&vs_source", GLchar**, is an array of strings that are the source code
	//		for your shader programs.
	//	Arg 4: length, "NULL", GLint*, this is an array of ints that hold the lengths of each 
	//		of the strings in you string array from arg 3. if you pass Null, it will assume
	//		that all of your strings are null terminated. in most cases they probably will
	//		be, but if they are ever not, make sure you tell it this or it will just go on
	//		forever trying to find where the string ends.

	glCompileShader(vs);
	//self explanitory. this does the actual compiling of the shader. the argument
	//is the reference to the shader. it has no return, so if you want to find out
	//about how the compile process went, that would be the next command.

	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
	//this command is used to query shader objects. it has 3 arguements, the shader
	//what you want to know, and where to put the value. there are 5 things that you
	//can ask: GL_SHADER_TYPE, GL_DELETE_STATUS, GL_COMPILE_STATUS, 
	//GL_INFO_LOG_LENGTH, and GL_SHADER_SOURCE_LENGTH. these are all relatively 
	//self explanitory, so I won't go into further detail. but if you want more:
	//http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetShaderiv.xml
	//in this case we are asking what is the compile status of the shader. it will
	//return a true or false for if it has been compiled yet or not. this is
	//obviously valueable in this situation where the shader has just been made.
	//but if you have many many shader objects and wanted to not compile them till
	//needed. you could store them in an array and before each use check, have you
	//been compiled? if not, compile it quickly and then continue on your marry way.
	if (!compile_ok) 
	{
		fprintf(stderr, "Error in vertex shader\n");
		return 0;
	}
	//this is just checking to see if it has compiled. if it hasn't, no point in
	//continueing the program. when we get to more complicated thing of course we 
	//would actually start doing contingency plans incase the shaders didn't work.
	//but for now, if it doesn't compile, there is something supremely screwed up.

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	//another reference to another shader. this time we are making the fragment
	//shader. to wiki again:
//Pixel shaders, also known as fragment shaders, compute color and other attributes of each pixel. Pixel shaders range from always outputting the same color, to applying a lighting value, to doing bump mapping, shadows, specular highlights, translucency and other phenomena. They can alter the depth of the pixel (for Z-buffering), or output more than one color if multiple render targets are active. A pixel shader alone cannot produce very complex effects, because it operates only on a single pixel, without knowledge of a scene's geometry.
	//these are your per-pixel operations. this is were post-processing happens.
	//for now we are just using it to do the actual painting of the object. 
	//you can get information about each pixel, where it is for example can be
	//found with the function gl_FragCoord.xy which returns a vector object
	//of 2: [x,y]. in this case we are modifying a vector object, gl_FragColor,
	//to have the values of the color we want. now, the thing that you must
	//remember though is this, this fragment shader only gets passed to the
	//pixels that the vertex shader above decided need to have it. so only the
	//pixels that fall inside of our triangle will actually get drawn.

	const char *fs_source =
		"#version 120           \n"
		"void main(void) {        "
		"  gl_FragColor[0] = 0.0; "
		"  gl_FragColor[1] = 0.0; "
		"  gl_FragColor[2] = 1.0; "
		"}";
	//here again is the source code for this shader. same basic idea as the last.

	glShaderSource(fs, 1, &fs_source, NULL);
	//set the shader source code to the shader object. 

	glCompileShader(fs);
	//complie the shader.

	glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
	//and lastly see if it compiles correctly.
	if (!compile_ok) 
	{
		fprintf(stderr, "Error in fragment shader\n");
		return 0;
	}

	//two more chunks to go:
	program = glCreateProgram();
	//this creates a program objet. it returns a reference to it that we will
	//use later when we start linking all the peices together. if this function
	//returns a 0, that means that no program object was created. this can 
	//happen because of memory issues amongst other things. the odds of it
	//failing here are relatively slim. so we are just going to assume.

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	//this is used to attach shader objects to your program, if you hadn't guessed.
	//the first argument is the program to attach to, and the second is the shader
	//to attach. program objects are used to link shaders together. because we want
	//both vertex and fragment shaders in our program, we attach them here as such.
	//i'm just guessing here, but I bet that when you change graphics levels in 
	//video games, what is happening is it is changing around what shaders are being
	//used. going to ones that require less horse power so the game can run faster.

	glLinkProgram(program);
	//a lot happens in this step, none of which is particularly important right now.
	//details: http://www.opengl.org/sdk/docs/man/xhtml/glLinkProgram.xml
	//what you need to know is that this is the step at which point the program is
	//actually made. everything is linked together and ready to roll as of now.

	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) //assuming somehting isn't wrong. same old drill for this step.
	{
		fprintf(stderr, "glLinkProgram:");
		return 0;
	}

	//this next, and last, chunk of code does an important step. it is binding an 
	//attribute. in our vertex shader's source code above you can see how it has
	//the line "attribute vec2 coord2d;" in it. this is declaring an attribute of
	//type vec2 named coord2d. I'm not exactly sure how this works, but what it
	//seems to be, is that this attribute is what is changing for all of the runs
	//of the vertex shader. in other words, it is what changes form vertex to vertex.
	//You'll see more in our new display function shortly.
	const char* attribute_name = "coord2d";
	//we will be referencing the attribute by name. the vertex shader's source has
	//it named "coord2d" so we have to call the same name. 
	
	attribute_coord2d = glGetAttribLocation(program, attribute_name);
	//this does the actual calling to find the location. just as with program it
	//returns to us a refernce that we can use to reference this shader later. 
	if (attribute_coord2d == -1) 
	{//and this of course is to check and make sure everything is as we want it.
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}

	return 1;
	//thats it. our resources have been initalized. now on to displaying stuff!
}	

//this is the function that will actually do the displaying. it is responsible for every
//thing getting painted. 
void display() 
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//when you go to draw a new picture, the first thing you need to do is clear
	//the buffer. or you get what happens in the first itteration where it holds
	//on to things arbirarily and pointlessly. but you can't just clear with out
	//saying what you are going to clear to. there actually is no default here.
	//so this method tells openGL what color it is to clear buffers too.

	glClear(GL_COLOR_BUFFER_BIT);
	//glClear is the method used to clear a buffer. there are only 3 buffers that
	//you can clear in this manner: GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, and 
	//GL_STENCIL_BUFFER_BIT. What each of these buffers are and do you will learn
	//later. as well as tricks you can do that effect glClear.

	glUseProgram(program);
	//declares that THIS IS THE PROGRAM that will be used to render. SHAZAM!
	//at this point, your gpu is now using your rendering pipeline as its instruction
	//to create the pictures on your screen. hurray!
	//more details here: http://www.opengl.org/sdk/docs/man/xhtml/glUseProgram.xml

	glEnableVertexAttribArray(attribute_coord2d);
	//this enables the vertex attribute array that we reference with the argument.
	//When this has been done the values in the vertex attribute array will then
	//be accessed and used for rendering. 

	GLfloat triangle_vertices[] = 
	{
		 0.0,  0.8,
		-0.8, -0.8,
		 0.8, -0.8,
	};
	//this is just a simple array of floats used to declare the vertices of the triangle.

	//we now have to describe what our vertcy array looks like to openGL:
	glVertexAttribPointer(
		attribute_coord2d, // the attribute that we are discribing.
		2,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		triangle_vertices  // pointer to the C array
	);
	//now I shall explain what just happened, in my classic style:
	//	Arg 1: index, "attribute_coord2d", GLuint. the index of the attribute of concern
	//	Arg 2: size, "2", GLint. the number of elements or components per generic vertex
	//		attribute. must be 1,2,3, or 4. the default is 4.
	//	Arg 3: type, "GL_FLOAT", GLenum. This used to tell openGL what type the arrray is of
	//	Arg 4: normalized, "GL_FALSE", GLboolean. should a fix point value be normalized
	//		or just converted directly as a fixed-point value when accessed. when
	//		it says "fixed-point" it is refering to the value as it is stored. is it
	//		a fixed point number or a floating point number. wiki them if you don't
	//		know the difference. right now just nod, it is usually going to be false.
	//	Arg 5: stride, 0, GLsizei. how many bytes off set are there between each vertex.
	//		this is used when you start interlacing data. for example if I were to
	//		store velocity data in this array as well as location. the velocity matters
	//		not to the vertex shader, so but we need it there. so we have to tell it
	//		how many bytes it has to skip to get past that data and onto the next vertex.
	//	Arg 6: pointer, "triangle_vertices", const GLvoid*. this is a pointer to the array
	//		of data of which we are concerned. this array is stored on the system main
	//		memory and not on the GPU. important little note.
	//Now that that has been done, it knows how to convert triangle_vertices in to coord2d for
	//each of the vertices. 

	glDrawArrays(GL_TRIANGLES, 0, 3);
	//this is where the actual draw step happens. the values get pushed to the vertex shader
	//and the vertex shader figures out what pixels to call the fragment shader on. and it
	//draws it to whats called the back buffer. I'll explain that in just a moment.

	glDisableVertexAttribArray(attribute_coord2d);
	//don't actually know why this is here. best practice probably. keeps the memory nice
	//and clean and prevents other possible errors.

	glutSwapBuffers();
	//last step is this one. swap buffers. what happens here is what was going on with the back
	//buffer and when i said that display mode needed to be double buffered in the last 
	//itteration. You can find more information on this very easily online, but i'll just say
	//this short and sweet. double buffering is so that you can have 1 buffer being displayed
	//from and the other buffer being drawn into. this way a complete image is always being
	//displayed. 

	//done. you are now displaying stuff. but we need to update the main method a bit.
}

int main(int argc, char *argv[])
{
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
        glutInitWindowSize(800,600);
        glutCreateWindow("My First Triangle");
	//these 4 don't change.

	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{//this is added in to init GLEW, just nod.
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 1;
	}

	if (init_resources())
	{//we don't want to display unless the resources are actually initialized.
		//this conditional makes sure of such.
	        glutDisplayFunc(display);//and these 2 don't really change
	        glutMainLoop();
	}

	free_resources();
	//lastly add this bit to free up the resouces and we're done.
        return 0;
}
/*.S.D.G*/
