#include <iomanip>
#include "Camera.h"
#include "Helpers.h"
#include "Matrix4.h"
#include <cmath>

Camera::Camera() {}

Camera::Camera(int cameraId,
               int projectionType,
               Vec3 position, Vec3 gaze,
               Vec3 u, Vec3 v, Vec3 w,
               double left, double right, double bottom, double top,
               double near, double far,
               int horRes, int verRes,
               std::string outputFilename)
{

    this->cameraId = cameraId;
    this->projectionType = projectionType;
    this->position = position;
    this->gaze = gaze;
    this->u = u;
    this->v = v;
    this->w = w;
    this->left = left;
    this->right = right;
    this->bottom = bottom;
    this->top = top;
    this->near = near;
    this->far = far;
    this->horRes = horRes;
    this->verRes = verRes;
    this->outputFilename = outputFilename;
}

Camera::Camera(const Camera &other)
{
    this->cameraId = other.cameraId;
    this->projectionType = other.projectionType;
    this->position = other.position;
    this->gaze = other.gaze;
    this->u = other.u;
    this->v = other.v;
    this->w = other.w;
    this->left = other.left;
    this->right = other.right;
    this->bottom = other.bottom;
    this->top = other.top;
    this->near = other.near;
    this->far = other.far;
    this->horRes = other.horRes;
    this->verRes = other.verRes;
    this->outputFilename = other.outputFilename;
}

std::ostream &operator<<(std::ostream &os, const Camera &c)
{
    const char *camType = c.projectionType ? "perspective" : "orthographic";

    os << std::fixed << std::setprecision(6) << "Camera " << c.cameraId << " (" << camType << ") => pos: " << c.position << " gaze: " << c.gaze << std::endl
       << "\tu: " << c.u << " v: " << c.v << " w: " << c.w << std::endl
       << std::fixed << std::setprecision(3) << "\tleft: " << c.left << " right: " << c.right << " bottom: " << c.bottom << " top: " << c.top << std::endl
       << "\tnear: " << c.near << " far: " << c.far << " resolutions: " << c.horRes << "x" << c.verRes << " fileName: " << c.outputFilename;
    return os;
}

Matrix4 Camera::getCameraTransformationMatrix()
{
    double cameraValues[4][4] = {{this->u.x, this->u.y, this->u.z, -(this->u.x*this->position.x + this->u.y*this->position.y + this->u.z*this->position.z)},
								  {this->v.x, this->v.y, this->v.z, -(this->v.x*this->position.x + this->v.y*this->position.y + this->v.z*this->position.z)},
								  {this->w.x, this->w.y, this->w.z, -(this->w.x*this->position.x + this->w.y*this->position.y + this->w.z*this->position.z)},
								  {0, 0, 0, 1}};
	return Matrix4(cameraValues);
}

Matrix4 Camera::getProjectionTransformationMatrix()
{
	if (this->projectionType == ORTOGRAPHIC_PROJECTION) {
		double projectionValues[4][4] = {
			{2 / (this->right - this->left), 0, 0, -(this->right + this->left) / (this->right - this->left)},
			{0, 2 / (this->top - this->bottom), 0, -(this->top + this->bottom) / (this->top - this->bottom)},
			{0, 0, -2 / (this->far - this->near), -(this->far + this->near) / (this->far - this->near)},
			{0, 0, 0, 1}
		};
        return Matrix4(projectionValues);
	} else {
		double projectionValues[4][4] = {
			{2 * this->near / (this->right - this->left), 0, (this->right + this->left) / (this->right - this->left), 0},
			{0, 2 * this->near / (this->top - this->bottom), (this->top + this->bottom) / (this->top - this->bottom), 0},
			{0, 0, -(this->far + this->near) / (this->far - this->near), -2 * this->far * this->near / (this->far - this->near)},
			{0, 0, -1, 0}
		};
		return Matrix4(projectionValues);
	}
}

Matrix4 Camera::getViewportTransformationMatrix()
{
    double viewportValues[4][4] = {
		{(double) this->horRes / 2, 0, 0, ((double) this->horRes - 1) / 2},
		{0, (double) this->verRes / 2, 0, ((double) this->verRes - 1) / 2},
		{0, 0, 0.5, 0.5},
		{0, 0, 0, 1}
	};
    return Matrix4(viewportValues);
}