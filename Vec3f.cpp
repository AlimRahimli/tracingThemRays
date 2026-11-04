#include "raytracer.hpp"
#include <iostream>
#include <cmath>


Vec3f normalize(Vec3f v){
    Vec3f vector;
    float howBig = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    
    if(howBig < 1e-7f){vector.x = 0; vector.y = 0; vector.z = 0; return vector;}
    vector.x = v.x/howBig; vector.y = v.y/howBig; vector.z = v.z/howBig;
    return vector;
    
}

Vec3f cross(Vec3f v, Vec3f u){
    Vec3f vector;
    vector.x = v.y * u.z - v.z * u.y; vector.y = v.z * u.x - v.x * u.z; vector.z = v.x * u.y - v.y * u.x;
    return vector;
}

float length(Vec3f v){
    float howBig = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return howBig;
}

Vec3f operator+(Vec3f v, Vec3f i){
    Vec3f vector;
    vector.x = v.x + i.x; vector.y = v.y + i.y; vector.z = v.z + i.z;
    return vector;
}

Vec3f operator-(Vec3f v, Vec3f i){
    Vec3f vector;
    vector.x = v.x - i.x; vector.y = v.y - i.y; vector.z = v.z - i.z;
    return vector;
}

float dot(Vec3f v, Vec3f u){
    return (v.x * u.x) + (v.y * u.y) + (v.z * u.z);
}

Vec3f componentWiseMult(Vec3f v, Vec3f u){
    Vec3f vector;
    vector.x = v.x * u.x; vector.y = v.y * u.y; vector.z = v.z * u.z;
    return vector;
}

Vec3f Vec3iToVec3f(Vec3i v){
    Vec3f vector;
    vector.x = v.r; vector.y = v.g; vector.z = v.b;
    return vector;
}