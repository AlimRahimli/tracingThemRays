#include "scene_io.h"
#include "raytracer.hpp"

#include <cmath>
#include <iostream>

void preprocessScene(Scene& scene){
    for (Cylinder& cylinder : scene.cylinders){
        cylinder.axis = normalize(cylinder.axis);
    }
    for(Plane& plane : scene.planes){
        plane.normal = normalize(plane.normal);
    }

    for(Triangle& triangle : scene.triangles){
        Vec3f v0 = scene.vertices[triangle.v0];
        Vec3f v1 = scene.vertices[triangle.v1];
        Vec3f v2 = scene.vertices[triangle.v2];
        triangle.normal = normalize(cross((v1-v0), (v2-v0)));
    }

    for(Mesh& mesh : scene.meshes){
        for(Triangle& triangle : mesh.faces){
            Vec3f v0 = scene.vertices[triangle.v0];
            Vec3f v1 = scene.vertices[triangle.v1];
            Vec3f v2 = scene.vertices[triangle.v2];
            triangle.normal = normalize(cross((v1-v0), (v2-v0)));
        }
    }
}

bool intersectPlane(const Ray& ray, const Plane& plane, const std::vector<Vec3f>& vertices, HitRecord& hitRecord){
    Vec3f center = vertices[plane.centerVertexId];
    float dirSimilarity = dot(ray.direction, plane.normal);
    if(std::fabs(dirSimilarity) < 1e-7f){
        return false;
    }

    float t = (dot((center - ray.origin), plane.normal))/dirSimilarity;
    if((t > 0) && (t < hitRecord.time)){
        hitRecord.time = t;
        hitRecord.materialID = plane.materialId;
        hitRecord.normal = plane.normal;
        hitRecord.point = ray.origin + ray.direction * hitRecord.time;
        return true;
    }
    return false;
}

bool intersectSphere(const Ray& ray, const Sphere& sphere, const std::vector<Vec3f>& vertices, HitRecord& hitRecord){
    Vec3f center = vertices[sphere.centerVertexId];
    Vec3f D = ray.direction;
    Vec3f O = ray.origin;
    Vec3f OminC = O - center;
    float r = sphere.radius;

    float A1 = dot(D, D);
    float A2 = 2.0f * dot(D, OminC);
    float A3 = dot(OminC, OminC) - r*r;
    float t = 0.0f;

    float discriminant = A2*A2 - 4*A1*A3;
    if(discriminant < 0){
        //imaginary intersection? :/
        return false;   
    }

    t = (-A2 - std::sqrt(discriminant))/(2*A1);
    if((t > 0) && (t < hitRecord.time)){
        Vec3f Pt = O + D*t;

        hitRecord.time = t;
        hitRecord.normal =  (Pt - center)*(1.0f / r);
        hitRecord.materialID = sphere.materialId;
        hitRecord.point = Pt;
        return true;
    }
    return false;
}