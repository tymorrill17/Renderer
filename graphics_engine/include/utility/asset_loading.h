#pragma once
#include "renderer/mesh.h"
#include <cstdint>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <memory>
#include <vector>
#include <optional>

class Renderer;

struct GeometricSurface {
    uint32_t index;
    uint32_t count;
};

class MeshAsset {
public:
    void cleanup();

    std::string name;
    std::vector<GeometricSurface> surfaces;
    GPUMesh GPU_mesh;
};

class AssetManager {
public:
    void initialize(Renderer* renderer);
    std::optional<std::vector<std::shared_ptr<MeshAsset>>> load_mesh_GLTF(std::filesystem::path filepath);

    Renderer* renderer;
};
