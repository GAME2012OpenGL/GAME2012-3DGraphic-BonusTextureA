//***************************************************************************
// bonus_Jang.cpp by Jang Doosung (C) 2018 All Rights Reserved.
//
// Bonus Texture Assignment submission.
//
// Description:
//	Texture Mapping
//
//*****************************************************************************

using namespace std;
#include <iostream>
#include "stdlib.h"
#include "time.h"
#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "SoilLib/SOIL.h"

#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,1,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)

GLuint vao, ibo, points_vbo, colours_vbo, modelID;
GLuint D8_tex = 0;

float rotAngle = 0.0f;

// Horizontal and vertical ortho offsets.
float osH = 0.0f, osV = 0.0f, scrollSpd = 0.25f;

int deltaTime, currentTime, lastTime = 0;
glm::mat4 mvp, view, projection;

int WindowWidth = 800;
int WindowHeight = 600;

glm::vec3 CameraPosition = glm::vec3(0.f, 0.f, 10);
float fCameraSpeed = 0.5f;

int iNumOfCubes = 0;
float* CubesAngleArray = nullptr;

GLfloat D8_vertices[] = 
{
	//Upper front
	0.f, 2.f, 0.f,	//0
	-1.f, 0.f, 1.f,
	1.f, 0.f, 1.f,

	//Upper right
	0.f, 2.f, 0.f,	//3
	1.f, 0.f, 1.f,
	1.f, 0.f, -1.f,

	//Upper Back
	0.f, 2.f, 0.f,	//6
	1.f, 0.f, -1.f,
	-1.f, 0.f, -1.f,

	//Upper left
	0.f, 2.f, 0.f,	//9
	-1.f, 0.f, -1.f,
	-1.f, 0.f, 1.f,

	//Down front
	0.f, -2.f, 0.f, //12
	1.f, 0.f, 1.f,
	-1.f, 0.f, 1.f,

	//Down right
	0.f, -2.f, 0.f, //15
	1.f, 0.f, -1.f,
	1.f, 0.f, 1.f,

	//Down back
	0.f, -2.f, 0.f, //18
	-1.f, 0.f, -1.f,
	1.f, 0.f, -1.f,

	//Down left
	0.f, -2.f, 0.f, //21
	-1.f, 0.f, 1.f,
	-1.f, 0.f, -1.f

};

GLshort D8_indices[] =
{
	//Upper front
	0, 1, 2,

	//Upper right
	3, 4, 5,

	//Upper back
	6, 7, 8,

	//Upper left
	9, 10, 11,

	//Down front
	12, 13, 14,

	//Down right
	15, 16, 17,

	//Down back
	18, 19, 20,

	//Down left
	21, 22, 23
};

void init(void)
{
	//Specifying the name of vertex and fragment shaders.
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};
	
	//Loading and compiling shaders
	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up

	modelID = glGetUniformLocation(program, "mvp");

	// Perspective arameters : Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	// projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	// Ortho parameters: left, right, bottom, top, nearVal, farVal
	//projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f); // In world coordinates
	projection = glm::perspective(glm::radians(45.f), (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.f);
	
	// Camera matrix
	view = glm::lookAt
	(
		CameraPosition,		// Camera pos in World Space
		glm::vec3(0, 0, 0),		// and looks at the origin
		glm::vec3(0, 1, 0)		// Head is up (set to 0,-1,0 to look upside-down)
	);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(D8_indices), D8_indices, GL_STATIC_DRAW);

		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(D8_vertices), D8_vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
	

	GLint width, height;
	unsigned char* image = SOIL_load_image("d8.png", &width, &height, 0, SOIL_LOAD_RGB);
	if (image == nullptr)
	{
		printf("Error: image not found\n");
	}

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &D8_tex);
	glBindTexture(GL_TEXTURE_2D, D8_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glUniform1i(glGetUniformLocation(program, "texture0"), 0);


	GLfloat D8_TexUV[] =
	{
		//Upper Front
		0.5f, 0.25f,
		0.25f, 0.5f,
		0.75f, 0.5f,

		//Upper right
		0.5f, 0.25f,
		0.75f, 0.5f,
		1.f, 0.25f,

		//Upper back
		0.5f, 0.25f,
		1.f, 0.25f,
		0.75f, 0.f,

		//Upper left
		0.5f, 0.25f,
		0.f, 0.25f,
		0.25f, 0.5f,

		//Down front
		0.5f, 0.75f,
		0.75f, 0.5f,
		0.25f, 0.5f,

		//Down right
		0.5f, 0.75f,
		1.f, 0.75f,
		0.75f, 0.5f,

		//Down back
		0.5f, 0.75f,
		0.75f, 1.f,
		1.f, 0.75f,

		//Down left
		0.5f, 0.75f,
		0.25f, 0.5f,
		0.f, 0.75
	};

	GLuint D8_tex_vbo = 0;
	glGenBuffers(1, &D8_tex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, D8_tex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(D8_TexUV), D8_TexUV, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);


	// Enable depth test.
	glEnable(GL_DEPTH_TEST);
}

//---------------------------------------------------------------------
//
// transformModel
//

void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) 
{
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);
	mvp = projection * view * Model;
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &mvp[0][0]);
}

//---------------------------------------------------------------------
//
// display
//

void display(void)
{
	// Delta time stuff.
	currentTime = glutGet(GLUT_ELAPSED_TIME); // Gets elapsed time in milliseconds.
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	//Camera Update
	view = glm::lookAt
	(
		CameraPosition,		// Camera pos in World Space
		glm::vec3(0, 0, 0),		// and looks at the origin
		glm::vec3(0, 1, 0)		// Head is up (set to 0,-1,0 to look upside-down)
	);

	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	



	glBindTexture(GL_TEXTURE_2D, D8_tex);
	glBindVertexArray(vao);

	static float fCubeAngle1 = 0.f;
	fCubeAngle1 += ((float)45 / (float)1000 * deltaTime);
	if (fCubeAngle1 >= 360.f)
		fCubeAngle1 -= 360.f;
	transformObject(glm::vec3(1.f, 1.f, 1.f), Y_AXIS, fCubeAngle1, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, sizeof(D8_indices) / sizeof(GLshort), GL_UNSIGNED_SHORT, 0);





	glutSwapBuffers(); // Instead of double buffering.
}

void idle()
{
	//glutPostRedisplay();
}

void timer(int id)
{ 
	glutPostRedisplay();
	glutTimerFunc(33, timer, 0); 
}

void keyDown(unsigned char key, int x, int y)
{
	// Orthographic.
	switch(key)
	{
		case 'w':
			CameraPosition.z -= fCameraSpeed;
			//osV -= scrollSpd;
			break;
		case 's':
			CameraPosition.z += fCameraSpeed;
			//osV += scrollSpd;
			break;
		case 'a':
			CameraPosition.x -= fCameraSpeed;
			//osH += scrollSpd;
			break;
		case 'd':
			CameraPosition.x += fCameraSpeed;
			//osH -= scrollSpd;
			break;
		case 'r':
			CameraPosition.y += fCameraSpeed;
			break;
		case 'f':
			CameraPosition.y -= fCameraSpeed;
			break;
	}
}

void keyUp(unsigned char key, int x, int y)
{
	// Empty for now.
}

void mouseMove(int x, int y)
{
	cout << "Mouse pos: " << x << "," << y << endl;
}

void mouseDown(int btn, int state, int x, int y)
{
	cout << "Clicked: " << (btn == 0 ? "left " : "right ") << (state == 0 ? "down " : "up ") <<
		"at " << x << "," << y << endl;
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow("Jang, Doosung, 101175013");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.
	init();

	// Set all our glut functions.
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutTimerFunc(33, timer, 0);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMouseFunc(mouseDown);
	glutPassiveMotionFunc(mouseMove); // or...
	//glutMotionFunc(mouseMove); // Requires click to register.
	glutMainLoop();

	delete[] CubesAngleArray;
}
