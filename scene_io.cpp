#include "scene_io.h"
#include "json.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

namespace {

// Helper: Normalizes a JSON node so a single object is treated as a 1-element array
json asArray(const json& node) {
    if (node.is_null()) return json::array();
    if (node.is_array()) return node;
    return json::array({node});
}

// Helpers for parsing whitespace-separated numbers inside JSON strings
Vec3f parseVec3f(const std::string& str) {
    std::stringstream ss(str);
    Vec3f v;
    ss >> v.x >> v.y >> v.z;
    return v;
}

Vec3i parseVec3i(const std::string& str) {
    std::stringstream ss(str);
    Vec3i v;
    ss >> v.r >> v.g >> v.b;
    return v;
}

float parseFloat(const std::string& str) {
    return std::stof(str);
}

int parseInt(const std::string& str) {
    return std::stoi(str);
}

// Helper to fetch "_data" or "data" safely
std::string getDataString(const json& node) {
    if (node.contains("_data")) return node["_data"].get<std::string>();
    if (node.contains("data")) return node["data"].get<std::string>();
    return "";
}

} // anonymous namespace

bool loadSceneFromJson(const std::string& filepath, Scene& scene) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open scene file: " << filepath << "\n";
        return false;
    }

    json root;
    try {
        file >> root;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << "\n";
        return false;
    }

    if (!root.contains("Scene")) {
        std::cerr << "Error: Missing root 'Scene' object in JSON.\n";
        return false;
    }

    const json& s = root["Scene"];

    // 1. Global Scene Settings
    if (s.contains("BackgroundColor")) {
        scene.backgroundColor = parseVec3i(s["BackgroundColor"].get<std::string>());
    }
    if (s.contains("ShadowRayEpsilon")) {
        scene.shadowRayEpsilon = parseFloat(s["ShadowRayEpsilon"].get<std::string>());
    }
    if (s.contains("MaxRecursionDepth")) {
        scene.maxRecursionDepth = parseInt(s["MaxRecursionDepth"].get<std::string>());
    }

    // 2. Cameras
    if (s.contains("Cameras") && s["Cameras"].contains("Camera")) {
        for (const auto& camNode : asArray(s["Cameras"]["Camera"])) {
            Camera cam;
            if (camNode.contains("_id")) cam.id = parseInt(camNode["_id"].get<std::string>());
            cam.position = parseVec3f(camNode["Position"].get<std::string>());
            cam.gaze = parseVec3f(camNode["Gaze"].get<std::string>());
            cam.up = parseVec3f(camNode["Up"].get<std::string>());

            std::stringstream np(camNode["NearPlane"].get<std::string>());
            np >> cam.nearPlaneLeft >> cam.nearPlaneRight >> cam.nearPlaneBottom >> cam.nearPlaneTop;

            cam.nearDistance = parseFloat(camNode["NearDistance"].get<std::string>());

            std::stringstream res(camNode["ImageResolution"].get<std::string>());
            res >> cam.imageWidth >> cam.imageHeight;

            cam.imageName = camNode["ImageName"].get<std::string>();
            scene.cameras.push_back(cam);
        }
    }

    // 3. Lights
    if (s.contains("Lights")) {
        const json& lights = s["Lights"];
        if (lights.contains("AmbientLight")) {
            scene.ambientLight = parseVec3f(lights["AmbientLight"].get<std::string>());
        }
        if (lights.contains("PointLight")) {
            for (const auto& plNode : asArray(lights["PointLight"])) {
                PointLight pl;
                if (plNode.contains("_id")) pl.id = parseInt(plNode["_id"].get<std::string>());
                pl.position = parseVec3f(plNode["Position"].get<std::string>());
                pl.intensity = parseVec3f(plNode["Intensity"].get<std::string>());
                scene.pointLights.push_back(pl);
            }
        }
    }

    // 4. Materials
    if (s.contains("Materials") && s["Materials"].contains("Material")) {
        for (const auto& matNode : asArray(s["Materials"]["Material"])) {
            Material mat;
            if (matNode.contains("_id")) mat.id = parseInt(matNode["_id"].get<std::string>());
            if (matNode.contains("_type") && matNode["_type"].get<std::string>() == "mirror") {
                mat.isMirror = true;
            }
            if (matNode.contains("AmbientReflectance")) {
                mat.ambientReflectance = parseVec3f(matNode["AmbientReflectance"].get<std::string>());
            }
            if (matNode.contains("DiffuseReflectance")) {
                mat.diffuseReflectance = parseVec3f(matNode["DiffuseReflectance"].get<std::string>());
            }
            if (matNode.contains("SpecularReflectance")) {
                mat.specularReflectance = parseVec3f(matNode["SpecularReflectance"].get<std::string>());
            }
            if (matNode.contains("MirrorReflectance")) {
                mat.mirrorReflectance = parseVec3f(matNode["MirrorReflectance"].get<std::string>());
            }
            if (matNode.contains("PhongExponent")) {
                mat.phongExponent = parseFloat(matNode["PhongExponent"].get<std::string>());
            }
            scene.materials.push_back(mat);
        }
    }

    // 5. VertexData
    if (s.contains("VertexData")) {
        std::string vData = getDataString(s["VertexData"]);
        std::stringstream ss(vData);
        Vec3f v;
        while (ss >> v.x >> v.y >> v.z) {
            scene.vertices.push_back(v);
        }
    }

    // 6. Objects
    if (s.contains("Objects")) {
        const json& objs = s["Objects"];

        // Meshes
        if (objs.contains("Mesh")) {
            for (const auto& meshNode : asArray(objs["Mesh"])) {
                Mesh mesh;
                mesh.materialId = parseInt(meshNode["Material"].get<std::string>()) - 1;
                if (meshNode.contains("Faces")) {
                    std::string fData = getDataString(meshNode["Faces"]);
                    std::stringstream ss(fData);
                    int i0, i1, i2;
                    while (ss >> i0 >> i1 >> i2) {
                        Triangle tri;
                        tri.materialId = mesh.materialId;
                        tri.v0 = i0 - 1;
                        tri.v1 = i1 - 1;
                        tri.v2 = i2 - 1;
                        mesh.faces.push_back(tri);
                    }
                }
                scene.meshes.push_back(mesh);
            }
        }

        // Triangles
        if (objs.contains("Triangle")) {
            for (const auto& triNode : asArray(objs["Triangle"])) {
                Triangle tri;
                tri.materialId = parseInt(triNode["Material"].get<std::string>()) - 1;
                std::stringstream ss(triNode["Indices"].get<std::string>());
                int i0, i1, i2;
                ss >> i0 >> i1 >> i2;
                tri.v0 = i0 - 1;
                tri.v1 = i1 - 1;
                tri.v2 = i2 - 1;
                scene.triangles.push_back(tri);
            }
        }

        // Spheres
        if (objs.contains("Sphere")) {
            for (const auto& sphNode : asArray(objs["Sphere"])) {
                Sphere sph;
                sph.materialId = parseInt(sphNode["Material"].get<std::string>()) - 1;
                sph.centerVertexId = parseInt(sphNode["Center"].get<std::string>()) - 1;
                sph.radius = parseFloat(sphNode["Radius"].get<std::string>());
                scene.spheres.push_back(sph);
            }
        }

        // Planes
        if (objs.contains("Plane")) {
            for (const auto& plNode : asArray(objs["Plane"])) {
                Plane pl;
                pl.materialId = parseInt(plNode["Material"].get<std::string>()) - 1;
                pl.centerVertexId = parseInt(plNode["Center"].get<std::string>()) - 1;
                pl.normal = parseVec3f(plNode["Normal"].get<std::string>());
                scene.planes.push_back(pl);
            }
        }

        // Cylinders
        if (objs.contains("Cylinder")) {
            for (const auto& cylNode : asArray(objs["Cylinder"])) {
                Cylinder cyl;
                cyl.materialId = parseInt(cylNode["Material"].get<std::string>()) - 1;
                cyl.centerVertexId = parseInt(cylNode["Center"].get<std::string>()) - 1;
                cyl.axis = parseVec3f(cylNode["Axis"].get<std::string>());
                cyl.radius = parseFloat(cylNode["Radius"].get<std::string>());
                cyl.height = parseFloat(cylNode["Height"].get<std::string>());
                scene.cylinders.push_back(cyl);
            }
        }
    }

    return true;
}

bool writePPM(const std::string& filename, int width, int height, const std::vector<Vec3i>& pixels) {
    if (static_cast<int>(pixels.size()) != width * height) {
        std::cerr << "Error: Pixel buffer size does not match width * height.\n";
        return false;
    }

    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open output PPM file: " << filename << "\n";
        return false;
    }

    // 3-line ASCII PPM (P3) header
    out << "P3\n";
    out << width << " " << height << "\n";
    out << "255\n";

    // Write RGB triplets row by row, starting from top-left
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Vec3i& p = pixels[y * width + x];
            out << p.r << " " << p.g << " " << p.b << (x + 1 == width ? "" : " ");
        }
        out << "\n";
    }

    return true;
}