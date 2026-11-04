 #ifndef SCENE_IO_H

#define SCENE_IO_H


#include <string>

#include <vector>


// Pure data containers only. 

// Implement your own vector math (dot, cross, normalize, operators) 

// separately to comply with the course AI policy.

struct Vec3f {

    float x = 0.0f, y = 0.0f, z = 0.0f;

};


struct Vec3i {

    int r = 0, g = 0, b = 0;

};


struct Camera {

    int id = 0;

    Vec3f position;

    Vec3f gaze;

    Vec3f up;

    float nearPlaneLeft = 0.0f;

    float nearPlaneRight = 0.0f;

    float nearPlaneBottom = 0.0f;

    float nearPlaneTop = 0.0f;

    float nearDistance = 0.0f;

    int imageWidth = 0;

    int imageHeight = 0;

    std::string imageName;

};


struct PointLight {

    int id = 0;

    Vec3f position;

    Vec3f intensity;

};


struct Material {

    int id = 0;

    bool isMirror = false;

    Vec3f ambientReflectance;

    Vec3f diffuseReflectance;

    Vec3f specularReflectance;

    Vec3f mirrorReflectance;

    float phongExponent = 1.0f;

};


struct Triangle {

    int materialId = 0; // 0-based index into Scene::materials

    int v0 = 0, v1 = 0, v2 = 0; // 0-based indices into Scene::vertices

    Vec3f normal;
};


struct Mesh {

    int materialId = 0; // 0-based index into Scene::materials

    std::vector<Triangle> faces;

};


struct Sphere {

    int materialId = 0; // 0-based index into Scene::materials

    int centerVertexId = 0; // 0-based index into Scene::vertices

    float radius = 0.0f;

};


struct Plane {

    int materialId = 0; // 0-based index into Scene::materials

    int centerVertexId = 0; // 0-based index into Scene::vertices

    Vec3f normal; // Remember to normalize this in your code!

};


struct Cylinder {

    int materialId = 0; // 0-based index into Scene::materials

    int centerVertexId = 0; // 0-based index into Scene::vertices

    Vec3f axis; // Remember to normalize this in your code!

    float radius = 0.0f;

    float height = 0.0f;

};


struct Scene {

    Vec3i backgroundColor;

    float shadowRayEpsilon = 1e-3f;

    int maxRecursionDepth = 0;


    std::vector<Camera> cameras;

    Vec3f ambientLight;

    std::vector<PointLight> pointLights;

    std::vector<Material> materials;

    std::vector<Vec3f> vertices;


    std::vector<Mesh> meshes;

    std::vector<Triangle> triangles;

    std::vector<Sphere> spheres;

    std::vector<Plane> planes;

    std::vector<Cylinder> cylinders;

};


// Parses a JSON scene file into the Scene struct.

bool loadSceneFromJson(const std::string& filepath, Scene& scene);


// Writes an image buffer of clamped [0, 255] RGB integers to an ASCII PPM (P3) file.

bool writePPM(const std::string& filename, int width, int height, const std::vector<Vec3i>& pixels);


#endif // SCENE_IO_H 