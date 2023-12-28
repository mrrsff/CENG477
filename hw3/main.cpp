#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h> 
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[2];
int gWidth, gHeight;

GLint modelingMatrixLoc[2];
GLint viewingMatrixLoc[2];
GLint projectionMatrixLoc[2];
GLint eyePosLoc[2];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct Image
{
	int width, height, channels;
	unsigned char* data;
} image;

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

float displacement = 0.0f;
float speed = 10.0f;
float acceleration = 0.01f;
float moveSpeed = 0.025f;
bool rightPressed = false;
bool leftPressed = false;
bool inHappyState = false;
float remainingTurnAngleHappy = 0.f;
bool isFainting = false;
float turnSpeed = 5.f;

struct Checkpoint
{
	GLfloat xPosition, zPosition;
	GLboolean isYellow, rendered;
} checkpoints[3];

float checkpointRadius = 0.25f; // This is the radius of the checkpoint. If the player is within this radius, then they have reached the checkpoint.

void CreateCheckpoints()
{
	// Create checkpoints far away from the player, and translate them closer to the player according to the player's speed.
	
	checkpoints[0].xPosition = -1.f;
	checkpoints[0].zPosition = -10.f;
	checkpoints[0].isYellow = false;

	checkpoints[1].xPosition = 0.f;
	checkpoints[1].zPosition = -10.f;
	checkpoints[1].isYellow = false;

	checkpoints[2].xPosition = 1.f;
	checkpoints[2].zPosition = -10.f;
	checkpoints[2].isYellow = false;

	// Choose a random checkpoint to be yellow.

	int randomCheckpoint = rand() % 3;
	checkpoints[randomCheckpoint].isYellow = true;

	// Print all checkpoints to the console.
	for (int i = 0; i < 3; i++)
	{
		cout << "Checkpoint " << i+1 << ": " << checkpoints[i].isYellow << endl;
	}
	std::cout << std::endl;


}

int CheckCollision(float xDisplacement)
{
	// Check if the player has collided with a checkpoint. If so, return the index of the checkpoint. Otherwise, return -1.

	for (int i = 0; i < 3; i++)
	{
		float xDistance = checkpoints[i].xPosition - xDisplacement;
		float zDistance = checkpoints[i].zPosition - 3.f;

		float distance = sqrt(pow(xDistance, 2) + pow(zDistance, 2));

		if (distance <= checkpointRadius)
		{
			return i;
		}		
	}

	return -1;
}

bool ParseObj(const string& fileName)
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						gTextures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					gFaces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x,
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x,
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x,
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices.size() == gNormals.size());

	return true;
}

bool ReadDataFromFile( const string& fileName, string& data)
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

	gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();

	// Create the shaders for both programs

	GLuint vs1 = createVS("bunny_vert.glsl");
	GLuint fs1 = createFS("bunny_frag.glsl");
	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs1);
	glAttachShader(gProgram[1], fs1);

	// Link the programs

	glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO()
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	assert(vao > 0);
	glBindVertexArray(vao);
	cout << "vao = " << vao << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer);
	glGenBuffers(1, &gIndexBuffer);

	assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
	gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat[gVertices.size() * 3];
	GLfloat* normalData = new GLfloat[gNormals.size() * 3];
	GLuint* indexData = new GLuint[gFaces.size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < gVertices.size(); ++i)
	{
		vertexData[3 * i] = gVertices[i].x;
		vertexData[3 * i + 1] = gVertices[i].y;
		vertexData[3 * i + 2] = gVertices[i].z;

		minX = std::min(minX, gVertices[i].x);
		maxX = std::max(maxX, gVertices[i].x);
		minY = std::min(minY, gVertices[i].y);
		maxY = std::max(maxY, gVertices[i].y);
		minZ = std::min(minZ, gVertices[i].z);
		maxZ = std::max(maxZ, gVertices[i].z);
	}

	// std::cout << "minX = " << minX << std::endl;
	// std::cout << "maxX = " << maxX << std::endl;
	// std::cout << "minY = " << minY << std::endl;
	// std::cout << "maxY = " << maxY << std::endl;
	// std::cout << "minZ = " << minZ << std::endl;
	// std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < gNormals.size(); ++i)
	{
		normalData[3 * i] = gNormals[i].x;
		normalData[3 * i + 1] = gNormals[i].y;
		normalData[3 * i + 2] = gNormals[i].z;
	}

	for (int i = 0; i < gFaces.size(); ++i)
	{
		indexData[3 * i] = gFaces[i].vIndex[0];
		indexData[3 * i + 1] = gFaces[i].vIndex[1];
		indexData[3 * i + 2] = gFaces[i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying to GPU memory; can free now from CPU memory
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void LoadImage(const string& fileName)
{
	int width, height, channels;
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &channels, 0);

	if (data)
	{
		image.width = width;
		image.height = height;
		image.channels = channels;
		image.data = data;
	}
	else
	{
		cout << "Cannot load image: " << fileName << endl;
		exit(-1);
	}
}

void init()
{
	//ParseObj("armadillo.obj");
	ParseObj("bunny.obj");
	LoadImage("sky.jpg");
	CreateCheckpoints();

	glEnable(GL_DEPTH_TEST);
	initShaders();
	initVBO();
}

void drawModel()
{
	//glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}


void display()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	float height = cos(speed * 10.f) / 10.f - 2.f;
	float yRot = (-90. / 180.) * M_PI;
	float zRot = -1.f;
	if (inHappyState)
	{
		yRot -= remainingTurnAngleHappy / 180.f * M_PI;
		remainingTurnAngleHappy -= turnSpeed;
		if (remainingTurnAngleHappy <= 0.f)
		{
			inHappyState = false;
		}
	}

	// Compute the modeling matrix 
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(displacement, height, -3.f)); // Translate to the right and down
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.5)); // Scale down by 50%
	glm::mat4 matYR = glm::rotate<float>(glm::mat4(1.0), yRot, glm::vec3(0.0, 1.0, 0.0)); // Rotate around Y axis to turn back
	if (isFainting)
	{
		speed = 0.f;
		acceleration = 0.f;
		zRot = (-90. / 180.) * M_PI;
		glm::mat4 matZR = glm::rotate<float>(glm::mat4(1.0), zRot, glm::vec3(0.0, 0.0, 1.0)); // Rotate around Z axis to fall down
		modelingMatrix = matT * matS * matZR * matYR; // starting from right side
	}
	else
		modelingMatrix = matT * matS * matYR; // starting from right side

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

	// Draw the scene
	drawModel();
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	// Use perspective projection
	float fovyRad = (float)(90.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)
	// 
	//viewingMatrix = glm::mat4(1);
	viewingMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

}

void setHappy()
{
	inHappyState = true;
	remainingTurnAngleHappy = 360.f;
}

void setFaint()
{
	isFainting = true;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			case GLFW_KEY_RIGHT:
			case GLFW_KEY_D:
				rightPressed = true;
				break;
			case GLFW_KEY_LEFT:
			case GLFW_KEY_A:
				leftPressed = true;
				break;
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;
			case GLFW_KEY_R:
				// TODO: reset game.
				break;
			case GLFW_KEY_H: // TODO: remove this later
				setHappy();
				break;
			case GLFW_KEY_F: // TODO: remove this later
				setFaint();
				break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		switch (key)
		{
			case GLFW_KEY_RIGHT:
			case GLFW_KEY_D:
				rightPressed = false;
				break;
			case GLFW_KEY_LEFT:
			case GLFW_KEY_A:
				leftPressed = false;
				break;
		}
	}
}

void calculateValues(){
	if (rightPressed)
	{
		displacement = min(displacement + moveSpeed, 2.0f);
	}
	else if (leftPressed)
	{
		displacement = max(displacement - moveSpeed, -2.0f);
	}
	speed += acceleration;
}

void checkCollision()
{
	int checkpointIndex = CheckCollision(displacement);
	if (checkpointIndex == -1) return;

	if (checkpoints[checkpointIndex].isYellow)
	{
		setHappy();
	}
	else
	{
		setFaint();
	}
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		checkCollision();
		calculateValues();
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this if on MacOS

	int width = 1000, height = 800;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };

#if defined(_WIN32) // If defined windows use strcpy_s instead of strcpy and strcat_s inst ead of strcat
	strcpy_s(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat_s(rendererInfo, " - ");
	strcat_s(rendererInfo, (const char*)glGetString(GL_VERSION));
#else
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
#endif
	glfwSetWindowTitle(window, rendererInfo);

	init();

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
