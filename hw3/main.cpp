#define _USE_MATH_DEFINES

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <math.h> 

#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header

#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H


using namespace std;

FT_Library library;

vector<GLuint> gProgram; // Each program contains a vertex shader and a fragment shader. For each program, we store its ID here.
int bunnyProgram; // This is the program for the bunny.
int skyboxProgram; // This is the program for the skybox.
int checkerboardProgram; // This is the program for the checkerboard.
int yellowCheckpointsProgram; // This is the program for the yellow checkpoints.
int redCheckpointsProgram; // This is the program for the red checkpoints.
int glyphProgram; // This is the program for the glyphs.

int gWidth, gHeight;

vector<GLint> modelingMatrixLoc;
vector<GLint> viewingMatrixLoc;
vector<GLint> projectionMatrixLoc;
vector<GLint> eyePosLoc;
GLfloat checkerboardScaleLoc, checkerboardOffsetLoc;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

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
} skybox;

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

map<char, Character> Characters;
unsigned int charVAO, charVBO;

struct Object
{
	string name;
	vector<Vertex> vertices;
	vector<Texture> textures;
	vector<Normal> normals;
	vector<Face> faces;
	GLuint vertexAttribBuffer, indexBuffer; // vbo's
	GLint inVertexLoc, inNormalLoc; // attribute locations
	GLuint vao; // vertex array object
	int vertexDataSizeInBytes, normalDataSizeInBytes;

	Object() {
	}

	friend ostream& operator<<(ostream& os, const Object& obj)
	{
		os << "Vertices: " << endl;
		for (int i = 0; i < obj.vertices.size(); ++i)
		{
			os << obj.vertices[i].x << " " << obj.vertices[i].y << " " << obj.vertices[i].z << endl;
		}

		os << "Normals: " << endl;
		for (int i = 0; i < obj.normals.size(); ++i)
		{
			os << obj.normals[i].x << " " << obj.normals[i].y << " " << obj.normals[i].z << endl;
		}

		os << "Faces: " << endl;
		for (int i = 0; i < obj.faces.size(); ++i)
		{
			os << obj.faces[i].vIndex[0] << " " << obj.faces[i].vIndex[1] << " " << obj.faces[i].vIndex[2] << endl;
		}

		return os;
	}
};

struct Checkpoint
{
	GLfloat xPosition, zPosition;
	GLboolean isYellow, rendered;
} checkpoints[3];

Object bunny;
Object checkpoint1, checkpoint2, checkpoint3;
Object checkerboard;

// Game variables
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
float score = 0;
float checkpointRadius = 0.1f; // This is the radius of the checkpoint. If the player is within this radius, then they have reached the checkpoint.
float timeStamp = 0.f;

Object ParseObj(const string& fileName)
{
	Object obj;
	fstream myfile;
	// Strip .obj from the file name
	obj.name = fileName.substr(0, fileName.length() - 4);

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
						obj.textures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						obj.normals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						obj.vertices.push_back(Vertex(c1, c2, c3));
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

					obj.faces.push_back(Face(vIndex, tIndex, nIndex));
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

	assert(obj.vertices.size() == obj.normals.size());
	return obj;
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

void LoadImage(const string& fileName)
{
	int width, height, channels;
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &channels, 0);

	if (data)
	{
		skybox.width = width;
		skybox.height = height;
		skybox.channels = channels;
		skybox.data = data;
	}
	else
	{
		cout << "Cannot load skybox: " << fileName << endl;
		exit(-1);
	}
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
	if (strlen(output) > 0) printf("VS compile log: %s\n", output);

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
	if (strlen(output) > 0) printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs
	gProgram.resize(6);

	for (int i = 0; i < gProgram.size(); ++i)
	{
		gProgram[i] = glCreateProgram();
	}

	// Create the shaders for both programs

	GLuint vs1 = createVS("bunny_vert.glsl");
	GLuint fs1 = createFS("bunny_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	bunnyProgram = 0;

	// Create the shaders for the checkerboard program
	GLuint vs2 = createVS("checkerboard_vert.glsl");
	GLuint fs2 = createFS("checkerboard_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	checkerboardProgram = 1;

	checkerboardScaleLoc = glGetUniformLocation(gProgram[1], "scale");
	checkerboardOffsetLoc = glGetUniformLocation(gProgram[1], "offset");

	// Create the shaders for the skybox program
	GLuint vs3 = createVS("skybox_vert.glsl");
	GLuint fs3 = createFS("skybox_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	skyboxProgram = 2;

	// Create the shaders for the yellow checkpoints program
	GLuint vs4 = createVS("ycheckpoint_vert.glsl");
	GLuint fs4 = createFS("ycheckpoint_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[3], vs4);
	glAttachShader(gProgram[3], fs4);

	yellowCheckpointsProgram = 3;

	// Create the shaders for the red checkpoints program
	GLuint vs5 = createVS("rcheckpoint_vert.glsl");
	GLuint fs5 = createFS("rcheckpoint_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[4], vs5);
	glAttachShader(gProgram[4], fs5);

	redCheckpointsProgram = 4;

	// Create the shaders for the glyphs program
	GLuint vs6 = createVS("glyph_vert.glsl");
	GLuint fs6 = createFS("glyph_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[5], vs6);
	glAttachShader(gProgram[5], fs6);

	glyphProgram = 5;

	// Link the programs
	GLint status;

	for (int i = 0; i < gProgram.size(); ++i)
	{
		glLinkProgram(gProgram[i]);
		glGetProgramiv(gProgram[i], GL_LINK_STATUS, &status);

		if (status != GL_TRUE)
		{
			cout << "Program " << i << " link failed" << endl;
			exit(-1);
		}
	}

	// Get the locations of the uniform variables from both programs
	modelingMatrixLoc.resize(gProgram.size());
	viewingMatrixLoc.resize(gProgram.size());
	projectionMatrixLoc.resize(gProgram.size());
	eyePosLoc.resize(gProgram.size());

	for (int i = 0; i < gProgram.size(); ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO(Object& obj)
{
	bool error = false;
	cout << "Initializing VBO for " << obj.name << endl;
	glGenVertexArrays(1, &obj.vao); // create a VAO for the object
	assert(obj.vao > 0); // make sure VAO has been created
	cout << obj.name << " VAO: " << obj.vao << endl;
	glBindVertexArray(obj.vao); // make it active
	auto err = glGetError();
	if (err != GL_NONE)
	{
		cout << "Error occurred when binding VAO for " << obj.name << ", error = " << err << endl;
		error = true;
	}

	glEnableVertexAttribArray(0); // inVertex
	glEnableVertexAttribArray(1); // inNormal
	err = glGetError();
	if (err != GL_NONE)
	{
		cout << "Error occurred when enabling vertex attributes for " << obj.name << ", error = " << err << endl;
		error = true;
	}

	glGenBuffers(1, &obj.vertexAttribBuffer); // create a VBO for the vertex attributes
	glGenBuffers(1, &obj.indexBuffer); // create a VBO for the indices

	assert(obj.vertexAttribBuffer > 0 && obj.indexBuffer > 0); // make sure VBOs have been created

	glBindBuffer(GL_ARRAY_BUFFER, obj.vertexAttribBuffer); // make it active, it is an array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.indexBuffer); // make it active, it is an element array buffer

	obj.vertexDataSizeInBytes = obj.vertices.size() * 3 * sizeof(GLfloat); // number of vertices * 3 coordinates * size of GLfloat
	obj.normalDataSizeInBytes = obj.normals.size() * 3 * sizeof(GLfloat); // number of normals * 3 coordinates * size of GLfloat
	int indexDataSizeInBytes = obj.faces.size() * 3 * sizeof(GLuint); // number of faces * 3 vertices per face * size of GLuint
	GLfloat* vertexData = new GLfloat[obj.vertices.size() * 3]; // array for vertex data
	GLfloat* normalData = new GLfloat[obj.normals.size() * 3]; // array for normal data
	GLuint* indexData = new GLuint[obj.faces.size() * 3]; // array for index data

	// copy the vertex attributes to the buffer object
	for (int i = 0; i < obj.vertices.size(); ++i)
	{
		vertexData[3 * i] = obj.vertices[i].x;
		vertexData[3 * i + 1] = obj.vertices[i].y;
		vertexData[3 * i + 2] = obj.vertices[i].z;
	}

	// copy the normal attributes to the buffer object
	for (int i = 0; i < obj.normals.size(); ++i)
	{
		normalData[3 * i] = obj.normals[i].x;
		normalData[3 * i + 1] = obj.normals[i].y;
		normalData[3 * i + 2] = obj.normals[i].z;
	}

	// copy the index data to the buffer object
	for (int i = 0; i < obj.faces.size(); ++i)
	{
		indexData[3 * i] = obj.faces[i].vIndex[0];
		indexData[3 * i + 1] = obj.faces[i].vIndex[1];
		indexData[3 * i + 2] = obj.faces[i].vIndex[2];
	}

	glBufferData(GL_ARRAY_BUFFER, obj.vertexDataSizeInBytes + obj.normalDataSizeInBytes, 0, GL_STATIC_DRAW); // allocate space and write the vertex data and normal data to the buffer object at once
	glBufferSubData(GL_ARRAY_BUFFER, 0, obj.vertexDataSizeInBytes, vertexData); // write the vertex data starting from offset 0
	glBufferSubData(GL_ARRAY_BUFFER, obj.vertexDataSizeInBytes, obj.normalDataSizeInBytes, normalData); // write the normal data starting from offset vertexDataSizeInBytes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW); // allocate space and write the index data to the buffer object

	// done copying to GPU memory; can free now from CPU memory
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // specify the vertex attributes (coordinates) in the buffer
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(obj.vertexDataSizeInBytes)); // specify the vertex attributes (normals) in the buffer

	if (error)
	{
		cout << "VBO initialization failed for " << obj.name << endl;
	}
	else
	{
		cout << "VBO initialization succeeded for " << obj.name << endl;
	}
}

void LoadFont(const char* path)
{
	if (FT_Init_FreeType(&library))
	{
		cout << "Cannot init freetype library" << endl;
		exit(-1);
	}
	FT_Face face;
	if (FT_New_Face(library, path, 0, &face))
	{
		cout << "Cannot open font file" << endl;
		exit(-1);
	}
	FT_Set_Pixel_Sizes(face, 0, 48);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
  
	// Load first 128 characters of ASCII set
	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture, 
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			(unsigned int) (face->glyph->advance.x)
		};
		Characters.insert(pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// Free FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	glGenVertexArrays(1, &charVAO);
    glGenBuffers(1, &charVBO);
    glBindVertexArray(charVAO);
    glBindBuffer(GL_ARRAY_BUFFER, charVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

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
	// for (int i = 0; i < 3; i++)
	// {
	// 	cout << "Checkpoint " << i+1 << ": " << checkpoints[i].isYellow << endl;
	// }
	// std::cout << std::endl;


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

void init()
{
	// Initialize freetype
	LoadFont("Roboto.ttf");

	// Initialize the bunny
	bunny = ParseObj("bunny.obj");
	checkerboard = ParseObj("quad.obj");
	// checkpoint1 = ParseObj("checkpoint.obj");
	// checkpoint2 = ParseObj("checkpoint.obj");
	// checkpoint3 = ParseObj("checkpoint.obj");
	
	LoadImage("sky.jpg");
	CreateCheckpoints();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
	initShaders();
	initVBO(bunny);
	initVBO(checkerboard);
}

void drawBunny()
{	
	float height = cos(speed * 10.f) / 10.f - 2.f;
	float yRot = (-90. / 180.) * M_PI;
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
		float zRot = (-90. / 180.) * M_PI;
		glm::mat4 matZR = glm::rotate<float>(glm::mat4(1.0), zRot, glm::vec3(0.0, 0.0, 1.0)); // Rotate around Z axis to fall down
		modelingMatrix = matT * matS * matZR * matYR; // starting from right side
	}
	else
		modelingMatrix = matT * matS * matYR; // starting from right side

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[bunnyProgram]);
	glUniformMatrix4fv(projectionMatrixLoc[bunnyProgram], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[bunnyProgram], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[bunnyProgram], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[bunnyProgram], 1, glm::value_ptr(eyePos));

	glBindBuffer(GL_ARRAY_BUFFER, bunny.vertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunny.indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(bunny.vertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, bunny.faces.size() * 3, GL_UNSIGNED_INT, 0);
}

void drawCheckerboard()
{
	glm::vec3 scale = glm::vec3(5, 0.5, 20);
	glm::vec3 offset = glm::vec3(0, -2, 0);
	// Compute the modeling matrix 
	glm::mat4 matT = glm::translate(glm::mat4(1.0), offset);
	glm::mat4 matS = glm::scale(glm::mat4(1.0), scale);

	modelingMatrix = matT * matS;

	offset += glm::vec3(0, -1, 0) * timeStamp; // We do this after computing the modeling matrix so that the checkerboard doesn't move with the player but the offset does.

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[checkerboardProgram]);
	glUniformMatrix4fv(projectionMatrixLoc[checkerboardProgram], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[checkerboardProgram], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[checkerboardProgram], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[checkerboardProgram], 1, glm::value_ptr(eyePos));

	glBindBuffer(GL_ARRAY_BUFFER, checkerboard.vertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, checkerboard.indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(checkerboard.vertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, checkerboard.faces.size() * 3, GL_UNSIGNED_INT, 0);
}

void drawCheckpoints()
{

}

void drawSkybox()
{

}
void RenderText(GLuint programId, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
	glUseProgram(programId);
    glUniform3f(glGetUniformLocation(programId, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(charVAO);

    // iterate through all characters
    for (string::const_iterator c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, charVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawScore()
{

}

void display()
{
	glClearColor(123, 123, 123, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	drawBunny();
	drawCheckerboard();
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
	speed = 0.f;
	acceleration = 0.f;
	moveSpeed = 0.f;
	turnSpeed = 0.f;
}

void Reset()
{
	displacement = 0.0f;
	speed = 10.0f;
	acceleration = 0.01f;
	moveSpeed = 0.025f;
	rightPressed = false;
	leftPressed = false;
	inHappyState = false;
	remainingTurnAngleHappy = 0.f;
	isFainting = false;
	turnSpeed = 5.f;
	score = 0;
	CreateCheckpoints();
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
				Reset();
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
		timeStamp += 0.01f;
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
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
#else
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
#endif
	glfwSetWindowTitle(window, rendererInfo);
	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	init();

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

