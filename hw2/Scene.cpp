#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include "tinyxml2.h"
#include "Triangle.h"
#include "Helpers.h"
#include "Scene.h"

using namespace tinyxml2;
using namespace std;

/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *xmlElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *rootNode = xmlDoc.FirstChild();

	// read background color
	xmlElement = rootNode->FirstChildElement("BackgroundColor");
	str = xmlElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	xmlElement = rootNode->FirstChildElement("Culling");
	if (xmlElement != NULL)
	{
		str = xmlElement->GetText();

		if (strcmp(str, "enabled") == 0)
		{
			this->cullingEnabled = true;
		}
		else
		{
			this->cullingEnabled = false;
		}
	}

	// read cameras
	xmlElement = rootNode->FirstChildElement("Cameras");
	XMLElement *camElement = xmlElement->FirstChildElement("Camera");
	XMLElement *camFieldElement;
	while (camElement != NULL)
	{
		Camera *camera = new Camera();

		camElement->QueryIntAttribute("id", &camera->cameraId);

		// read projection type
		str = camElement->Attribute("type");

		if (strcmp(str, "orthographic") == 0)
		{
			camera->projectionType = ORTOGRAPHIC_PROJECTION;
		}
		else
		{
			camera->projectionType = PERSPECTIVE_PROJECTION;
		}

		camFieldElement = camElement->FirstChildElement("Position");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->position.x, &camera->position.y, &camera->position.z);

		camFieldElement = camElement->FirstChildElement("Gaze");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->gaze.x, &camera->gaze.y, &camera->gaze.z);

		camFieldElement = camElement->FirstChildElement("Up");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->v.x, &camera->v.y, &camera->v.z);

		camera->gaze = normalizeVec3(camera->gaze);
		camera->u = crossProductVec3(camera->gaze, camera->v);
		camera->u = normalizeVec3(camera->u);

		camera->w = inverseVec3(camera->gaze);
		camera->v = crossProductVec3(camera->u, camera->gaze);
		camera->v = normalizeVec3(camera->v);

		camFieldElement = camElement->FirstChildElement("ImagePlane");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &camera->left, &camera->right, &camera->bottom, &camera->top,
			   &camera->near, &camera->far, &camera->horRes, &camera->verRes);

		camFieldElement = camElement->FirstChildElement("OutputName");
		str = camFieldElement->GetText();
		camera->outputFilename = string(str);

		this->cameras.push_back(camera);

		camElement = camElement->NextSiblingElement("Camera");
	}

	// read vertices
	xmlElement = rootNode->FirstChildElement("Vertices");
	XMLElement *vertexElement = xmlElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (vertexElement != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = vertexElement->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = vertexElement->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		this->vertices.push_back(vertex);
		this->colorsOfVertices.push_back(color);

		vertexElement = vertexElement->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	xmlElement = rootNode->FirstChildElement("Translations");
	XMLElement *translationElement = xmlElement->FirstChildElement("Translation");
	while (translationElement != NULL)
	{
		Translation *translation = new Translation();

		translationElement->QueryIntAttribute("id", &translation->translationId);

		str = translationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		this->translations.push_back(translation);

		translationElement = translationElement->NextSiblingElement("Translation");
	}

	// read scalings
	xmlElement = rootNode->FirstChildElement("Scalings");
	XMLElement *scalingElement = xmlElement->FirstChildElement("Scaling");
	while (scalingElement != NULL)
	{
		Scaling *scaling = new Scaling();

		scalingElement->QueryIntAttribute("id", &scaling->scalingId);
		str = scalingElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		this->scalings.push_back(scaling);

		scalingElement = scalingElement->NextSiblingElement("Scaling");
	}

	// read rotations
	xmlElement = rootNode->FirstChildElement("Rotations");
	XMLElement *rotationElement = xmlElement->FirstChildElement("Rotation");
	while (rotationElement != NULL)
	{
		Rotation *rotation = new Rotation();

		rotationElement->QueryIntAttribute("id", &rotation->rotationId);
		str = rotationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		this->rotations.push_back(rotation);

		rotationElement = rotationElement->NextSiblingElement("Rotation");
	}

	// read meshes
	xmlElement = rootNode->FirstChildElement("Meshes");

	XMLElement *meshElement = xmlElement->FirstChildElement("Mesh");
	while (meshElement != NULL)
	{
		Mesh *mesh = new Mesh();

		meshElement->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = meshElement->Attribute("type");

		if (strcmp(str, "wireframe") == 0)
		{
			mesh->type = WIREFRAME_MESH;
		}
		else
		{
			mesh->type = SOLID_MESH;
		}

		// read mesh transformations
		XMLElement *meshTransformationsElement = meshElement->FirstChildElement("Transformations");
		XMLElement *meshTransformationElement = meshTransformationsElement->FirstChildElement("Transformation");

		while (meshTransformationElement != NULL)
		{
			char transformationType;
			int transformationId;

			str = meshTransformationElement->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			meshTransformationElement = meshTransformationElement->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *cloneStr;
		int v1, v2, v3;
		XMLElement *meshFacesElement = meshElement->FirstChildElement("Faces");
		str = meshFacesElement->GetText();
		size_t length = strlen(str);
		cloneStr = new char[length+1];
		memcpy(cloneStr,str, length + 1);

		row = strtok(cloneStr, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);

			if (result != EOF)
			{
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		this->meshes.push_back(mesh);

		meshElement = meshElement->NextSiblingElement("Mesh");
	}
}

void Scene::assignColorToPixel(int i, int j, Color c)
{
	this->image[i][j].r = c.r;
	this->image[i][j].g = c.g;
	this->image[i][j].b = c.b;
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;
			vector<double> rowOfDepths;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
				rowOfDepths.push_back(1.01);
			}

			this->image.push_back(rowOfColors);
			this->depth.push_back(rowOfDepths);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				assignColorToPixel(i, j, this->backgroundColor);
				this->depth[i][j] = 1.01;
				this->depth[i][j] = 1.01;
				this->depth[i][j] = 1.01;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFilename.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFilename << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
*/
void Scene::convertPPMToPNG(string ppmFileName)
{
	string command;

	// TODO: Change implementation if necessary.
	command = "./magick convert " + ppmFileName + " " + ppmFileName + ".png";
	int res = system(command.c_str());
}

/*
	Transformations, clipping, culling, rasterization are done here.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{
	// TODO: Implement this function

	// Camera Transformation Matrix
	double cameraValues[4][4] = {{camera->u.x, camera->u.y, camera->u.z, -(camera->u.x*camera->position.x + camera->u.y*camera->position.y + camera->u.z*camera->position.z)},
								  {camera->v.x, camera->v.y, camera->v.z, -(camera->v.x*camera->position.x + camera->v.y*camera->position.y + camera->v.z*camera->position.z)},
								  {camera->w.x, camera->w.y, camera->w.z, -(camera->w.x*camera->position.x + camera->w.y*camera->position.y + camera->w.z*camera->position.z)},
								  {0, 0, 0, 1}};
	Matrix4 cameraTransformationMatrix(cameraValues);

	// Projection Transformation Matrix
	Matrix4 projectionTransformationMatrix;

	if (camera->projectionType == ORTOGRAPHIC_PROJECTION) {
		double projectionValues[4][4] = {
			{2 / (camera->right - camera->left), 0, 0, -(camera->right + camera->left) / (camera->right - camera->left)},
			{0, 2 / (camera->top - camera->bottom), 0, -(camera->top + camera->bottom) / (camera->top - camera->bottom)},
			{0, 0, 2 / (camera->near - camera->far), -(camera->near + camera->far) / (camera->near - camera->far)},
			{0, 0, 0, 1}
		};
		projectionTransformationMatrix = Matrix4(projectionValues);
	} else {
		double projectionValues[4][4] = {
			{2 * camera->near / (camera->right - camera->left), 0, (camera->right + camera->left) / (camera->right - camera->left), 0},
			{0, 2 * camera->near / (camera->top - camera->bottom), (camera->top + camera->bottom) / (camera->top - camera->bottom), 0},
			{0, 0, (camera->near + camera->far) / (camera->near - camera->far), 2 * camera->near * camera->far / (camera->near - camera->far)},
			{0, 0, -1, 0}
		};
		projectionTransformationMatrix = Matrix4(projectionValues);
	}

	// Viewport Transformation Matrix
	double viewportValues[4][4] = {
		{camera->horRes / 2, 0, 0, (camera->horRes - 1) / 2},
		{0, camera->verRes / 2, 0, (camera->verRes - 1) / 2},
		{0, 0, 0.5, 0.5},
		{0, 0, 0, 1}
	};

	Matrix4 viewportTransformationMatrix(viewportValues);

	for(Mesh* mesh : this->meshes){
		Matrix4 transformationMatrix = getIdentityMatrix();
		for(int i = 0; i < mesh->numberOfTransformations ; i++){
			// Translation
			if(mesh->transformationTypes[i] == 't'){
				Translation* translation = this->translations[mesh->transformationIds[i] - 1];
				double translationValues[4][4] = {
					{1, 0, 0, translation->tx},
					{0, 1, 0, translation->ty},
					{0, 0, 1, translation->tz},
					{0, 0, 0, 1}
				};
				Matrix4 translationMatrix(translationValues);

				// TODO: Multiply operation overload
				//transformationMatrix = transformationMatrix * translationMatrix;

				transformationMatrix = multiplyMatrixWithMatrix(transformationMatrix, translationMatrix);
			}

			// Scaling
			if(mesh->transformationTypes[i] == 's'){
				Scaling* scaling = this->scalings[mesh->transformationIds[i] - 1];
				double scalingValues[4][4] = {
					{scaling->sx, 0, 0, 0},
					{0, scaling->sy, 0, 0},
					{0, 0, scaling->sz, 0},
					{0, 0, 0, 1}
				};
				Matrix4 scalingMatrix(scalingValues);

				// TODO: Multiply operation overload
				//transformationMatrix = transformationMatrix * translationMatrix;

				transformationMatrix = multiplyMatrixWithMatrix(transformationMatrix, scalingMatrix);
			}

			// Rotation
			if(mesh->transformationTypes[i] == 'r'){
				Rotation* rotation = this->rotations[mesh->transformationIds[i] - 1];
				double rotationValues[4][4] = {
					{cos(rotation->angle) + pow(rotation->ux, 2) * (1 - cos(rotation->angle)), rotation->ux * rotation->uy * (1 - cos(rotation->angle)) - rotation->uz * sin(rotation->angle), rotation->ux * rotation->uz * (1 - cos(rotation->angle)) + rotation->uy * sin(rotation->angle), 0},
					{rotation->uy * rotation->ux * (1 - cos(rotation->angle)) + rotation->uz * sin(rotation->angle), cos(rotation->angle) + pow(rotation->uy, 2) * (1 - cos(rotation->angle)), rotation->uy * rotation->uz * (1 - cos(rotation->angle)) - rotation->ux * sin(rotation->angle), 0},
					{rotation->uz * rotation->ux * (1 - cos(rotation->angle)) - rotation->uy * sin(rotation->angle), rotation->uz * rotation->uy * (1 - cos(rotation->angle)) + rotation->ux * sin(rotation->angle), cos(rotation->angle) + pow(rotation->uz, 2) * (1 - cos(rotation->angle)), 0},
					{0, 0, 0, 1}
				};
				Matrix4 rotationMatrix(rotationValues);

				// TODO: Multiply operation overload
				//transformationMatrix = transformationMatrix * translationMatrix;

				transformationMatrix = multiplyMatrixWithMatrix(transformationMatrix, rotationMatrix);
			}
		
			for(Triangle triangle : mesh->triangles){
				Vec3* v1 = this->vertices[triangle.vertexIds[0] - 1];
				Vec3* v2 = this->vertices[triangle.vertexIds[1] - 1];
				Vec3* v3 = this->vertices[triangle.vertexIds[2] - 1];

				Vec4 v1Vec4(v1->x, v1->y, v1->z, 1);
				Vec4 v2Vec4(v2->x, v2->y, v2->z, 1);
				Vec4 v3Vec4(v3->x, v3->y, v3->z, 1);

				Vec4 v1Vec4Transformed = multiplyMatrixWithVec4(transformationMatrix, v1Vec4);
				Vec4 v2Vec4Transformed = multiplyMatrixWithVec4(transformationMatrix, v2Vec4);
				Vec4 v3Vec4Transformed = multiplyMatrixWithVec4(transformationMatrix, v3Vec4);

				Vec4 v1Vec4TransformedCamera = multiplyMatrixWithVec4(cameraTransformationMatrix, v1Vec4Transformed);
				Vec4 v2Vec4TransformedCamera = multiplyMatrixWithVec4(cameraTransformationMatrix, v2Vec4Transformed);
				Vec4 v3Vec4TransformedCamera = multiplyMatrixWithVec4(cameraTransformationMatrix, v3Vec4Transformed);

				Vec4 v1Vec4TransformedCameraProjection = multiplyMatrixWithVec4(projectionTransformationMatrix, v1Vec4TransformedCamera);
				Vec4 v2Vec4TransformedCameraProjection = multiplyMatrixWithVec4(projectionTransformationMatrix, v2Vec4TransformedCamera);
				Vec4 v3Vec4TransformedCameraProjection = multiplyMatrixWithVec4(projectionTransformationMatrix, v3Vec4TransformedCamera);

				// Perspective division
				if(camera->projectionType == PERSPECTIVE_PROJECTION){
					
					//TODO: Scalar divide operator overload
					//v1Vec4TransformedCameraProjection = v1Vec4TransformedCameraProjection / v1Vec4TransformedCameraProjection.t;
					//v2Vec4TransformedCameraProjection = v2Vec4TransformedCameraProjection / v2Vec4TransformedCameraProjection.t;
					//v3Vec4TransformedCameraProjection = v3Vec4TransformedCameraProjection / v3Vec4TransformedCameraProjection.t;
				}

				Vec4 v1Vec4TransformedCameraProjectionViewport = multiplyMatrixWithVec4(viewportTransformationMatrix, v1Vec4TransformedCameraProjection);
				Vec4 v2Vec4TransformedCameraProjectionViewport = multiplyMatrixWithVec4(viewportTransformationMatrix, v2Vec4TransformedCameraProjection);
				Vec4 v3Vec4TransformedCameraProjectionViewport = multiplyMatrixWithVec4(viewportTransformationMatrix, v3Vec4TransformedCameraProjection);

				// Convert to Vec3
				Vec3 v1Vec3TransformedCameraProjectionViewport(v1Vec4TransformedCameraProjectionViewport.x, v1Vec4TransformedCameraProjectionViewport.y, v1Vec4TransformedCameraProjectionViewport.z);
				Vec3 v2Vec3TransformedCameraProjectionViewport(v2Vec4TransformedCameraProjectionViewport.x, v2Vec4TransformedCameraProjectionViewport.y, v2Vec4TransformedCameraProjectionViewport.z);
				Vec3 v3Vec3TransformedCameraProjectionViewport(v3Vec4TransformedCameraProjectionViewport.x, v3Vec4TransformedCameraProjectionViewport.y, v3Vec4TransformedCameraProjectionViewport.z);

				//TODO: Color interpolation (wireframe and solid rendering)
				Color* v1Color = this->colorsOfVertices[triangle.vertexIds[0] - 1];
				Color* v2Color = this->colorsOfVertices[triangle.vertexIds[1] - 1];
				Color* v3Color = this->colorsOfVertices[triangle.vertexIds[2] - 1];

			}
		}
	}
}