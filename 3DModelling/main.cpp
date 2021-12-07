// Code adapted from www.learnopengl.com, www.glfw.org


#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<ctime>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/constants.hpp>

#include "shader.h"
#include <corecrt_math_defines.h>

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement();
void takeInput();

// Window dimensions
const GLuint WIDTH = 640, HEIGHT = 640;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 20.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat yaw = -90.0f;	
GLfloat pitch = -30.0f;
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// SpeedVariables
double increments = 0;
double speedControl = 0.08;
double resolutionIncrement = .010;
float cameraVelocity = 10.0f;

//ligthing 
float darkness = 0.6f;

//Grid variables
int maxLength = 80;

//Other Variables
bool firstMouse = true;
bool invertedCameraControls_X = false;
bool invertedCameraControls_Y = false;
bool invertedMouseControls_X = false;
bool invertedMouseControls_Y = false;
GLfloat cameraSensitivity = 0.1f;

struct Planet
{
	double radius = 1;
	double xpos, ypos, zpos;
	int id = 0;
	float red;
	float green;
	float blue;
};

// Planet Variables
int currentPlanet = 0;
const int ammountPlanet = 10;
double planetResolution = 2;
Planet planets[ammountPlanet];
int maxResolution = 100;

// I don't recomend changing the minimum Resolution to anything lower, 2 is the minimum for it to draw a Square/Triangles
// Anything below 2 will not render, although feel free to increment it as longer as it less than the maximum.
int minResolution = 2;

//--------------------------------------------------------------------------------------------------//


void drawGrid() {
	int i;

	for (i= 0; i < maxLength*2; i++)
	{
		glPushMatrix();
		glBegin(GL_LINES);
		if (i< maxLength) {
			glVertex3f(-(maxLength/2), -0.1, i-(maxLength/2));
			glVertex3f((maxLength / 2) - 1, -0.1, i - (maxLength / 2));
		}
		else
		{
			glVertex3f(i-(maxLength*1.5), -0.1, -(maxLength / 2));
			glVertex3f(i-(maxLength*1.5), -0.1, (maxLength/2)-1);
		}
		glEnd();
		glPopMatrix();
	}
}

void drawSphere(double r, double xpos, double ypos, double zpos) {

	int i, j;
	glPushMatrix();
	glLoadIdentity();
	for (i = 0; i <= planetResolution; i++) {
		double lat0 = M_PI * (-0.5 + (double)(i - 1) / planetResolution);
		double z0 = sin(lat0);
		double zr0 = cos(lat0);

		double lat1 = M_PI * (-0.5 + (double)i / planetResolution);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);

		/*
		* Note for developer using this
		* This program uses lines because its easier to visualize the Spheres being drawn
		* However you are free to experiment with the others types of shapes inside the glBegin(*change here*)
		* 
		* GL_LINES
		* GL_LINE_STRIP
		* GL_LINE_LOOP
		* GL_TRIANGLES
		* GL_TRIANGLE_STRIP
		* GL_TRIANGLE_FAN
		* GL_QUADS
		* GL_QUAD_STRIP
		* GL_POLYGON
		*/
		glBegin(GL_LINE_LOOP);


		for (j = 0; j <= planetResolution; j++) {
			double lng = 2 * M_PI * (double)(j - 1) / planetResolution;
			double x = cos(lng);
			double y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f((r * x * zr0) + xpos, r * y * zr0+ ypos, (r * z0)+ zpos);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f((r * x * zr1) + xpos, r * y * zr1+ ypos, (r * z1)+ zpos);
		}
		glEnd();
	}
	glPopMatrix();
}

/*
* 
* 
* 
* 
* 
* 
* 
* 
*/
void setPlanetsCoordinates() {

	for (signed i = 1; i < ammountPlanet+1; i++)
	{
		planets[i-1].red = (float)rand() / RAND_MAX;
		planets[i-1].green = (float)rand() / RAND_MAX;
		planets[i-1].blue = (float)rand() / RAND_MAX;

		 srand((unsigned int)time(NULL));
		 double r = (double)(rand() /RAND_MAX);
		 planets[i-1].radius += (r);
		 r = ((double)rand() / (RAND_MAX));
		 planets[i-1].xpos = (i - 1) + r+i* planets[i - 1].radius;
		 r = ((double)rand() / (RAND_MAX));
		 planets[i-1].zpos = (i - 1) + r + i* planets[i - 1].radius;


		 //TODO randomize spawn
		 if (i % 2 == 0) {
			 srand((unsigned int)time(NULL));
			 r = (-1 +((double)rand() / (RAND_MAX)));
			 planets[i - 1].xpos = planets[i - 1].xpos * sin(r);
			 planets[i - 1].zpos = planets[i - 1].zpos * cos(r);
		}
	}
}

/*
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
*
*/
void drawPlanets(GLuint shader) {

	glUseProgram(shader);
	GLint objectColorLoc = glGetUniformLocation(shader, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(shader, "lightColor");
	GLint lightPosLoc = glGetUniformLocation(shader, "lightPos");

	glm::vec3 lightPos(0.0f, .0f, .0f);

	for (signed int i = 0; i < ammountPlanet; i++)
	{
		glUniform3f(objectColorLoc, planets[i].red, planets[i].green, planets[i].blue);

		glUniform3f(lightColorLoc,
			1.0f-(darkness - (darkness / (i + 1))),
			1.0f-(darkness - (darkness / (i + 1))),
			1.0f-(darkness -(darkness /(i+1))));
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	

		drawSphere(planets[i].radius, planets[i].xpos, planets[i].ypos,planets[i].zpos);
		planets[i].id = i;
	}
}

/*
* 
* 
* 
* 
* 
* 
* 
*/
void changeView() {

	cameraPos = glm::vec3(planets[currentPlanet].xpos, planets[currentPlanet].ypos +10, planets[currentPlanet].zpos);

	//TODO respotiion camera
	GLfloat yaw = -90.0f;
	GLfloat pitch = -90.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	increments = increments + speedControl;
	if (increments>1) {
		currentPlanet++;
		increments = 0;
	}
	if (currentPlanet > ammountPlanet) {
		currentPlanet = 0;
	}
}

/*
* 
* 
*/
void incrementResolution() {
	planetResolution = planetResolution + resolutionIncrement;
	
}
void decreaseResolution() {
	planetResolution = planetResolution - resolutionIncrement;
}

int main(void)
{
	//++++create a glfw window+++++++++++++++++++++++++++++++++++++++
	GLFWwindow* window;

	//Initialize the library
	if (!glfwInit()) 
		return -1;

	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	//Make the window's context current
	glfwMakeContextCurrent(window);
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	glfwSetCursorPosCallback(window, mouse_callback);

	//Enables some properties
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	

	//++++Initialize GLEW to setup the OpenGL Function pointers+++++++
	glewExperimental = GL_TRUE;
	glewInit();

	//++++Define the viewport dimensions++++++++++++++++++++++++++++
	glViewport(0, 0, HEIGHT, HEIGHT);

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Setup all planets properties to be ready and be drawn, this has to be before the drawing loop
	setPlanetsCoordinates();

	//++++++++++Build and compile shader program+++++++++++++++++++++
	GLuint shaderProgram = initShader("vert.glsl","frag.glsl");

	glm::vec3 lightPos(0.0f, 0.0f, 1.0f);

	//++++++++++++++++++++++++++++++++++++++++++++++
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = (GLfloat) glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		/* Render here */
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use cooresponding shader when setting uniforms/drawing objects

		glUseProgram(shaderProgram);
		GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

		glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

		glLoadIdentity();
		drawGrid();
		drawPlanets(shaderProgram);
		do_movement();
		takeInput();

		// use shader
		glUseProgram(shaderProgram);

		// Create transformations
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		//model = glm::rotate(model, (GLfloat)glfwGetTime() * 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		// Get their uniform location
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
		// Pass them to the shaders
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

 //Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	float inverted_X = 1;
	float inverted_Y = 1;
	if (invertedCameraControls_X) {
		inverted_X = -1;
	}
	if (invertedCameraControls_Y) {
		inverted_Y = -1;
	}

	// Camera controls
	GLfloat cameraSpeed = cameraVelocity * deltaTime;
	if (keys[GLFW_KEY_W])
		cameraPos += cameraSpeed * cameraFront* inverted_Y;
	if (keys[GLFW_KEY_S])
		cameraPos -= cameraSpeed * cameraFront* inverted_Y;
	if (keys[GLFW_KEY_A])
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed* inverted_X;
	if (keys[GLFW_KEY_D])
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed* inverted_X;
}

void takeInput() {
	if (keys[GLFW_KEY_SPACE]) {
		changeView();
	}
	if (keys[GLFW_KEY_C]) {
		if (planetResolution <= maxResolution) {
			incrementResolution();
		}
	}
	if (keys[GLFW_KEY_V]) {
		if (planetResolution >= minResolution) {
			decreaseResolution();
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float inverted_X = 1;
	float inverted_Y = 1;
	if (invertedMouseControls_X) {
		inverted_X = -1;
	}
	if (invertedMouseControls_Y) {
		inverted_Y = -1;
	}

	if (firstMouse)
	{
		lastX = (GLfloat) xpos;
		lastY = (GLfloat) ypos;
		firstMouse = false;
	}

	GLfloat xoffset = (GLfloat) xpos - lastX;
	GLfloat yoffset = lastY - (GLfloat) ypos; 
	lastX = (GLfloat) xpos;
	lastY = (GLfloat) ypos;

	GLfloat sensitivity = cameraSensitivity;
	xoffset *= sensitivity* inverted_X;
	yoffset *= sensitivity* inverted_Y;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}