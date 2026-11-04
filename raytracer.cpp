#include "raytracer.hpp"
#include "scene_io.h"

#include <iostream>
#include <cmath>
#include <algorithm>

struct CameraBasis {
    
    Vec3f u, v, w, position; //right, up, opposite gaze, position

    public: CameraBasis(const Camera& camera)
    {
        v = normalize(camera.up);
        w = -1 * normalize(camera.gaze);
        u = normalize(cross(v, w));
        v = cross(w, u); // this and line above enforce that basis vectors are orthagonal.
        position = camera.position;
    } 

};

struct ImagePlane {

    float pixheight, pixwidth; //pixel height, pixel width

    Vec3f m, q; //center of near plane, top left corner point

    public: ImagePlane(const Camera& camera, const CameraBasis& cameraBasis)
    {
        pixheight = (camera.nearPlaneTop - camera.nearPlaneBottom) / camera.imageHeight;
        pixwidth = (camera.nearPlaneRight - camera.nearPlaneLeft) / camera.imageWidth;

        m = cameraBasis.position + (camera.nearDistance * -1.0f * cameraBasis.w);
        q = m + (camera.nearPlaneLeft * cameraBasis.u) + (camera.nearPlaneTop * cameraBasis.v);
    }
};

Vec3f computeColor(const Scene& scene, const Ray& ray, int depth);
Vec3i Clamp(Vec3f xyz);

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Please specify only one (1) scene file." << std::endl;
        return 0;
    }

    Scene scene;
    //int camNo = 1;
    loadSceneFromJson(argv[1], scene);

    for(const Camera& cam : scene.cameras)
    {
        std::vector<Vec3i> pixelmap;
        pixelmap.reserve(cam.imageWidth * cam.imageHeight);
        //std::string filename = argv[1] + "Cam" + std::to_string(camNo++);

        CameraBasis camBasis(cam);
        ImagePlane imgPlane(cam, camBasis);

        for(int j = 0; j < cam.imageHeight; j++)
        {
            for(int i = 0; i < cam.imageWidth; i++)
            {
                Vec3f targetPoint = 
                    imgPlane.q
                    + (((i + 0.5f) * imgPlane.pixwidth) * camBasis.u)
                    - (((j + 0.5f) * imgPlane.pixheight) * camBasis.v);
                
                Ray ray(cam.position, normalize(targetPoint - cam.position));
                Vec3i pixel = Clamp(computeColor(scene, ray, 0));
                pixelmap.push_back(pixel);
            }
        }

        writePPM(cam.imageName, cam.imageWidth, cam.imageHeight, pixelmap);
    }
}

Vec3f computeColor(const Scene& scene, const Ray& ray, int depth)
{
    HitRecord hit;

    if(!intersectScene(scene, ray, hit, false))
    {
        if(depth == 0)
            return Vec3iToVec3f(scene.backgroundColor);
        
        Vec3f zero;
        return zero;
    }

    Material material = scene.materials[hit.materialID];

    Vec3f ambientApplied = componentWiseMult(material.ambientReflectance, scene.ambientLight);

    Vec3f diffuseApplied;
    Vec3f specularApplied;

    for(const PointLight& light : scene.pointLights)
    {
        Vec3f lightUnnormal = light.position - hit.point;
        float distance = length(lightUnnormal);
        Vec3f lightNormal = normalize(lightUnnormal);

        HitRecord shadowHit;
        Ray shadowRay(hit.point + scene.shadowRayEpsilon * hit.normal, lightNormal);
        
        Vec3f view = -1 * ray.direction;

        if(!intersectScene(scene, shadowRay, shadowHit, true) || shadowHit.t >= distance)
        {
            Vec3f irradiance = (1/(distance * distance)) * light.intensity;
            Vec3f compDiffuse = componentWiseMult(material.diffuseReflectance, irradiance);
            
            Vec3f diffuse = 
            std::max(0.0f, (dot(hit.normal, lightNormal))) * compDiffuse;
            
            diffuseApplied = diffuseApplied + diffuse;
            
            Vec3f halfwayVector = normalize(view + lightNormal);
            
            Vec3f compSpecular = componentWiseMult(material.specularReflectance, irradiance);
            Vec3f specular = pow(std::max(0.0f, dot(hit.normal, halfwayVector)), material.phongExponent) * compSpecular;
            specularApplied = specularApplied + specular;
        }
    }

    if (material.isMirror && depth < scene.maxRecursionDepth)
        return ambientApplied + diffuseApplied + specularApplied;
}

Vec3i Clamp(Vec3f xyz)
{
    Vec3i rgb;

    rgb.r = static_cast<int>(std::round(std::min(255.0f, std::max(0.0f, xyz.x))));
    rgb.g = static_cast<int>(std::round(std::min(255.0f, std::max(0.0f, xyz.y))));
    rgb.b = static_cast<int>(std::round(std::min(255.0f, std::max(0.0f, xyz.z))));

    return rgb;
}