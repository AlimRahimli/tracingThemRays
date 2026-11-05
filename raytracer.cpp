#include "raytracer.hpp"
#include "scene_io.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <pthread.h>

struct CameraBasis {
    
    Vec3f u, v, w, position; //right, up, opposite gaze, position
    
    public: CameraBasis(const Camera& camera)
    {
        v = normalize(camera.up);
        w = -1.0f * normalize(camera.gaze);
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

struct ThreadData {
    const Camera* cam;
    const ImagePlane* imgPlane;
    const CameraBasis* camBasis;
    const Scene* scene;
    std::vector<Vec3i>* pixelmap;
    int threadID;
    int numThreads;
};

Vec3f computeColor(const Scene& scene, const Ray& ray, int depth);
Vec3i Clamp(Vec3f xyz);
void* render(void* args);

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
    preprocessScene(scene);
    
    for(const Camera& cam : scene.cameras)
    {
        std::vector<Vec3i> pixelmap;
        pixelmap.resize(cam.imageWidth * cam.imageHeight);
        //std::string filename = argv[1] + "Cam" + std::to_string(camNo++);
        
        CameraBasis camBasis(cam);
        ImagePlane imgPlane(cam, camBasis);
        
        pthread_t threads[16];
        ThreadData threadDataArr[16];

        for(int t = 0; t < 16; t++)
        {
            ThreadData threadData = {
                &cam,
                &imgPlane,
                &camBasis,
                &scene,
                &pixelmap,
                t,
                16
            };

            threadDataArr[t] = threadData;

            pthread_create(&threads[t], nullptr, &render, &threadDataArr[t]);
        }

        for(int t = 0; t < 16; t++)
        {
            pthread_join(threads[t], nullptr);
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
    
    Vec3f view = -1.0f * ray.direction;

    for(const PointLight& light : scene.pointLights)
    {
        Vec3f lightUnnormal = light.position - hit.point;
        float distance = length(lightUnnormal);
        Vec3f lightNormal = normalize(lightUnnormal);
        
        HitRecord shadowHit;
        shadowHit.time = distance;
        Ray shadowRay(hit.point + scene.shadowRayEpsilon * hit.normal, lightNormal);
        
        if(!intersectScene(scene, shadowRay, shadowHit, true) || shadowHit.time >= distance)
        {
            Vec3f irradiance = (1/(distance * distance)) * light.intensity;
            Vec3f compDiffuse = componentWiseMult(material.diffuseReflectance, irradiance);
            
            Vec3f diffuse = 
            std::max(0.0f, (dot(hit.normal, lightNormal))) * compDiffuse;
            
            diffuseApplied = diffuseApplied + diffuse;
            
            Vec3f halfwayVector = normalize(view + lightNormal);
            float nDotL = dot(hit.normal, lightNormal);
            
            if(nDotL > 0.0f)
            {
                Vec3f compSpecular = componentWiseMult(material.specularReflectance, irradiance);
                Vec3f specular = pow(std::max(0.0f, dot(hit.normal, halfwayVector)), material.phongExponent) * compSpecular;
                specularApplied = specularApplied + specular;
            }
        }
    }
    
    Vec3f color = ambientApplied + diffuseApplied + specularApplied;
    
    if (material.isMirror && depth < scene.maxRecursionDepth)
    {
        Vec3f reflectionDir = normalize(ray.direction - 2.0f * (dot(ray.direction, hit.normal) * hit.normal));
        Ray reflectionRay(hit.point + scene.shadowRayEpsilon * hit.normal, reflectionDir);
        Vec3f mirror = componentWiseMult(computeColor(scene, reflectionRay, depth + 1), material.mirrorReflectance);
        color = color + mirror;
    }
    
    return color;
}

Vec3i Clamp(Vec3f xyz)
{
    Vec3i rgb;
    
    rgb.r = static_cast<int>(std::round(std::min(255.0f, std::max(0.0f, xyz.x))));
    rgb.g = static_cast<int>(std::round(std::min(255.0f, std::max(0.0f, xyz.y))));
    rgb.b = static_cast<int>(std::round(std::min(255.0f, std::max(0.0f, xyz.z))));
    
    return rgb;
}

void* render(void* args)
{
    ThreadData* threadData = static_cast<ThreadData*>(args);

    for(int j = threadData->threadID; j < threadData->cam->imageHeight; j += threadData->numThreads)
    {
        for(int i = 0; i < threadData->cam->imageWidth; i++)
        {
            Vec3f targetPoint = 
            threadData->imgPlane->q
            + (((i + 0.5f) * threadData->imgPlane->pixwidth) * threadData->camBasis->u)
            - (((j + 0.5f) * threadData->imgPlane->pixheight) * threadData->camBasis->v);
            
            Ray ray(threadData->cam->position, normalize(targetPoint - threadData->cam->position));
            Vec3i pixel = Clamp(computeColor((*threadData->scene), ray, 0));
            (*threadData->pixelmap)[j * threadData->cam->imageWidth + i] = (pixel);
        }
    }

    return nullptr;
}