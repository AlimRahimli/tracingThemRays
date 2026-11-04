#include "scene_io.h"

struct Ray
{
    Vec3f origin;
    Vec3f direction;

    public: Ray(Vec3f, Vec3f);
};

Vec3f normalize(Vec3f v);

Vec3f cross(Vec3f v, Vec3f u);

Vec3f operator*(int i, Vec3f v);

Vec3f operator*(float i, Vec3f v);

Vec3f operator+(Vec3f v, Vec3f i);

Vec3f operator-(Vec3f v, Vec3f i);