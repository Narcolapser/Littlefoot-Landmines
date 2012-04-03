/* this iteration will introduce the glutIdleFunc. it will be the first steps towards animation. 
 * what we are going to do is fade the triangle in and out. its not a complicated animation, but it
 * will introduce you to the basic concept of animation. what happens is that when glut has 
 * finished all the tasks involved with doing a frame, it then just sits and does nothing. in the
 * case of our program it never even has to redraw. but in a video game, in the time between the
 * frames a lot of updating needs to happen and then the screen redrawn. wouldn't it be nice if 
 * there was a way to accomplish this updating and requesting of redrawing? well i'm not going to
 * suprise you by saying of course there is. it is the glutIdleFunc. just like glutDisplayFunc you
 * pass the function you want to be the one that does all the drawing process. glutIdleFunc is the
 * function you wnat glut to call when it has nothing to do. this is where you will typically do
 * all of your updating. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>//also included now is math.h. this is because we use the "sinf" function later.
	//when compiling be sure to include "-lm" now in your compile line. 
#include <GL/glew.h>
#include <GL/glut.h>


GLuint program;
GLint attribute_coord2d, attribute_v_color;
GLuint vbo_triangle, vbo_triangle_colors;
GLint uniform_fade;

//i've left the comments in these 3 functions because they are more general comments.
char* file_read(const char* filename)
{
	FILE* in = fopen(filename, "rb");
	//open the file that was requested
	if (in == NULL) return NULL;
	//if the file doesn't open, there is nothing we can do, return NULL

	int res_size = BUFSIZ;
	//BUFSIZ is a macro from stdio.h. if you ar compiling using gcc, it will probably be 1024.
	//but the actual size of this
	char* res = (char*)malloc(res_size);
	//this allocates memory for the file to be read into.
	int nb_read_total = 0;
	//counter for number of bytes read. 
 
	while (!feof(in) && !ferror(in)) 
	{//while it isn't the end of the file and there isn't an error.
		if (nb_read_total + BUFSIZ > res_size) 
		{//if the buffer size is to small, increase it.
			if (res_size > 10*1024*1024) break;
			res_size = res_size * 2;
			res = (char*)realloc(res, res_size);
		}//p_res is the pointer to the end of the array. where the next character should
		//be read in to.
		char* p_res = res + nb_read_total;
		//this function starts at pointer p_res, reads 1 byte at a time, for BUFSIZ bytes
		//or until the end of file maker is reached, from the file in.
		nb_read_total += fread(p_res, 1, BUFSIZ, in);
	}

	fclose(in);
	//close the file

	res = (char*)realloc(res, nb_read_total + 1);
	//resize the buffer to be exactly the length of the shader source code. +1 for the end of
	//string marker.
	res[nb_read_total] = '\0';
	//the end of string marker.
	return res;
	//return the file.
}

//Display compilation errors from the OpenGL shader compiler
void print_log(GLuint object)
{
	GLint log_length = 0;//this will be used to get the log length.
	if (glIsShader(object))//is the log we are going to be printing from a shadder?
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);//if it is we use this call
	else if (glIsProgram(object))//if not shader. is it a program maybe?
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);//then use this call
	else 
	{//if not either of those, we don't know what to do with it. probably that is the problem.
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}

	char* log = (char*)malloc(log_length);
	//now that we know the length of the log. allocate memory for it to be dumped into.

	if (glIsShader(object))//get the log from a shader
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))//get the log from a program
		glGetProgramInfoLog(object, log_length, NULL, log);

	fprintf(stderr, "%s", log);//print out the log to stderr.
	free(log);//free that memory.
}


//Compile the shader from file 'filename', with error handling
GLint create_shader(const char* filename, GLenum type)
{
	//open the file and read in the contents. this actually returns a pointer to a char, but
	//it just gets cast to a const GLchar* automatically.
	const GLchar* source = file_read(filename);

	if (source == NULL) 
	{//if the source is empty, say there was an error.
		fprintf(stderr, "Error opening %s: ", filename); perror("");
		return 0;
	}
	//create the new shader of the respective type.
	GLuint res = glCreateShader(type);
	//pluggin the source.
	glShaderSource(res, 1, &source, NULL);
	//the source code can now be free'd from local memory
	free((void*)source);

	//compile the shader
	glCompileShader(res);
	//init the compile ok variable.
	GLint compile_ok = GL_FALSE;
	//find the compile status of the shader.
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) 
	{//if something went wrong, print the log to stderr
		fprintf(stderr, "%s:", filename);
		print_log(res);
		glDeleteShader(res);
		return 0;
	}
	//the shader is now done and the reference to it is ready to be returned. 
	return res;
}

void free_resources()
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_triangle);
}

int init_resources()
{
	GLfloat triangle_attributes[] ={
		 0.0,  0.8,   1.0, 1.0, 0.0,
		-0.8, -0.8,   0.0, 0.0, 1.0,
		 0.8, -0.8,   1.0, 0.0, 0.0,
		};

	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	
	GLint link_ok = GL_FALSE;

	GLuint vs, fs;
	if ((vs = create_shader("triangle6.v.glsl", GL_VERTEX_SHADER))   == 0) return 0;
	if ((fs = create_shader("triangle8.f.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		fprintf(stderr, "glLinkProgram:");
		return 0;
	}

	const char* attribute_name = "coord2d";	
	attribute_coord2d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord2d == -1) 
	{
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}

	attribute_name = "v_color";
	attribute_v_color = glGetAttribLocation(program, attribute_name);
	if (attribute_v_color == -1)
	{
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}
	const char* uniform_name;
	uniform_name = "fade";
	uniform_fade = glGetUniformLocation(program, uniform_name);
	if (uniform_fade == -1)
	{
		fprintf(stderr, "could not bind uniform %s\n",uniform_name);
		return 0;
	}

	return 1;
}	

void display() 
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);

	glEnableVertexAttribArray(attribute_coord2d);
	glEnableVertexAttribArray(attribute_v_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glVertexAttribPointer(
		attribute_coord2d,   // the attribute that we are discribing.
		2,                   // number of elements per vertex, here (x,y)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		5 * sizeof(GLfloat), // no extra data between each position
		0                    // offset of first element,
	);
	glVertexAttribPointer(
		attribute_v_color,   // attribute
		3,                   // number of elements per vertex, here (r,g,b)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		5 * sizeof(GLfloat), // no extra data between each position
		(GLvoid*) (2*sizeof(GLfloat))  // offset of first element
	);
	
//	glUniform1f(uniform_fade, 0.1);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord2d);
	glDisableVertexAttribArray(attribute_v_color);
	glutSwapBuffers();
}

void idle()//this is basically your main loop. so it makes most sense to put it right before
{//your main method so that all the other functions are accesable.
	float cur_fade = sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0 * (2*3.14) / 5 ) / 2 + 0.5;
	//this will find the current value of fade we want in a nice smooth sin wave. sin here will
	//return to us a value from -1 to 1. so we divide it by 2 and get -0.5 to 0.5, then we add
	//0.5 and we get a value from 0 to 1. 
	glUseProgram(program);
	glUniform1f(uniform_fade, cur_fade);
	//set the uniform to be the current fade and so our triangle uniformly fades in and out.
	glutPostRedisplay();
	//ask for glut to redisplay. 
}

int main(int argc, char *argv[])
{
        glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
        glutInitWindowSize(800,600);
        glutCreateWindow("My First Triangle");
	glutIdleFunc(idle);
	//this is where you declare what function you want to be called when glut is idle. it takes
	//a pointer to a function. in this case "idle"

	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 1;
	}

	if (init_resources())
	{
	        glutDisplayFunc(display);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	        glutMainLoop();
	}

	free_resources();
        return 0;
}
/*.S.D.G*/
