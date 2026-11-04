#include "scene_io.h"
#include "raytracer.hpp"


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