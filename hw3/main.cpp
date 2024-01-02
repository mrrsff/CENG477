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
GLuint glyphProgram; // This is the program for the glyphs.

int gWidth, gHeight;

vector<GLint> modelingMatrixLoc;
vector<GLint> viewingMatrixLoc;
vector<GLint> projectionMatrixLoc;
vector<GLint> eyePosLoc;
GLfloat checkerboardScaleLoc, checkerboardOffsetLoc;

glm::mat4 projectionMatrix;
glm::mat4 orthoProjectionMatrix;
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

<<<<<<< HEAD
struct Character 
{ 
=======
struct Character { 
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

map<char, Character> Characters; // Bitmaps of the glyphs
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
	GLuint textureID; // texture object
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

Object bunny;
Object checkpoint1, checkpoint2, checkpoint3;
Object checkerboard;
Object skyboxObj;

struct Checkpoint
{
	GLfloat xPosition, zPosition;
	GLboolean isYellow, active;
	Object& obj = checkpoint1;

	friend ostream& operator<<(ostream& os, const Checkpoint& obj)
	{
		os << "xPosition: " << obj.xPosition << endl;
		os << "zPosition: " << obj.zPosition << endl;
		os << "isYellow: " << obj.isYellow << endl;
		os << "active: " << obj.active << endl;
		os << "obj: " << obj.obj << endl;
		return os;
	}
} checkpoints[3];


// Game variables
float displacement = 0.0f;
float speed = 10.0f;
float acceleration = 0.003f;
float horizontalMoveSpeed = 0.05f;
bool rightPressed = false;
bool leftPressed = false;
bool inHappyState = false;
float remainingTurnAngleHappy = 0.f;
bool isFainting = false;
float turnSpeed = 5.f;
float score = 0;
float checkpointRadius = 0.5f; // This is the radius of the checkpoint. If the player is within this radius, then they have reached the checkpoint.
float timeStamp = 0.f;
bool pause = false;
int checkerboardXRot = 90;
int checkerboardYRot = 180;
int checkerboardZRot = 0;
float checkerboardZOffset = 0.f;
<<<<<<< HEAD
glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 yellow = glm::vec3(1.0f, 1.0f, 0.0f);
glm::vec3 textColor = yellow;
bool restartPressed = false;
=======
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d

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
	else 
	{
		cout << "Unable to open file";
		assert(false);
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
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &channels, STBI_rgb);

	// Set up the textures
	glGenTextures(1, &skyboxObj.textureID);
	glBindTexture(GL_TEXTURE_2D, skyboxObj.textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
<<<<<<< HEAD

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // repeat texture on wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // repeat texture on wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // linearly interpolate texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linearly interpolate texture

	assert(glGetError() == GL_NONE);
=======
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
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

	assert(glGetError() == GL_NONE);

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

	assert(glGetError() == GL_NONE);

	return fs;
}

void initShaders()
{
	GLint status;

	// Create the programs
	gProgram.resize(5);

	for (int i = 0; i < gProgram.size(); ++i)
	{
		gProgram[i] = glCreateProgram();
	}
	auto err = glGetError();
	if (err != GL_NONE)
	{
		cout << "Error occurred when creating shader programs, error = " << err << endl;
		assert(false);
	}

	// Create the shaders for both programs
	GLuint vs1 = createVS("bunny_vert.glsl");
	GLuint fs1 = createFS("bunny_frag.glsl");
	// Attach the shaders to the programs
	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	bunnyProgram = 0;

	assert(glGetError() == GL_NONE);

	// Create the shaders for the checkerboard program
	GLuint vs2 = createVS("checkerboard_vert.glsl");
	GLuint fs2 = createFS("checkerboard_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	checkerboardProgram = 1;

	assert(glGetError() == GL_NONE);

	// Create the shaders for the skybox program
	GLuint vs3 = createVS("skybox_vert.glsl");
	GLuint fs3 = createFS("skybox_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	skyboxProgram = 2;

	assert(glGetError() == GL_NONE);

	// Create the shaders for the yellow checkpoints program
	GLuint vs4 = createVS("ycheckpoint_vert.glsl");
	GLuint fs4 = createFS("ycheckpoint_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[3], vs4);
	glAttachShader(gProgram[3], fs4);

	yellowCheckpointsProgram = 3;
	
	assert(glGetError() == GL_NONE);

	// Create the shaders for the red checkpoints program
	GLuint vs5 = createVS("rcheckpoint_vert.glsl");
	GLuint fs5 = createFS("rcheckpoint_frag.glsl");

	// Attach the shaders to the programs
	glAttachShader(gProgram[4], vs5);
	glAttachShader(gProgram[4], fs5);

	redCheckpointsProgram = 4;

	assert(glGetError() == GL_NONE);

	// Link the programs
	for (int i = 0; i < gProgram.size(); ++i)
	{
		glLinkProgram(gProgram[i]);
		assert(glGetError() == GL_NONE);
		glGetProgramiv(gProgram[i], GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			cout << "Failed to link shader program " << i << endl;
<<<<<<< HEAD
			assert(false);
=======
			exit(-1);
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
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

		assert(glGetError() == GL_NONE);
	}
	checkerboardScaleLoc = glGetUniformLocation(gProgram[checkerboardProgram], "scale");
	checkerboardOffsetLoc = glGetUniformLocation(gProgram[checkerboardProgram], "offset");

	assert(glGetError() == GL_NONE);
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

	err = glGetError();
	if (err != GL_NONE)
	{
		cout << "Error occurred when binding VBOs for " << obj.name << ", error = " << err << endl;
		error = true;
	}

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

	err = glGetError();
	if (err != GL_NONE)
	{
		cout << "Error occurred when copying data to VBOs for " << obj.name << ", error = " << err << endl;
		error = true;
	}

	// done copying to GPU memory; can free now from CPU memory
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // specify the vertex attributes (coordinates) in the buffer
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(obj.vertexDataSizeInBytes)); // specify the vertex attributes (normals) in the buffer

	err = glGetError();
	if (err != GL_NONE)
	{
		cout << "Error occurred when setting vertex attribute pointers for " << obj.name << ", error = " << err << endl;
		error = true;
	}

	glBindVertexArray(0); // unbind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind the VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind the VBO

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
	// Create program for the glyphs
	glyphProgram = glCreateProgram();

	// Create the shaders for the glyphs program
	GLuint vs = createVS("glyph_vert.glsl");
	GLuint fs = createFS("glyph_frag.glsl");

	// Attach the shaders to the program
	glAttachShader(glyphProgram, vs);
	glAttachShader(glyphProgram, fs);

	assert(glGetError() == GL_NONE);

	// Link the program
	glLinkProgram(glyphProgram);

	assert(glGetError() == GL_NONE);

	GLint status;
	glGetProgramiv(glyphProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		cout << "Failed to link shader program for glyphs" << endl;
		assert(false);
	}

	assert(glGetError() == GL_NONE);

	if (FT_Init_FreeType(&library))
	{
		cout << "Cannot init freetype library" << endl;
		assert(false);
	}
	FT_Face face;
	if (FT_New_Face(library, path, 0, &face))
	{
		cout << "Cannot open font file" << endl;
		assert(false);
	}
	FT_Set_Pixel_Sizes(face, 0, 96); // set font size

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	assert(glGetError() == GL_NONE);

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

		assert(glGetError() == GL_NONE);
		
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		assert(glGetError() == GL_NONE);

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

	assert(glGetError() == GL_NONE);

	// Free FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	// configure VAO/VBO for texture quads
<<<<<<< HEAD
	glGenVertexArrays(1, &charVAO); // create a VAO for the object
    glGenBuffers(1, &charVBO); // create a VBO for the vertex attributes
    glBindVertexArray(charVAO); // make it active
    glBindBuffer(GL_ARRAY_BUFFER, charVBO); // bind VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); // allocate space for VBO
    glEnableVertexAttribArray(0); // configure vertex attributes
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); // specify vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO
    glBindVertexArray(0); // unbind VAO

	assert(glGetError() == GL_NONE);
=======
	glGenVertexArrays(1, &charVAO);
    glGenBuffers(1, &charVBO);
    glBindVertexArray(charVAO);
    glBindBuffer(GL_ARRAY_BUFFER, charVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
}

void CreateCheckpoints()
{
	// Create checkpoints far away from the player, and translate them closer to the player according to the player's speed.
	
	checkpoints[0].xPosition = -1.75f;
<<<<<<< HEAD
	checkpoints[0].zPosition = -40.f;
=======
	checkpoints[0].zPosition = -20.f;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
	checkpoints[0].isYellow = false;
	checkpoints[0].active = true;
	checkpoints[0].obj = checkpoint1;

	checkpoints[1].xPosition = 0.f;
<<<<<<< HEAD
	checkpoints[1].zPosition = -40.f;
=======
	checkpoints[1].zPosition = -20.f;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
	checkpoints[1].isYellow = false;
	checkpoints[1].active = true;
	checkpoints[1].obj = checkpoint2;

	checkpoints[2].xPosition = 1.75f;
<<<<<<< HEAD
	checkpoints[2].zPosition = -40.f;
=======
	checkpoints[2].zPosition = -20.f;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
	checkpoints[2].isYellow = false;
	checkpoints[2].active = true;
	checkpoints[2].obj = checkpoint3;

	// Choose a random checkpoint to be yellow.
	int randomCheckpoint = rand() % 3;
	checkpoints[randomCheckpoint].isYellow = true;
}

int CheckCollision(float xDisplacement)
{
	// Check if the player has collided with a checkpoint. If so, return the index of the checkpoint. Otherwise, return -1.

	for (int i = 0; i < 3; i++)
	{
		if (checkpoints[i].active == false) continue;

		float xDistance = checkpoints[i].xPosition - xDisplacement;
		float zDistance = checkpoints[i].zPosition + 1.75f;
		float distance = sqrt(xDistance * xDistance + zDistance * zDistance);
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
	LoadFont("Antonio.ttf");

	// Initialize the bunny
	bunny = ParseObj("bunny.obj");
	checkerboard = ParseObj("quad.obj");
	checkpoint1 = ParseObj("cube.obj");
	checkpoint2 = ParseObj("cube.obj");
	checkpoint3 = ParseObj("cube.obj");
	skyboxObj = ParseObj("quad.obj");
	
	LoadImage("sky.jpg");

	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	initShaders();
	initVBO(checkerboard);
	initVBO(bunny);
	initVBO(checkpoint1);
	initVBO(checkpoint2);
	initVBO(checkpoint3);
	initVBO(skyboxObj);

	CreateCheckpoints();

	// Set up the projection matrix for the glyphs.
	orthoProjectionMatrix = glm::ortho(0.0f, static_cast<float>(gWidth), 0.0f, static_cast<float>(gHeight));
	glUseProgram(gProgram[glyphProgram]);
	glUniformMatrix4fv(glGetUniformLocation(gProgram[glyphProgram], "projection"), 1, GL_FALSE, glm::value_ptr(orthoProjectionMatrix));
<<<<<<< HEAD
}

float calculateHeight(float value)
{
	// Linear function for height
	// Get modulo of value with 2
	float modulo = fmod(value, 2.f);
	if (modulo <= 1.f)
	{
		return (modulo - 1.f)/5.f;
	}
	else
	{
		return (1.f - modulo)/5.f;
	}
=======
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
}

void drawBunny()
{	
<<<<<<< HEAD
	float height = calculateHeight(speed * (speed - 10.f)) - 1.25f;
=======
	float height = cos(speed * 150.f) / 10.f - 1.35f;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
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

	glm::vec3 scale = glm::vec3(1, 1, 1);
	scale *= 0.25f;
	// Compute the modeling matrix 
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(displacement, height, -1.75f)); // Translate to the right and down
	glm::mat4 matS = glm::scale(glm::mat4(1.0), scale); // Scale down by 50%
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

	// Bind the VBOs and VAO
	glBindVertexArray(bunny.vao);
	glBindBuffer(GL_ARRAY_BUFFER, bunny.vertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunny.indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(bunny.vertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, bunny.faces.size() * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void drawCheckerboard()
{
	// Set up the values of the modeling matrix for the checkerboard.
<<<<<<< HEAD
	glm::vec3 scale = glm::vec3(3, 1, 40);
	glm::vec3 offset = glm::vec3(0, -1.5f, -20.f);
=======
	glm::vec3 scale = glm::vec3(3, 1, 20);
	glm::vec3 offset = glm::vec3(0, -1.5f, -10.f);
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
	// Compute the modeling matrix 
	glm::mat4 matT = glm::translate(glm::mat4(1.0), offset);
	glm::mat4 matS = glm::scale(glm::mat4(1.0), scale);
	// Rotate so that the checkerboard is on the xz plane
	glm::mat4 matXR = glm::rotate<float>(glm::mat4(1.0), (checkerboardXRot / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 matYR = glm::rotate<float>(glm::mat4(1.0), (checkerboardYRot / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 matZR = glm::rotate<float>(glm::mat4(1.0), (checkerboardZRot / 180.) * M_PI, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix = matT * matS * matXR * matYR * matZR;

	// We do this after computing the modeling matrix so that the checkerboard doesn't actually move but the fragment shader will move the texture.
	offset.z = -checkerboardZOffset * 2;

	// Set the active program and the values of its uniform variables	
	glUseProgram(gProgram[checkerboardProgram]);
	glUniformMatrix4fv(projectionMatrixLoc[checkerboardProgram], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[checkerboardProgram], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[checkerboardProgram], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[checkerboardProgram], 1, glm::value_ptr(eyePos));
	glUniform3fv(checkerboardScaleLoc, 1, glm::value_ptr(scale));
	glUniform1f(checkerboardOffsetLoc, offset.z);

	// Bind the VBOs and VAO
	glBindVertexArray(checkerboard.vao);
	glBindBuffer(GL_ARRAY_BUFFER, checkerboard.vertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, checkerboard.indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(checkerboard.vertexDataSizeInBytes));

	
	glDrawElements(GL_TRIANGLES, checkerboard.faces.size() * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void drawCheckpoints()
{
	for (int i = 0; i < 3; i++)
	{
		if (checkpoints[i].active == false) continue;
		
		// Set up the values of the modeling matrix for the yellow checkpoints.
		glm::vec3 offset = glm::vec3(checkpoints[i].xPosition, -0.75f, checkpoints[i].zPosition);
		glm::vec3 scale = glm::vec3(0.25f,0.5f,0.25f);

		// Compute the modeling matrix 
		glm::mat4 matT = glm::translate(glm::mat4(1.0), offset);
		glm::mat4 matS = glm::scale(glm::mat4(1.0), scale);
		modelingMatrix = matT * matS;
		
		int program = checkpoints[i].isYellow ? yellowCheckpointsProgram : redCheckpointsProgram;
<<<<<<< HEAD
=======
		
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
		// Set the active program and the values of its uniform variables
		glUseProgram(gProgram[program]);
		assert(glGetError() == GL_NONE);
		glUniformMatrix4fv(projectionMatrixLoc[program], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(viewingMatrixLoc[program], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
		glUniformMatrix4fv(modelingMatrixLoc[program], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
		glUniform3fv(eyePosLoc[program], 1, glm::value_ptr(eyePos));

		// Bind the VBOs and VAO
		glBindVertexArray(checkpoints[i].obj.vao);
		glBindBuffer(GL_ARRAY_BUFFER, checkpoints[i].obj.vertexAttribBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, checkpoints[i].obj.indexBuffer);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(checkpoints[i].obj.vertexDataSizeInBytes));
		
		glDrawElements(GL_TRIANGLES, checkpoints[i].obj.faces.size() * 3, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
}

void drawSkybox()
{
	// Set up the values of the modeling matrix for the skybox.
<<<<<<< HEAD
	modelingMatrix = glm::mat4(1.0); // Identity matrix for the skybox
=======
	glm::vec3 scale = glm::vec3(100, 100, 100);
	glm::vec3 offset = glm::vec3(0, 0, -50.f);
	// Compute the modeling matrix 
	glm::mat4 matT = glm::translate(glm::mat4(1.0), offset);
	glm::mat4 matS = glm::scale(glm::mat4(1.0), scale);
	modelingMatrix = matT * matS;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d

	// Set the active program and the values of its uniform variables	
	glUseProgram(gProgram[skyboxProgram]);
	glUniformMatrix4fv(projectionMatrixLoc[skyboxProgram], 1, GL_FALSE, glm::value_ptr(orthoProjectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[skyboxProgram], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[skyboxProgram], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[skyboxProgram], 1, glm::value_ptr(eyePos));

	// Bind the VBOs and VAO
	glBindVertexArray(skyboxObj.vao);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxObj.vertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxObj.indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(skyboxObj.vertexDataSizeInBytes));

	glActiveTexture(GL_TEXTURE0);
<<<<<<< HEAD
	glBindTexture(GL_TEXTURE_2D, skyboxObj.textureID);
=======
	// glBindTexture(GL_TEXTURE_2D, skybox.texture);
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
	glDrawElements(GL_TRIANGLES, skyboxObj.faces.size() * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
	glUseProgram(glyphProgram);
	glUniformMatrix4fv(glGetUniformLocation(glyphProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProjectionMatrix));
    glUniform3f(glGetUniformLocation(glyphProgram, "textColor"), color.x, color.y, color.z);
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
	assert(glGetError() == GL_NONE);
}

void drawScore()
{
<<<<<<< HEAD
	RenderText("Score: " + to_string((int)score), 0.f, gHeight - 100, 1.f, textColor);
}

void drawPause()
{
	RenderText("PAUSED", gWidth / 2 - 125, gHeight / 2 - 50, 1.f, glm::vec3(1.f, 1.f, 1.f));
=======
	RenderText(gProgram[glyphProgram], "Score: " + to_string(score), 10.f, 10.f, 5.f, glm::vec3(1,1,1));
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
}

void display()
{
	glClearColor(0.3, 0.5, 0.3, 1);
	glClearDepth(1.f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
<<<<<<< HEAD
	// Disable z-buffering so that the skybox is always drawn on top of everything else.
	glDisable(GL_DEPTH_TEST);
	drawSkybox();
	drawScore();
	// Re-enable z-buffering.
	glEnable(GL_DEPTH_TEST);

	drawCheckpoints();
	drawCheckerboard();
	drawBunny();
=======
	drawSkybox();
	drawCheckpoints();
	drawCheckerboard();
	drawBunny();
	drawScore();
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
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

	// Use orthographic projection for the glyphs
	orthoProjectionMatrix = glm::ortho(0.0f, static_cast<float>(w), 0.0f, static_cast<float>(h));

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
<<<<<<< HEAD
	score += 1000;
	textColor = yellow;
=======
	score += 5;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
}

void setFaint()
{
	isFainting = true;
	speed = 0.f;
	acceleration = 0.f;
	horizontalMoveSpeed = 0.f;
	turnSpeed = 0.f;
	textColor = red;
}

void Reset()
{
	isFainting = false;
	speed = 10.0f;
	horizontalMoveSpeed = 0.05f;
	acceleration = 0.003f;
	turnSpeed = 5.f;
	rightPressed = false;
	leftPressed = false;
	inHappyState = false;
	remainingTurnAngleHappy = 0.f;
	displacement = 0.0f;
	score = 0;
	textColor = yellow;
	checkerboardZOffset = 0.f;
	CreateCheckpoints();
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			// case GLFW_KEY_UP:
			// case GLFW_KEY_W:
			// 	eyePos.z -= 1.f;
			// 	break;
			// case GLFW_KEY_DOWN:
			// case GLFW_KEY_S:
			// 	eyePos.z += 1.f;
			// 	break;
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
				restartPressed = true;
				break;
			// case GLFW_KEY_H: // TODO: remove this later
			// 	setHappy();
			// 	break;
			// case GLFW_KEY_F: // TODO: remove this later
			// 	setFaint();
			// 	break;
			case GLFW_KEY_P:
				pause = !pause;
				break;
			// case GLFW_KEY_X:
			// 	checkerboardXRot += 45;
			// 	checkerboardXRot %= 360;
			// 	cout << "Checkerboard total rotation: " << checkerboardXRot << " " << checkerboardYRot << " " << checkerboardZRot << endl;
			// 	break;
			// case GLFW_KEY_Y:
			// 	checkerboardYRot += 45;
			// 	checkerboardYRot %= 360;
			// 	cout << "Checkerboard total rotation: " << checkerboardXRot << " " << checkerboardYRot << " " << checkerboardZRot << endl;
			// 	break;
			// case GLFW_KEY_Z:
			// 	checkerboardZRot += 45;
			// 	checkerboardZRot %= 360;
			// 	cout << "Checkerboard total rotation: " << checkerboardXRot << " " << checkerboardYRot << " " << checkerboardZRot << endl;
			// 	break;
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
			case GLFW_KEY_R:
				restartPressed = false;
				break;
		}
	}
}

void calculateValues(){
	if (isFainting) return;

	if (rightPressed)
	{
		displacement = min(displacement + horizontalMoveSpeed, 2.0f);
	}
	else if (leftPressed)
	{
		displacement = max(displacement - horizontalMoveSpeed, -2.0f);
	}
	speed += acceleration;
<<<<<<< HEAD
	float speedMulti = 0.5f;
	for (int i = 0; i < 3; i++)
	{
		checkpoints[i].zPosition += speed * 0.01f * speedMulti;
=======
	for (int i = 0; i < 3; i++)
	{
		checkpoints[i].zPosition += speed * 0.01f;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
		if (checkpoints[i].zPosition > 0.f)
		{
			CreateCheckpoints();
			break;
		}
	}
<<<<<<< HEAD
	checkerboardZOffset += speed * 0.006f * speedMulti;
	score += 0.25f;
=======
	checkerboardZOffset += speed * 0.006f;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
}

void checkCollision()
{
	int checkpointIndex = CheckCollision(displacement);
	if (checkpointIndex == -1) return;
	if (checkpoints[checkpointIndex].active == false) return;
	
	checkpoints[checkpointIndex].active = false;

	if (checkpoints[checkpointIndex].isYellow) setHappy();
	else setFaint();
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
<<<<<<< HEAD
		if (pause) {
			glDisable(GL_DEPTH_TEST);
			drawPause();
			glEnable(GL_DEPTH_TEST);
			continue;
		}
		if (restartPressed) Reset();
		timeStamp += 0.01f;
=======
		if (pause) continue;
		timeStamp += 0.01f;
		score += 0.01f;
>>>>>>> 339b027af9adc05e89730ba66f9008772af30c9d
		display();
		checkCollision();
		calculateValues();
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
	window = glfwCreateWindow(width, height, "CENG477 HW3 Bunny Run", NULL, NULL);

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
