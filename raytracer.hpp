#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP

#include "scene_io.h"

struct Ray
{
    Vec3f origin;
    Vec3f direction;

    public: Ray(Vec3f, Vec3f);
};

struct hitRecord
{
    float time = 1e30f;
    Vec3f point;
    Vec3f normal;
    int materialId = -1;
    
};

Vec3f normalize(Vec3f v);

Vec3f cross(Vec3f v, Vec3f u);

Vec3f operator*(int i, Vec3f v);

Vec3f operator*(float i, Vec3f v);

Vec3f operator+(Vec3f v, Vec3f i);

Vec3f operator-(Vec3f v, Vec3f i);

float dot(Vec3f v, Vec3f u);

float length(Vec3f v);

Vec3f componentWiseMult(Vec3f v, Vec3f u);

Vec3f Vec3iToVec3f(Vec3i v); //Cast Vec3i into Vec3f

bool intersectScene(const Scene& scene, const Ray& ray, HitRecord& hit, bool isShadowRay);

#endif