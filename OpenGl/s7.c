/* this iteration will be on interweaving data. interweaving data is where you take more than one
 * kind of data, in this case vertex and color data, and pack it together in the same array.
 * this is a little more complicated, but is incredibly useful. and actually happens all the time
 * your computer's main memory. take the following struct as an example:
 * struct data{
 * 	int someVal;
 * 	float someOtherVal;
 * 	char aLetter;
 * 	int anotherVal;
 * }
 * now, if you have an array of this data type, in your memory it would look like this:
 * memory position:	1000	1004	1008	1012	1016	1020	1024	1028
 * item in position:	int	float	char	int	int	float	char	int
 * that is two of the structs one right after the other. if you were to create an array of ints,
 * floats, chars, and another array of ints. you could achieve a very similar effect, but there
 * would be the included hastle of trying to keep track of 4 pointers. so by making structs you 
 * can interweave data and save your self some hastle. same idea here. you can't make structs in
 * openGL, but you can manually interweave, which you can do in C, but it isn't straight forward.
 * so just think of interweaving as a method of making structs in openGl and you get why it is 
 * useful. */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>


GLuint program;
GLint attribute_coord2d, attribute_v_color;
GLuint vbo_triangle, vbo_triangle_colors;


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
/*these will be replaced with interweaving.
	GLfloat triangle_colors[] = {
		1.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 0.0, 0.0,
		};
	glGenBuffers(1, &vbo_triangle_colors);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_colors), triangle_colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLfloat triangle_vertices[] = {
		 0.0,  0.8,
		-0.8, -0.8,
		 0.8, -0.8,
		};
	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
*/
	GLfloat triangle_attributes[] ={
		 0.0,  0.8,   1.0, 1.0, 0.0,
		-0.8, -0.8,   0.0, 0.0, 1.0,
		 0.8, -0.8,   1.0, 0.0, 0.0,
		};//this is what the data looks like interweaved. as you can see, it is all the
		//data, just all in one array now. the rest of the process also is un changed. the
		//differences happen down in the display method.

	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	
	GLint link_ok = GL_FALSE;

	GLuint vs, fs;
	if ((vs = create_shader("triangle6.v.glsl", GL_VERTEX_SHADER))   == 0) return 0;
	if ((fs = create_shader("triangle6.f.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

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

	return 1;
}	

void display() 
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
/*un-interweaved:
	glEnableVertexAttribArray(attribute_coord2d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glVertexAttribPointer(
		attribute_coord2d, // the attribute that we are discribing.
		2,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0		   // offset of first element, >>>>> this line changed <<<<<
	);

	glEnableVertexAttribArray(attribute_v_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_colors);
	glVertexAttribPointer(
		attribute_v_color, // attribute
		3,                 // number of elements per vertex, here (r,g,b)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);
*/

	//now for how it is done with interweaving:
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
	//that is how you interweave. the majority of this isn't changed, it is the last two lines
	//that have changed for both attribPointers. 5th line tells it how many bytes there are
	//the start of one set of data to the start of the next. this includes the size of the data
	//for this attribute. when you had no interweaving you could get away with telling it 0 and
	//it would be just fine because it knew it could pick up right where it leaves off. but 
	//when there is interweaving it want to know exactly how many bytes i needs to off set when
	//it reads. it would require more effort for the graphics card to figure out how many bytes
	//are this set of data and then add on to them the extra. seems kind of silly I know. but 
	//just remember, this is basically the total number of bytes of the data structure over all
	//the last argument tells it how far into this struct the start of the next data is. in the
	//case of the coordinates, they are right away at the begining, so their offset is 0. the 
	//color data however is after the two coordinates. so we get the size of two GLfloasts and
	//then cast that value as a GLvoid pointer. it will do pointer arithmatic in the GPU and
	//get everything aligned based of of this. that's it. you are now interweavin'!
	

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord2d);
	glDisableVertexAttribArray(attribute_v_color);
	glutSwapBuffers();
}

int main(int argc, char *argv[])
{
        glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
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
	}

	free_resources();
        return 0;
}
/*.S.D.G*/
