#pragma once
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include <cstdint>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <vector>
#include <optional>

struct GeometricSurface {
    uint32_t index;
    uint32_t count;
};

struct MeshAsset {
    std::string name;

    std::vector<GeometricSurface> surfaces;
    Mesh mesh;
};

namespace AssetManager {
    std::optional<std::vector<MeshAsset*>> load_mesh_GLTF(Renderer* renderer, std::filesystem::path filepath);
}
