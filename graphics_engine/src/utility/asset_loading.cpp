#include "utility/asset_loading.h"
#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
#include "utility/logger.h"
#include <optional>

std::optional<std::vector<MeshAsset*>> AssetManager::load_mesh_GLTF(Renderer* renderer, std::filesystem::path filepath) {
    Logger::log("Loading GLTF: " + filepath.string());

    fastgltf::GltfDataBuffer GLTF_data;
    GLTF_data.FromPath(filepath);

    constexpr auto GLTF_options = fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadGLBBuffers;

    fastgltf::Asset asset;
    fastgltf::Parser parser{};

    auto load = parser.loadGltfBinary(GLTF_data, filepath.parent_path(), GLTF_options);
    if (load) {
        asset = std::move(load.get());
    } else {
        Logger::logError("Failed to load GLTF: " + filepath.string());
        return {};
    }
}
