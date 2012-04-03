/* this iteration will introduce vertex buffer objects and we'll start making the shader a little 
 * more interesting. buffer objects are sections of memory on the GPU. if you store info there,
 * like texture or geometry data, the GPU will be able to access is faster with orders of magnitude
 * difference in the latency. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>


GLuint program;
GLint attribute_coord2d;
GLuint vbo_triangle;
//another reference. this time to our our vertex buffer object

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
	glDeleteBuffers(1, &vbo_triangle);//here's where we delete our buffer object.
}

int init_resources()
{
	
	GLfloat triangle_vertices[] = {
		 0.0,  0.8,
		-0.8, -0.8,
		 0.8, -0.8,
		};
	//create the arra of info that we will send to the buffer.
	glGenBuffers(1, &vbo_triangle);
	//this creates the buffer. the 1 says you only want one buffer object. the second argument
	//should be an array if you plan on getting more than 1. i think it it will allocate you 
	//space if you ask for more than 1, but it doesn't say clearly either way. I think it would
	//be safe to assume it does though.
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	//binds a buffer to reference you have. when you generate a buffer object, you don't 
	//actually make a buffer object, you prepare a reference to it. this will allow you to
	//create and use a named buffer object. here you also specify what kind of buffer object
	//you want it to be. 
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);
	//creates and initializes a buffer object's data store. this is where you put crap into the
	//buffer. the arguments are the type of object you want to store it in, the size (in number
	//of bytes) of the data to be passed, a pointer to the actual data, and lastly now it will
	//be used. This is a symbolic constant that will be one of the following: 
        //          GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, 
        //          GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, 
        //          GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, or GL_DYNAMIC_COPY.
	//Stream, Static, Dynamic. Draw, Read, Copy. quoting again:
//The frequency of access may be one of these:
//                    STREAM
//                        The data store contents will be modified once and used at most a few times.
//                    STATIC
//                        The data store contents will be modified once and used many times.
//                    DYNAMIC
//                        The data store contents will be modified repeatedly and used many times.
//                    
//The nature of access may be one of these:
//                    DRAW
//                        The data store contents are modified by the application, and used as the source for GL drawing and
//                        image specification commands.
//                    READ
//                        The data store contents are modified by reading data from the GL, and used to return that data 
//                        when queried by the application.
//                    COPY
//                        The data store contents are modified by reading data from the GL, and used as the source for GL
//                        drawing and image specification commands.
                    
                

	GLint link_ok = GL_FALSE;

	GLuint vs, fs;
	if ((vs = create_shader("triangle.v.glsl", GL_VERTEX_SHADER))   == 0) return 0;
	if ((fs = create_shader("triangle.f.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

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

	return 1;
}	

void display() 
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	glEnableVertexAttribArray(attribute_coord2d);
	//these 4 lines unchanged

/*	This is no longer needed.
	GLfloat triangle_vertices[] = 
	{
		 0.0,  0.8,
		-0.8, -0.8,
		 0.8, -0.8,
	};*/
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	//so why do we have to bind the buffer again? well, this is like locks in multi-processing.
	//the sequence in which you do things with buffers will follow this patter:
	//create -> bind -> stuff data -> unbind;
	//then in your display method:
	//bind -> display -> unbind;
	glVertexAttribPointer(
		attribute_coord2d, // the attribute that we are discribing.
		2,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0		   // offset of first element, >>>>> this line changed <<<<<
	);

	//these unchanged
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord2d);
	glutSwapBuffers();
}

int main(int argc, char *argv[])
{
        glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);//added alpha.
        glutInitWindowSize(800,600);
        glutCreateWindow("My First Triangle");

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
		//and that is all the changes for this iteration. give it a go. the next bits of commentary
		//will be in the shader source files. so head on over to those files and take a look!
	}

	free_resources();
        return 0;
}
/*.S.D.G*/
