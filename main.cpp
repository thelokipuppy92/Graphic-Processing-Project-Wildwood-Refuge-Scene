//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include <iostream>


int glWindowWidth = 1920;  // set on fullsize
int glWindowHeight = 1080; // set on fullsize
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;


gps::Camera myCamera(
	glm::vec3(-25.697445f, 2.187538f, 5.128894f),
	glm::vec3(0.992601f, -0.107999f, 0.055495f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.05f;

GLboolean pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D lightCube;
gps::Model3D screenQuad;

//my objects
gps::Model3D myScene;
gps::Model3D elice;

glm::vec3 elicePos = glm::vec3(0.601104f, 0.073092f, -0.611585f);
float eliceAngle;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

GLfloat angle;

bool showDepthMap=false;

GLuint textureID;

//fog
glm::vec3 fogNoFogLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
int isFogRunning = 0;

//scene tour
int activatetour = 0;
float previewAngle = 0.0f;
bool tourStart = false;
int statePreview = 0;


//point light:fire
int pointLightStart = 0;
glm::vec3 lightPos1;
GLuint lightPos1Loc;


int visualize = 0;
float movementSpeed = 2.0f;
float rotationSpeed = 500.0f;
float start = 0.0f;
float duration = 10.0f;
float lastTimeStamp = glfwGetTime();

//boolean variables
bool isFirstMouse = true; //move the scene according to the mouse movement


// camera animation
std::vector<glm::vec3> cameraPathPoints = {
	{-5.37379f, 0.655384f, -10.7537f},
	{-11.0193f, 0.234614f, -3.84629f},
	{-8.19794f, -0.309808f, 1.3335f},
	{-4.22466f, 0.410035f, 4.70612f},
	{-1.32635f, -0.337625f, -1.20883f}
};

int currentPathIndex = 0;
bool cameraTourActive = false;
float travelTime = 5.0f;  // Time to move between points
float interpolationFactor = 0.0f;  // Interpolation factor for smooth movement
float deltaTime;

// Function to start the tour
void startTour() {
	cameraTourActive = true;
	currentPathIndex = 0;  // Start at the first path point
}

// Function to stop the tour
void stopTour() {
	cameraTourActive = false;
}

void updateCameraAnimation(float deltaTime) {
	if (!cameraTourActive) {
		return;
	}

	// Ensure we don't go past the last point
	if (currentPathIndex >= (int)cameraPathPoints.size() - 1) {
		cameraTourActive = false;
		return;
	}

	// Increment the interpolation factor based on deltaTime for continuous movement
	interpolationFactor += deltaTime / travelTime;  // travelTime controls the speed of movement

	// Get the current and next points on the path
	glm::vec3 startPos = cameraPathPoints[currentPathIndex];
	glm::vec3 endPos = cameraPathPoints[currentPathIndex + 1];

	// Parametric line equation: P(t) = P0 * (1 - t) + P1 * t
	glm::vec3 newPos = startPos * (1.0f - interpolationFactor) + endPos * interpolationFactor;
	myCamera.setCameraPosition(newPos);

	// If interpolation is done (t = 1), move to the next path segment
	if (interpolationFactor >= 1.0f) {
		// Reset interpolation factor and move to the next point
		interpolationFactor = 0.0f;
		currentPathIndex++;

		// If we reached the last point, stop the tour
		if (currentPathIndex >= cameraPathPoints.size()) {
			cameraTourActive = false;
		}
	}
}

/*
void updateCameraAnimation(float deltaTime) {
	if (!cameraTourActive) {
		return;
	}

	// Ensure we don't go past the last point
	if (currentPathIndex >= (int)cameraPathPoints.size() - 1) {
		cameraTourActive = false;
		return;
	}

	// Increment the interpolation factor based on deltaTime for continuous movement
	interpolationFactor += deltaTime / travelTime;  // travelTime controls the speed of movement

	// Get the current and next points on the path
	glm::vec3 startPos = cameraPathPoints[currentPathIndex];
	glm::vec3 endPos = cameraPathPoints[currentPathIndex + 1];

	// Parametric line equation: P(t) = P0 * (1 - t) + P1 * t
	glm::vec3 newPos = startPos * (1.0f - interpolationFactor) + endPos * interpolationFactor;
	myCamera.setCameraPosition(newPos);

	// Calculate the camera's target position slightly beyond the next point (P(1.1))
	glm::vec3 cameraTarget = startPos * (1.0f - 1.1f) + endPos * 1.1f; // Using t = 1.1 for the target
	glm::vec3 cameraFront = glm::normalize(cameraTarget - newPos);  // Direction to look at
	myCamera.setCameraFrontDirection(cameraFront);  // Update the camera's front direction

	// If interpolation is done (t = 1), move to the next path segment
	if (interpolationFactor >= 1.0f) {
		// Reset interpolation factor and move to the next point
		interpolationFactor = 0.0f;
		currentPathIndex++;

		// If we reached the last point, stop the tour
		if (currentPathIndex >= cameraPathPoints.size()) {
			cameraTourActive = false;
		}
	}
}
*/


GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}

bool mouse = true;
double lastX = 0, lastY = 0;
float yaw = 0.0f, pitch = 0.0f; //yaw= rotation around the vertical axis, pitch = rotation around the horizontal axis

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	//TODO
	if (mouse) //preventing large jumps
	{
		lastX = xpos;
		lastY = ypos;
		mouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.2f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	//prevent for flipping the camera over
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	if (!tourStart) {
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		myCamera.setCameraFrontDirection(glm::normalize(front));
	}
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	//up
	if (pressedKeys[GLFW_KEY_R]) {
		glm::vec3 cameraPos = myCamera.getCameraPosition();
		cameraPos.y += 0.05f;
		myCamera.setCameraPosition(cameraPos);
	}

	//down
	if (pressedKeys[GLFW_KEY_F]) {
		glm::vec3 cameraPos = myCamera.getCameraPosition();
		cameraPos.y -= 0.05f;
		myCamera.setCameraPosition(cameraPos);
	}

	//Fog
	//Start fog
	if (pressedKeys[GLFW_KEY_Z]) {
		myCustomShader.useShaderProgram();
		fogNoFogLightColor = glm::vec3(0.4f, 0.4f, 0.0f);
		isFogRunning = 1;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isFogRunning"), isFogRunning);
	}

	//Stop fog
	if (pressedKeys[GLFW_KEY_X]) {
		myCustomShader.useShaderProgram();
		fogNoFogLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		isFogRunning = 0;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isFogRunning"), isFogRunning);
	}

	//fire on
	if (pressedKeys[GLFW_KEY_P]) {
		myCustomShader.useShaderProgram();
		pointLightStart = 1;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightStart"), pointLightStart);
	}

	// fire off
	if (pressedKeys[GLFW_KEY_O]) {
		myCustomShader.useShaderProgram();
		pointLightStart = 0;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightStart"), pointLightStart);
	}

	//start camera tour
	if (pressedKeys[GLFW_KEY_T]) {
		startTour();  
	}

	//stop camera tour
	if (pressedKeys[GLFW_KEY_G]) {
		stopTour();   
	}

	//Shadow moving
	if (pressedKeys[GLFW_KEY_K]) {
		lightAngle -= 1.0f;
	}

	//Shadow moving
	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_M]) {

		showDepthMap = true;

	}

	if (pressedKeys[GLFW_KEY_N]) {

		showDepthMap = false;

	}

	//solid mode
	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//wireframe mode
	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//point view
	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}


}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);


	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	myScene.LoadModel("objects/scene/scena_final.obj");
	elice.LoadModel("objects/scene/elice.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	
}


void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();

	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
	
}



void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//get the view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	//send to shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction 
	lightDir = glm::vec3(-8.3099f, 5.02252f, -0.663094f);

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	//white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));


	//point liht
    lightPos1 = glm::vec3(-3.82752f, -0.372744f, 2.7717f); // Light position in world space
    lightPos1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos1");
	glUniform3fv(lightPos1Loc, 1, glm::value_ptr(lightPos1));  // Pass transformed light position to shader


	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO

	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//the coordinates of all objects in the scene must be transformed to be relative to the light instead of the camera
glm::mat4 computeLightSpaceTrMatrix() {
	const GLfloat near_plane = 0.1f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -50.0f, 50.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	myScene.Draw(shader);
	
	//elice
	shader.useShaderProgram();
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	model = glm::translate(model, elicePos);
	model = glm::rotate(model, eliceAngle += 0.03f, glm::vec3(-1, 0, 0));
	model = glm::translate(model, -elicePos);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	elice.Draw(shader);

	
}



void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	// render depth map on screen - toggled with the M key

	//render the scene to the depth buffer

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, false);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);

	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light
		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);
	}


}



void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {
	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	

	glCheckError();

	start = glfwGetTime();

	double lastTime = glfwGetTime();  // Time of the last frame
	float deltaTime = 0.0;

	while (!glfwWindowShouldClose(glWindow)) {
		double currentTime = glfwGetTime();
		deltaTime = float(currentTime - lastTime); // Time difference between current and last frame
		lastTime = currentTime;  // Update last time

		processMovement();
		updateCameraAnimation( deltaTime);
		renderScene();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}


	cleanup();

	return 0;
}