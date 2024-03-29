// Code adapted from www.learnopengl.com, www.glfw.org


#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


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

// Camera Position and similar Properties
glm::vec3 cameraPos = glm::vec3(0.0f, 40.0f, 20.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right
// (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat yaw = -90.0f;	
GLfloat pitch = -90.0f;
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

/*
* The next Upcoming variables are all up to change by the user.
* I recommend experimenting with every kind of permutation until you get a result you enjoyed.
* I have set some presets but you can also make your own if you save the variables inside the functions.
* The Program will spawn spheres inside a Grid and both can be configured.
* 
* Pressing 'C' will increment the resolution and vertices utilized in the Spheres.
* Pressing 'V' will do the Opposite so you can go back and forth through the animation.
* Pressin 'Space' will center the camera above the center sphere
* 
* Pressing 'Q' will increment the speed of the camera rotation
* Pressing 'E' will decrement the speed of the camera rotation
* 
* It also supports preset you can set and save
* Pressing 'N' will go to the next preset in the array if you are already
* on the last preset it will just generate a new one of the same preset
* Pressing 'M' will go to the previous preset in the array if you are already
* on the first preset it will just generate a new one of the same preset
*/

/* Spheres Structure
* 
* This is the struct that is used to build all the Spheres in the program feel free to change if you need anything else
* It holds values for the standard radius of 1.
* The position which is changed in setPlanetsProperties()
* The id is incremented with the new creation of each sphere
* The RGB colour is also stored and currently set randomly in the setPlanetsProperties()
* 
*/ 
struct Planet
{
	double radius = 1;
	double xpos, ypos, zpos;
	int id = 0;
	float red;
	float green;
	float blue;
};

/*SpeedVariables
* 
* @increments is a global counter so you shouldn't change it unless you really need to.
* @speedControl is to define how fast we want to iterate trough each sphere when holding 'Space'
* @resolutionIncrementSpeed is to define how much resolution(longtidude and latitudes) we want to increase and decrease
* of the spheres when holding 'C' and 'V' accordingly
* @increaseSpeed This is an optional speed variable to control how fast you add the resolution.
* @decreaseSpeed This is also an optional speed variable to decrease the resolution.
* @cameraVelocity is to define how fast the camera moves troughout the 3D space
* @cameraSensitivity defines how fast we want the camera to react to the mouse control
* 
*/
double increments = 0;
double speedControl = 0.008;
double resolutionIncrementSpeed = .050;
float increaseSpeed = 1.0f;
float decreaseSpeed = 3.0f;
float cameraVelocity = 10.0f;
GLfloat cameraSensitivity = 0.1f;
GLfloat cameraRotationSpeed = 1;

/*Lighting
* 
* @darkness defines how dark we want the Spheres to become as they get further away from the origin
* 
*/
float darkness = 0.7f;

/*Grid variables
* 
* @maxLength is how many lines we want to be drawn and each line is separated by one unit.
* @spaceWidth is how many units you can wish to separate each line, this mean we can get bigger grids with less lines
* This is done in the drawGrid() function and you can change it if want bigger grid or simply spaced out Grid
* 
*/
int maxLength = 80;
float spaceWidth = 1.0f;

/*Planet Variables
* 
* @ammountPlanet is how many Spheres we want to generate on the grid
* @planets is the array that holds the value of all the vertices in each Sphere.
* @planetResolution this is the starting resolution of a Sphere and it keeps tracks and changes as more is increased/decreased
* @currentPlanet is a global counter to keep track of which Sphere we are currently seeing when 'Space' is pressed 
* @planets is the array that holds the value of all the vertices in each Sphere.
* @maxResolution is the ceiling capped number when incrementing using 'C'
* @minResolution is flooring capped number when decreasing using 'V'
* I don't recomend changing the minimum Resolution to anything lower, 2 is the minimum for it to draw a Square/Triangles
* and Anything below 2 will not render.
* 
* All of these variables are utilized in drawSphere(),incrementResolution(),decreaseResolution()
* 
*/
const int ammountPlanet = 100;
Planet planets[ammountPlanet];
double planetResolution = 2;
int currentPlanet = 0;
int maxResolution = 100;
int minResolution = 2;


/* Spheres spawn generation
* 
* @SpiralSize is how clustered the spheres are, smaller numbers will be more clustered in the center 
* and bigger numbers gives more spread of the Spheres.
* Use setPlanetsProperties() if you need to change how the Spheres position are generated
* 
* This program uses GL_LINE_LOOP because its easier to visualize the Spheres being drawn
* However it is possible to change the type of shape you want to use if you so desire.
* @shapeChoice the integer that selects which shape to fill the Spheres we can use to fill the Sphere.
* @shapes[] is an array that holds the Enums for these types of shapes we can use you can even add more.
* 
*/
float spiralSize = .2f;
int shapeChoice = 7;
GLenum shapes[] = { GL_LINES, // choice 0
					GL_LINE_STRIP, // choice 1
					GL_LINE_LOOP, // choice 2
					GL_TRIANGLES, // choice 3
					GL_TRIANGLE_STRIP, // choice 4
					GL_TRIANGLE_FAN, // choice 5
					GL_QUADS, // choice 6
					GL_QUAD_STRIP, // choice 7
					GL_POLYGON // choice 8
};

/*Other Variables
* 
* @firstMouse enables to take the value of the first location of the mouse.
* Do not change this variable unless you really need to.
* @invertedCameraControls_X if true inverts camera controls for the keys 'A' and 'D'
* @invertedCameraControls_Y if true inverts camera controls for the keys 'W' and 'S'
* @invertedMouseControls_X if true inverts mouse controls when moving horizontally
* @invertedMouseControls_Y if true inverts mouse controls when moving vertically
* 
*/
bool firstMouse = true;
bool invertedCameraControls_X = false;
bool invertedCameraControls_Y = false;
bool invertedMouseControls_X = false;
bool invertedMouseControls_Y = false;
bool rotateCamera = true;

//--------------------------------------------------------------------------------------------------//

/*Using Presets
* 
* If you want to use Preset the first thing to do is to follow the next Steps
* @usingPresets turn this variable true if you want to turn on Presets.
* @totalPresets are the amount of Presets you want to store in the program. This is hardcoded
* @currentPreset will keep track of which Preset you want to show and helps if you want to scroll over more.
* The default is 0 and will not show a Preset unless you change the switch statement
* 
* If you want to hardcode a new Preset update the totalPresets and place a new switch case inside the usePreset() Function
* 
*/
bool usingPresets = false;
const int totalPresets = 2;
int currentPreset = 1;

/*
* 
* 
*/
void usePreset(int preset) {
	
	switch (preset){
	case 1: {
		increments = 0;
		speedControl = 0.08;
		resolutionIncrementSpeed = .050;
		increaseSpeed = 1.0f;
		decreaseSpeed = 3.0f;
		cameraVelocity = 10.0f;
		cameraSensitivity = 0.1f;

		darkness = 0.5f;

		maxLength = 80;
		spaceWidth = 1.0f;

		const int ammountPlanets = 10;
		Planet planets[ammountPlanets];
		planetResolution = 2;
		currentPlanet = 0;
		maxResolution = 100;
		minResolution = 2;

		spiralSize = .2f;
		shapeChoice = 4;
		break;
	}
	case 2: {
		increments = 0;
		speedControl = 0.08;
		resolutionIncrementSpeed = .050;
		increaseSpeed = 1.0f;
		decreaseSpeed = 3.0f;
		cameraVelocity = 10.0f;
		cameraSensitivity = 0.1f;

		darkness = 0.8f;

		maxLength = 100;
		spaceWidth = 1.0f;

		const int ammountPlanets = 100;
		Planet planets[ammountPlanets];
		planetResolution = 2;
		currentPlanet = 0;
		maxResolution = 100;
		minResolution = 2;

		spiralSize = .1f;
		shapeChoice = 4;
		break;
	}
	}
}


//--------------------------------------------------------------------------------------------------//

/*
* This method uses a loop to draw a grid using lines given the parameters
* This grid is squared grid and each line is equally spaced out
* 
*/
void drawGrid() {

	int i;

	for (i= 0; i < maxLength*2; i++)
	{
		glBegin(GL_LINES);
		if (i< maxLength) {
			glVertex3f((-(maxLength/2)) * spaceWidth, -0.1, (i-(maxLength/2))*spaceWidth);
			glVertex3f(((maxLength / 2) - 1) * spaceWidth, -0.1, (i - (maxLength / 2))* spaceWidth);
		}
		else
		{
			glVertex3f((i-(maxLength*1.5)) * spaceWidth, -0.1, -(maxLength / 2)* spaceWidth);
			glVertex3f((i-(maxLength*1.5)) * spaceWidth, -0.1, ((maxLength/2)-1)* spaceWidth);
		}
		glEnd();
	}
}

/*
* This method draws a Sphere given a position and radius
* After the Sphere is place in the 3D enviroment it will start generating itself by how many longitudes and latitudes given
* 
*/
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

		glBegin(shapes[shapeChoice]);


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
* This method iterates through each Sphere and sets the colour and position
* This could be used to define what shape we are going to draw, therefore this should always be called at least once before you start drawing them
* 
*/
void setPlanetsProperties() {

	for (signed i = 1; i < ammountPlanet+1; i++)
	{

		srand((unsigned int)time(NULL));
		planets[i-1].red = (float)rand() / RAND_MAX;
		planets[i-1].green = (float)rand() / RAND_MAX;
		planets[i-1].blue = (float)rand() / RAND_MAX;

		planets[i-1].xpos = cos(i - 1) *(i-1)* spiralSize;
		planets[i-1].zpos = sin(i - 1) *(i-1)* spiralSize;
	}
}

/*
* If the Spheres properties are defined, this method will iterate trough all of them and call the method to start drawing them
* This includes colour and you can change to your own liking.
* There is currently a implementation for the colour to subdue as it gets to the last Sphere in the array
* 
* 
*/
void drawPlanets(GLuint shader) {

	glUseProgram(shader);
	GLint objectColorLoc = glGetUniformLocation(shader, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(shader, "lightColor");
	GLint lightPosLoc = glGetUniformLocation(shader, "lightPos");

	for (signed int i = 0; i < ammountPlanet; i++)
	{
		glm::vec3 lightPos(planets[i].xpos, planets[i].ypos, planets[i].zpos);

		glUniform3f(objectColorLoc, planets[i].red, planets[i].green, planets[i].blue);

		glUniform3f(lightColorLoc,
			1.0f-(darkness - (darkness / (ammountPlanet/(ammountPlanet-i)))),
			1.0f-(darkness - (darkness / (ammountPlanet / (ammountPlanet - i)))),
			1.0f-(darkness -(darkness / (ammountPlanet / (ammountPlanet - i)))));
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	

		drawSphere(planets[i].radius, planets[i].xpos, planets[i].ypos,planets[i].zpos);
		planets[i].id = i;
	}
}

/*
* This method will update the camera and jump to a Sphere so it is possible to visualize each one individually 
* 
*/
void changeView() {

	GLfloat yaw = -90.0f;
	GLfloat pitch = -90.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraPos = glm::vec3(planets[0].xpos, planets[0].ypos + 30, planets[0].zpos);

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

 	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

/*
* Methods to increment and decrement variables utilized in the program
* 
*/
void incrementResolution() {
	planetResolution = planetResolution + resolutionIncrementSpeed*increaseSpeed;
	
}
void decreaseResolution() {
	planetResolution = planetResolution - resolutionIncrementSpeed*decreaseSpeed;
}

void increasePreset() {
	currentPreset++;
}

void decreasePreset() {
	currentPreset--;
}
void increaseRotationSpeed() {
	cameraRotationSpeed = cameraRotationSpeed+ speedControl;
}
void decreaseRotationSpeed() {
	cameraRotationSpeed = cameraRotationSpeed- speedControl;
}

/*
* This method should be called everytime you want to refresh the variables utilized for the Sphere generation presets
* It uses the currentPreset variable to update each Spheres properties
*
*/
void refreshPreset() {
	usePreset(currentPreset);
	setPlanetsProperties();
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
	if (usingPresets) {
		usePreset(currentPreset);
	}
	setPlanetsProperties();

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

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		if (rotateCamera) {
			model = glm::rotate(model, (GLfloat)glfwGetTime() * -cameraRotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

/*
* 
* Is called whenever a key is pressed/released via GLFW
* 
*/
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


/*
* This method lets the user control the camera by moving with the WASD keys
* It also has some logic to implement reverse controls
* 
*/
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

/*
* Searches for a certain input and runs a method call everytime it coincides with the preferred button press
* 
*/
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
	if (keys[GLFW_KEY_N]) {
		if (currentPreset <= totalPresets && usingPresets) {
			increasePreset();
			refreshPreset();
		}
	}
	if (keys[GLFW_KEY_M]) {
		if (currentPreset >= 1 && usingPresets) {
			decreasePreset();
			refreshPreset();
		}
	}
	if (keys[GLFW_KEY_Q]) {
		if (cameraRotationSpeed < 50) {
			increaseRotationSpeed();
		}
	}
	if (keys[GLFW_KEY_E]) {
		if (cameraRotationSpeed > 1) {
			decreaseRotationSpeed();
		}
	}
}

/*
* Takes input from the movement of the mouuse and updates the camera positions as it moves
* 
*/
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