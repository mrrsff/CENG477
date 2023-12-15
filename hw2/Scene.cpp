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
#include "Clipping.h"

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
	c = c.round();
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
	// string command;

	// // TODO: Change implementation if necessary.
	// command = "./magick convert " + ppmFileName + " " + ppmFileName + ".png";
	// int res = system(command.c_str());
}

Matrix4 Scene::getModelingTransformationMatrix(Mesh* mesh)
{
	Matrix4 transformationMatrix = getIdentityMatrix();
	// Calculating transformation matrix
	for(int i = 0; i < mesh->numberOfTransformations ; i++){
		switch(mesh->transformationTypes[i])
		{
			case 't':
			{
				Translation* translation = this->translations[mesh->transformationIds[i] - 1];
				transformationMatrix = translation->getTranslationMatrix() * transformationMatrix;
				break;
			}
			case 's':
			{
				Scaling* scaling = this->scalings[mesh->transformationIds[i] - 1];
				transformationMatrix = scaling->getScalingMatrix() * transformationMatrix;
				break;
			}
			case 'r':
			{
				Rotation* rotation = this->rotations[mesh->transformationIds[i] - 1];
				transformationMatrix = rotation->getRotationMatrix() * transformationMatrix;
				break;
			}
			default:
				break;
		}
	}
	return transformationMatrix;
}

bool isBackfaceCulled(Vec4 & v0, Vec4 & v1, Vec4 & v2) {
    Vec3 v_0 = Vec3(v0.x, v0.y, v0.z, v0.colorId);
    Vec3 v_1 = Vec3(v1.x, v1.y, v1.z, v1.colorId);
    Vec3 v_2 = Vec3(v2.x, v2.y, v2.z, v2.colorId);
    Vec3 edge01 = subtractVec3(v_1, v_0);
    Vec3 edge02 = subtractVec3(v_2, v_0);
    Vec3 normalVector = normalizeVec3(crossProductVec3(edge01, edge02));
    double res = dotProductVec3(normalVector, v_0); // View Vector = v_0 - origin
    return !(res < 0);
}

void Scene::rasterizeLine(Line* line)
{
	int x0 = line->p0.x;
	int y0 = line->p0.y;
	int x1 = line->p1.x;
	int y1 = line->p1.y;
	Color c0 = line->p0Color;
	Color c1 = line->p1Color; 
	int dx = abs(x1 - x0); // abs(dx) >= abs(dy)
	int dy = abs(y1 - y0); // abs(dy) <= abs(dx)
	int sx = (x0 < x1) ? 1 : -1; // sign of x-step
	int sy = (y0 < y1) ? 1 : -1; // sign of y-step
	int err = dx - dy; // error value e_xy
	int x = x0;
	int y = y0;
	Color c = c0;
	while (true)
	{
		// Check for z-buffer
		if (this->depth[x][y] > line->p0.z)
		{
			this->depth[x][y] = line->p0.z;
			assignColorToPixel(x, y, c);
		}

		if (x == x1 && y == y1) break; // Reached the end
		
		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y += sy;
		}
		// Calculate color interpolation using y-coordinate
		double t = (double)(y - y0) / (double)(y1 - y0);
		c = c0 + (c1 - c0) * t;
	}
}

// Calculates barycentric coordinates of a triangle with cramers rule
void barycentricCoordinates(Vec4 p, Vec4 v1, Vec4 v2, Vec4 v3, double& alpha, double& beta, double& gamma){
	double detA = (v1.x - v3.x) * (v2.y - v3.y) - (v2.x - v3.x) * (v1.y - v3.y);
	double detA1 = (p.x - v3.x) * (v2.y - v3.y) - (v2.x - v3.x) * (p.y - v3.y);
	double detA2 = (v1.x - v3.x) * (p.y - v3.y) - (p.x - v3.x) * (v1.y - v3.y);
	alpha = detA1 / detA;
	beta = detA2 / detA;
	gamma = 1 - alpha - beta;
}
void Scene::rasterizeTriangle(Vec4* v1, Vec4* v2, Vec4* v3)
{
	// Set min and max values for x and y. Also check for boundaries.
	int xmin = max(min(v1->x, min(v2->x, v3->x)), 0.0);
	int xmax = min(max(v1->x, max(v2->x, v3->x)), (double)this->cameras[0]->horRes - 1);
	int ymin = max(min(v1->y, min(v2->y, v3->y)), 0.0);
	int ymax = min(max(v1->y, max(v2->y, v3->y)), (double)this->cameras[0]->verRes - 1);

	Color *c1 = this->colorsOfVertices[v1->colorId - 1];
	Color *c2 = this->colorsOfVertices[v2->colorId - 1];
	Color *c3 = this->colorsOfVertices[v3->colorId - 1];
	Color c;

	// Calculate barycentric coordinates
	double alpha, beta, gamma;
	for(int x = xmin; x <= xmax; x++){
		for(int y = ymin; y <= ymax; y++){
			Vec4 p = Vec4(x, y, 0, NO_COLOR);
			barycentricCoordinates(p, *v1, *v2, *v3, alpha, beta, gamma);
			if(alpha >= 0 && beta >= 0 && gamma >= 0){
				// Calculate linear interpolation with respect to barycentric coordinates
				double z = v1->z * alpha + v2->z * beta + v3->z * gamma;
				// Check for z-buffer
				if (this->depth[x][y] > z)
				{
					c = *c1 * alpha + *c2 * beta + *c3 * gamma;
					this->depth[x][y] = z;
					assignColorToPixel(x, y, c);
				}
			}
		}
	}
}

/*
	Transformations, clipping, cullent index fiing, rasterization are done here.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{
	Matrix4 cameraTransformationMatrix = camera->getCameraTransformationMatrix();
	Matrix4 projectionTransformationMatrix = camera->getProjectionTransformationMatrix();
	Matrix4 viewportTransformationMatrix = camera->getViewportTransformationMatrix();

	for(Mesh* mesh : this->meshes){
		/*
			For each triangle we need to do these steps:
			1. Apply modeling transformation matrix
			2. Apply camera transformation matrix
			3. Apply projection transformation matrix
			4. Clip
			5. Cull
			6. Apply viewport transformation matrix
			7. Rasterize
		*/
		Matrix4 modelingTransformationMatrix = getModelingTransformationMatrix(mesh);
		Matrix4 camera_modeling_transformationMatrix = multiplyMatrixWithMatrix(cameraTransformationMatrix, modelingTransformationMatrix);
		Matrix4 proj_camera_modeling_transformationMatrix = multiplyMatrixWithMatrix(projectionTransformationMatrix, camera_modeling_transformationMatrix);

		for(Triangle triangle : mesh->triangles){
			Vec3* v1 = this->vertices[triangle.vertexIds[0] - 1];
			Vec3* v2 = this->vertices[triangle.vertexIds[1] - 1];
			Vec3* v3 = this->vertices[triangle.vertexIds[2] - 1];

			Color* c1 = this->colorsOfVertices[triangle.vertexIds[0] - 1];
			Color* c2 = this->colorsOfVertices[triangle.vertexIds[1] - 1];
			Color* c3 = this->colorsOfVertices[triangle.vertexIds[2] - 1];

			Vec4 v1Vec4 = Vec4(v1->x, v1->y, v1->z, 1, v1->colorId);
			Vec4 v2Vec4 = Vec4(v2->x, v2->y, v2->z, 1, v2->colorId);
			Vec4 v3Vec4 = Vec4(v3->x, v3->y, v3->z, 1, v3->colorId);

			// Apply camera transformation matrix
			v1Vec4 = multiplyMatrixWithVec4(proj_camera_modeling_transformationMatrix, v1Vec4);
			v2Vec4 = multiplyMatrixWithVec4(proj_camera_modeling_transformationMatrix, v2Vec4);
			v3Vec4 = multiplyMatrixWithVec4(proj_camera_modeling_transformationMatrix, v3Vec4);

			if (this->cullingEnabled && isBackfaceCulled(v1Vec4, v2Vec4, v3Vec4))
			{
				continue;
			}

			// Perspective division
			v1Vec4 = v1Vec4 / v1Vec4.t;
			v2Vec4 = v2Vec4 / v2Vec4.t;
			v3Vec4 = v3Vec4 / v3Vec4.t;
			
			Vec3 v1Vec3 = Vec3(v1Vec4.x, v1Vec4.y, v1Vec4.z, v1Vec4.colorId);
			Vec3 v2Vec3 = Vec3(v2Vec4.x, v2Vec4.y, v2Vec4.z, v2Vec4.colorId);
			Vec3 v3Vec3 = Vec3(v3Vec4.x, v3Vec4.y, v3Vec4.z, v3Vec4.colorId);
			

			if (mesh->type == WIREFRAME_MESH)
			{
				// Wireframe rendering mode, use line clipping and draw lines
				// Generate lines for each triangle
				Line lines[3] = {
					Line(v1Vec3, v2Vec3, *c1, *c2),
					Line(v2Vec3, v3Vec3, *c2, *c3),
					Line(v3Vec3, v1Vec3, *c3, *c1)
				};

				// Clip and draw each line
				for (int i = 0; i < 3; ++i) {
					if (liangBarsky(lines[i])) {
						// Apply viewport transformation to the line coordinates before drawing
						lines[i].applyTransformationMatrix(viewportTransformationMatrix);
						rasterizeLine(&lines[i]);
					}
					else cout << lines[i] << endl;
				}
			}
			else
			{
				// Solid rendering mode, just draw triangles
				// Apply viewport transformation to the triangle coordinates before drawing
				v1Vec4 = multiplyMatrixWithVec4(viewportTransformationMatrix, v1Vec4);
				v2Vec4 = multiplyMatrixWithVec4(viewportTransformationMatrix, v2Vec4);
				v3Vec4 = multiplyMatrixWithVec4(viewportTransformationMatrix, v3Vec4);

				v1Vec4 = v1Vec4 / v1Vec4.t;
				v2Vec4 = v2Vec4 / v2Vec4.t;
				v3Vec4 = v3Vec4 / v3Vec4.t;
				
				v1Vec4.colorId = v1->colorId;
				v2Vec4.colorId = v2->colorId;
				v3Vec4.colorId = v3->colorId;

				rasterizeTriangle(&v1Vec4, &v2Vec4, &v3Vec4);
			}
		}
	}
}