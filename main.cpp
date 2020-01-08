#include "glad.h"
#include "shader.h"
#include "model.h"
#include <GL/gl.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <experimental/filesystem>
using namespace glm;

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, double xpos, double ypos);

void createFloor(Shader shader);
void renderScene(Shader shader,Model IronMan);

void updateLight();

//Windows Dimension
const GLint WIDTH = 1366, HEIGHT = 768;
const GLuint SHADOW_WIDTH = 5018, SHADOW_HEIGHT = 5152;

GLuint VAO, VBO, EBO;
GLuint depthMapFBO, depthMap;

//camera values
glm::vec3 cPosInit = glm::vec3(0.0f, 2.0f, 3.0f);
glm::vec3 cFrontInit = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cUpInit = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraPos = cPosInit;
glm::vec3 cameraFront = cFrontInit;
glm::vec3 cameraUp = cUpInit;
float lastX = WIDTH/2.0;
float lastY = HEIGHT/2.0;
bool firstFrame = true;
float yaw = -90.0f;
float pitch = 0.0f;
float fovy = 45.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lightsOn = false;

glm::vec3 lightPos = glm::vec3(5.0f, 2.0f, 3.0f);
GLfloat mUnits = -0.02f;
bool moveLight = false;


int main()
{
   
   //Initailize GLFW
	if (!glfwInit()) {
		printf("GLFW initialization failed. Exiting..\n");
		glfwTerminate();
		return 1;
	}

	//Setup GLFW Windows Properties
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//No backward compatibilty
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Forward Compatibilty
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//Create a window
	GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "The Classroom", NULL, NULL);
	if (!mainWindow) {
		printf("Window Not Created. Exiting..\n");
		glfwTerminate();
		return 1;
	}

	glfwSetKeyCallback(mainWindow, keyCallBack);
	//glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(mainWindow, mouseCallBack);

	//Get Buffer Size Info
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

    //load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, bufferWidth, bufferHeight);

    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
	Shader ourShader("shaders/vertexshader.glsl", "shaders/fragmentshader.glsl");
	Shader depthShader("shaders/depth_vs.glsl", "shaders/depth_fs.glsl");

	
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ourShader.use();
    ourShader.setInt("shadowMap", 1);


    // load models
    //Model ground("obj/ground.obj");
    //Model IronMan("obj/IronMan.obj");
    Model Shelby("obj/Shelby.obj");

    printf("Press Q to toggle lights on/off\n");
    printf("Press E to start/stop light movement\n");

    //Loop until window closed
    while (!glfwWindowShouldClose(mainWindow))
    {
        float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Get and Handle User Events 
		glfwPollEvents();

        glClearColor(0.41f, 0.41f, 0.41f, 0.9f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		updateLight();

        /********/
		glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 20.0f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            renderScene(depthShader,Shelby);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*********/
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ourShader.use();
        ourShader.setBool("lightsOn",lightsOn);
        ourShader.setVec3("cameraPos",cameraPos);
        ourShader.setVec3("lightPos1", lightPos);
        ourShader.setVec3("lightColor", vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        //construct and send projection matrix to shader
        glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(fovy), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        //construct and send view matrix to shader
        glm::mat4 view;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        renderScene(ourShader,Shelby);

        glfwSwapBuffers(mainWindow);
    }

    //terminate glfw
    glfwTerminate();
    return 0;
}

void updateLight(){

	if(!moveLight)
		return;

	if(lightPos.x>5.0f){
		lightPos.x = 5.0f;
		mUnits *=-1.0f;
	}
	else if(lightPos.x<-5.0f){
		lightPos.x = -5.0f;
		mUnits *=-1.0f;
	}

	lightPos.x = lightPos.x + mUnits;

}

void renderScene(Shader shader,Model model){
	createFloor(shader);
    
    // render the loaded models
    /*
    mat4 IronManMat(1.0f);
    IronManMat = translate(IronManMat, vec3(0.0f, 0.01f, 1.0f));
    IronManMat = rotate(IronManMat, radians(0.0f), vec3(0.0, 1.0, 0.0));
    IronManMat = scale(IronManMat, vec3(0.005f, 0.005f, 0.005f));
    shader.setMat4("model", IronManMat);
    IronMan.Draw(shader);
    */

    mat4 modelMat(1.0f);
    modelMat = translate(modelMat, vec3(0.0f, 0.01f, 1.0f));
    modelMat = rotate(modelMat, radians(90.0f), vec3(0.0, 1.0, 0.0));
    modelMat = scale(modelMat, vec3(0.6f, 0.6f, 0.6f));
    shader.setMat4("model", modelMat);
    model.Draw(shader);

}

void createFloor(Shader shader)
{
	/**
	 * Draws the floor of the classroom.
	 * Does so using two traingles.
	*/

	GLfloat vertices[] = {
		2.0f,  2.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		2.0f, -2.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f
	};


	GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	mat4 model(1.0f);
    model = translate(model, vec3(0.0f, 0.0f, 0.0f));
    model = rotate(model, radians(90.0f), vec3(1.0, 0.0, 0.0));
    model = scale(model, vec3(2.5f*0.75f, 2.8f*0.8f, 2.8f*0.9f));
    shader.setMat4("model", model);

	shader.setVec3("color", vec3(0.0f,0.8f,0.8f));
    glBindVertexArray(VAO);
    GLuint isize = sizeof(indices)/sizeof(GLuint);
    glDrawElements(GL_TRIANGLES, isize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}


void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/**
	 * Callback function to control what each key does.
	*/

	float cameraSpeed = 2.5 * deltaTime;
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_W:
			cameraPos += cameraSpeed * cameraFront;
			break;
		case GLFW_KEY_S:
			cameraPos -= cameraSpeed * cameraFront;
			break;
		case GLFW_KEY_A:
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case GLFW_KEY_D:
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case GLFW_KEY_R:
			cameraPos = cPosInit;
			cameraFront = cFrontInit;
			cameraUp = cUpInit;
			fovy = 45.0f;
			firstFrame = true;
			break;
		case GLFW_KEY_Q:
			if(lightsOn)
				lightsOn = false;
			else
				lightsOn = true;
			break;
		case GLFW_KEY_E:
			moveLight = !moveLight;
			break;
		}
	}
}


void mouseCallBack(GLFWwindow* window, double xpos, double ypos) 
{
	/**
	 * Callback function to control what mouse movements do.
	*/

	if (firstFrame)
	{
		lastX = xpos;
		lastY = ypos;
		firstFrame = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

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