#include "raytracer.hpp"
#include "intersect.hpp"
#include "scene_io.h"

#include <iostream>

struct CameraBasis {
    
    Vec3f u, v, w, position; //right, up, opposite gaze, position

    public: CameraBasis(const Camera& camera)
    {
        v = normalize(camera.up);
        w = -1 * normalize(camera.gaze);
        u = normalize(cross(v, w));
        v = cross(u, w); // this and line above enforce that basis vectors are orthagonal.
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

        m = cameraBasis.position + (camera.nearDistance * normalize(camera.gaze));
        q = m + (camera.nearPlaneLeft * cameraBasis.u) + (camera.nearPlaneTop * cameraBasis.v);
    }
};

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Please specify only one (1) scene file." << std::endl;
        return 0;
    }

    Scene scene;
    loadSceneFromJson(argv[1], scene);

    for(Camera cam : scene.cameras)
    {
        CameraBasis camBasis(cam);
        ImagePlane imgPlane(cam, camBasis);

        for(int j = 0; j < camera.imageHeight; j++)
        {
            for(int i = 0; i < camera.imageWidth; i++)
            {
                Vec3f targetPoint = 
                    imagePlane.q
                    + (((i + 0.5f) * imagePlane.pixwidth) * cameraBasis.u)
                    - (((j + 0.5f) * imagePlane.pixheight) * cameraBasis.v);
                
                Ray ray(cam.position, normalize(targetPoint - cam.position));
            }
        }
    }
}
