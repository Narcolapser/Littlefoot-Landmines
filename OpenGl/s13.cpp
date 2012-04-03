/* This iteration is we finally get to add 3D! first up you can see right away There are some new
 * globals. most of this I'll explain as we go, so just go down and check out the new stuff.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLuint program;
GLint attribute_coord3d, attribute_v_color;
GLuint vbo_triangle, vbo_triangle_colors;
//GLint uniform_fade;
GLint uniform_m_transform;
GLuint vbo_cube_vertices, vbo_cube_colors;//3 new references. 
GLuint ibo_cube_elements;

struct attributes 
{
	GLfloat coord3d[3];
	GLfloat v_color[3];
};
struct attributes triangle_attributes[] = 
{
	{{ 0.0,  0.8, 0.0}, {1.0, 1.0, 0.0}},
	{{-0.8, -0.8, 0.0}, {0.0, 0.0, 1.0}},
	{{ 0.8, -0.8, 0.0}, {1.0, 0.0, 0.0}}
};

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

	GLfloat cube_vertices[] = {
		// front
		-1.0, -1.0,  1.0,
		 1.0, -1.0,  1.0,
		 1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// back
		-1.0, -1.0, -1.0,
		 1.0, -1.0, -1.0,
		 1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
	};
	glGenBuffers(1, &vbo_cube_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	
	GLfloat cube_colors[] = {
		// front colors
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 1.0, 1.0,
		// back colors
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 1.0, 1.0,
		};
	glGenBuffers(1, &vbo_cube_colors);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);


	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 0, 3,
		3, 7, 4,
		// left
		4, 5, 1,
		1, 0, 4,
		// right
		3, 2, 6,
		6, 7, 3,
		};
	//Geometry in graphics is best done with triangles. that is what the hard ware has been 
	//designed to do really quickly. So here we make up each face of the cube with two 
	//triangles. rather than specifying each vertex of each triangle. we specify the vertices
	//once, and then simply say what vertices are used by each triangle. The order that the
	//vertices are listed in does matter. specifically they all need to go the same way. here
	//they are going counter-clockwise. the reason being has to do with texture mapping and
	//lighting. so just nod for now. 
	glGenBuffers(1, &ibo_cube_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	//notice it is a GL_ELEMENT_ARRAY_BUFFER not a GL_ARRAY_BUFFER. I'll explain later.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

//	glGenBuffers(1, &vbo_triangle);
//	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	
	GLint link_ok = GL_FALSE;

	GLuint vs, fs;
	if ((vs = create_shader("triangle12.v.glsl", GL_VERTEX_SHADER))   == 0) return 0;
	if ((fs = create_shader("triangle13.f.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

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

	const char* attribute_name = "coord3d";	
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord3d == -1) 
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
//	uniform_name = "fade";
//	uniform_fade = glGetUniformLocation(program, uniform_name);
//	if (uniform_fade == -1)
//	{
//		fprintf(stderr, "could not bind uniform %s\n",uniform_name);
//		return 0;
//	}
	
	uniform_name = "m_transform";
	uniform_m_transform = glGetUniformLocation(program, uniform_name);
	if (uniform_m_transform == -1)
	{//this should all be old hat to you by this point. 
		fprintf(stderr, "Could not bind uniform %s\n",uniform_name);
		return 0;
	}

	return 1;
}	

void display() 
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

	glEnableVertexAttribArray(attribute_coord3d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glVertexAttribPointer(
		attribute_coord3d,   // the attribute that we are discribing.
		3,                   // number of elements per vertex, here (x,y)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		0,                   // no extra data between each position
		0                    // offset of first element,
	);

	glEnableVertexAttribArray(attribute_v_color);
	glBindBuffer(GL_ARRAY_BUFFER,  vbo_cube_colors);
	glVertexAttribPointer(
		attribute_v_color,   // attribute
		3,                   // number of elements per vertex, here (r,g,b)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		0,                   // no extra data between each position
		0                    // offset of first element
	);//since we aren't iterweaving anymore, the offset is back to 0.


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	int size;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	//here is why it was an element_array_buffer and not just an array_buffer. we are using a
	//slightly different method call. glDrawElements takes a set of indices that refer to a
	//vertex array. 

//	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_v_color);
	glutSwapBuffers();
}
//this works, but the problem is that it just looks like a square. so it feels like nothing has 
//really been accomplished. This is were the other very important part of a 3D modeling system 
//comes into play. The camera. next iteration will cover that. 

void idle()
{
	float move = sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0 * (2*3.14) / 5 ); //the move will happen over 5 seconds.
	float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0 *45; // 45 degrees per second
	glm::vec3 axis_z(0,0,1);
	glm::mat4 m_transform = glm::translate(glm::mat4(1.0f), glm::vec3(move, 0.0, 0.0)) * glm :: rotate(glm::mat4(1.0f), angle, axis_z);
//	float cur_fade = sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0 * (2*3.14) / 5 ) / 2 + 0.5;

	glUniformMatrix4fv(uniform_m_transform,1,GL_FALSE, glm::value_ptr(m_transform));
	glUseProgram(program);
//	glUniform1f(uniform_fade, cur_fade);
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
        glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
        glutInitWindowSize(800,600);
        glutCreateWindow("My First Triangle");
	glutIdleFunc(idle);
	glEnable(GL_DEPTH_TEST);

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
