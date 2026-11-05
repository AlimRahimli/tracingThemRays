#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP

#include "scene_io.h"

struct Ray
{
    Vec3f origin;
    Vec3f direction;

    public: Ray(Vec3f, Vec3f);
};

struct HitRecord
{

    float time = 1e30f; //hit distance/time
    Vec3f point; //intersection coordinate
    Vec3f normal; //surface unit normal at intersection point
    int materialID = -1;

};

Vec3f normalize(Vec3f v);

Vec3f cross(Vec3f v, Vec3f u);

//I removed int overload because cpp automatically converts ints to floats anyways

inline Vec3f operator*(float f, Vec3f v){
    Vec3f vector;
    vector.x = v.x*f; vector.y = v.y*f; vector.z = v.z*f;
    return vector;
}

inline Vec3f operator*(Vec3f v, float f) {
    return f * v;
}

inline Vec3f operator*(Vec3f v, Vec3f u) {
    Vec3f vector;
    vector.x = v.x * u.x; vector.y = v.y * u.y; vector.z = v.z * u.z;
    return vector;
}


//need to implement operator+/-(vec3f, int/float)?
Vec3f operator+(Vec3f v, Vec3f i);

Vec3f operator-(Vec3f v, Vec3f i);

float dot(Vec3f v, Vec3f u);

float length(Vec3f v);


//I overloaded operator*() as well
//so this function is unnecessary now, I am delegating the decision to remove it to you
Vec3f componentWiseMult(Vec3f v, Vec3f u);

Vec3f Vec3iToVec3f(Vec3i v); //Cast Vec3i into Vec3f

bool intersectScene(const Scene& scene, const Ray& ray, HitRecord& hit, bool isShadowRay);



void preprocessScene(Scene& scene);

bool intersectPlane(const Ray& ray, const Plane& plane, const std::vector<Vec3f>& vertices, HitRecord& hitRecord);

bool intersectSphere(const Ray& ray, const Sphere& sphere, const std::vector<Vec3f>& vertices, HitRecord& hitRecord);

float findDet(const float& a, const float& b, const float& c, const float& d, const float& e,
            const float& f, const float& g, const float& h, const float& i);

bool intersectTriangle(const Ray& ray, const Triangle& triangle, const std::vector<Vec3f>& vertices, const bool& isShadowRay, HitRecord& hitRecord);
#endif