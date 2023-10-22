#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <string>
#include <vector>
#include <cmath>

namespace parser
{
    //Notice that all the structures are as simple as possible
    //so that you are not enforced to adopt any style or design.
    struct Vec3f
    {
        float x, y, z;
        Vec3f operator*(float scalar)
        {
            return Vec3f{this->x * scalar, this->y * scalar, this->z * scalar};
        };
        Vec3f operator*(int scalar)
        {
            return Vec3f{this->x * scalar, this->y * scalar, this->z * scalar};
        };
        Vec3f operator+ (Vec3f other)
        {
            return Vec3f{this->x + other.x, this->y + other.y, this->z + other.z};
        };
        Vec3f operator- (Vec3f other)
        {
            return Vec3f{this->x - other.x, this->y - other.y, this->z - other.z};
        };
        float length()
        {
            return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
        };
        Vec3f normalize()
        {
            float length = this->length();
            return Vec3f{this->x / length, this->y / length, this->z / length};
        };
        float dot(Vec3f other)
        {
            return this->x * other.x + this->y * other.y + this->z * other.z;
        };
        Vec3f cross(Vec3f other)
        {
            return Vec3f{this->y * other.z - this->z * other.y, this->z * other.x - this->x * other.z, this->x * other.y - this->y * other.x};
        };
        float operator[](int index)
        {
            if (index == 0)
                return this->x;
            else if (index == 1)
                return this->y;
            else if (index == 2)
                return this->z;
            else
                return 0;
        };
        bool operator==(Vec3f other) // 
        {
            return (this->x - other.x < 0.0001 && this->y - other.y < 0.0001 && this->z - other.z < 0.0001);
        };
        std::string toString()
        {
            return std::to_string(this->x) + " " + std::to_string(this->y) + " " + std::to_string(this->z);
        };
    };

    struct Vec3i
    {
        int x, y, z;
    };

    struct Vec4f
    {
        float x, y, z, w;
    };

    struct Camera
    {
        Vec3f position;
        Vec3f gaze;
        Vec3f up;
        Vec4f near_plane;
        float near_distance;
        int image_width, image_height;
        std::string image_name;
    };

    struct PointLight
    {
        Vec3f position;
        Vec3f intensity;
    };

    struct Material
    {
        bool is_mirror;
        Vec3f ambient;
        Vec3f diffuse;
        Vec3f specular;
        Vec3f mirror;
        float phong_exponent;
    };


    struct Face
    {
        int v0_id;
        int v1_id;
        int v2_id;
    };
    struct Mesh
    {
        int material_id;
        std::vector<Face> faces;
    };

    struct Triangle
    {
        int material_id;
        Face indices;
    };

    struct Sphere
    {
        int material_id;
        int center_vertex_id;
        float radius;
    };

    struct Scene
    {
        //Data
        Vec3i background_color;
        float shadow_ray_epsilon;
        int max_recursion_depth;
        std::vector<Camera> cameras;
        Vec3f ambient_light;
        std::vector<PointLight> point_lights;
        std::vector<Material> materials;
        std::vector<Vec3f> vertex_data;
        std::vector<Mesh> meshes;
        std::vector<Triangle> triangles;
        std::vector<Sphere> spheres;

        //Functions
        void loadFromXml(const std::string &filepath);
    };
}

#endif
