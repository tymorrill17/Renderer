#include "utility/asset_loading.h"
#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
#include "renderer/mesh.h"
#include "utility/logger.h"
#include <memory>
#include <optional>
#include <vector>

void MeshAsset::cleanup() {
    GPU_mesh.cleanup();
}

void AssetManager::initialize(Renderer* renderer) {
    this->renderer = renderer;
}

std::optional<std::vector<std::shared_ptr<MeshAsset>>> AssetManager::load_mesh_GLTF(std::filesystem::path filepath) {
    Logger::log("Loading GLTF: " + filepath.string());

    fastgltf::Parser parser{};

    // Structure that holds information for reading data
    auto GLTF_data = fastgltf::GltfDataBuffer::FromPath(filepath);
    if (GLTF_data.error() != fastgltf::Error::None) {
        Logger::logError("The file couldn't be loaded: " + filepath.string() + " or the buffer could not be created");
        return {};
    }

    constexpr auto GLTF_options = fastgltf::Options::LoadExternalBuffers;

    auto asset = parser.loadGltf(GLTF_data.get(), filepath.parent_path(), GLTF_options); // Actually load in the binary data
    if (asset.error() != fastgltf::Error::None) {
        Logger::logError("Failed to load GLTF data: " + filepath.string());
        return {};
    }

    std::vector<std::shared_ptr<MeshAsset>> meshes;

    std::vector<uint32_t> indices;
    std::vector<MeshVertex> vertices;
    for (fastgltf::Mesh& mesh : asset->meshes) {
        MeshAsset new_mesh_asset;

        new_mesh_asset.name = mesh.name;

        // clear the mesh arrays each mesh, we dont want to merge them by error
        indices.clear();
        vertices.clear();

        for (auto&& primitive : mesh.primitives) {
            GeometricSurface new_surface;
            new_surface.index = (uint32_t)indices.size();
            new_surface.count = (uint32_t)asset->accessors[primitive.indicesAccessor.value()].count;

            size_t initial_vertex = vertices.size();

            // Load indices
            {
                fastgltf::Accessor& index_accessor = asset->accessors[primitive.indicesAccessor.value()];
                indices.reserve(indices.size() + index_accessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(asset.get(), index_accessor,
                    [&](std::uint32_t idx) {
                        indices.push_back(idx + initial_vertex);
                    });
            }

            // Load vertex positions
            {
                fastgltf::Accessor& vertex_pos_accessor = asset->accessors[primitive.findAttribute("POSITION")->accessorIndex];
                vertices.resize(vertices.size() + vertex_pos_accessor.count);

                // Also initialize the other optional attributes
                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), vertex_pos_accessor,
                    [&](glm::vec3 pos, size_t index) {
                        MeshVertex new_vertex;
                        new_vertex.position = pos;
                        new_vertex.normal = { 1, 0, 0 };
                        new_vertex.color = glm::vec4 { 1.0f };
                        new_vertex.uv_x = 0;
                        new_vertex.uv_y = 0;
                        vertices[initial_vertex + index] = new_vertex;
                    });
            }

            // Load vertex normals
            auto normals = primitive.findAttribute("NORMAL");
            if (normals != primitive.attributes.end()) { // If the normals are present
                fastgltf::Accessor& vertex_normal_accessor = asset->accessors[normals->accessorIndex];
                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), vertex_normal_accessor,
                    [&](glm::vec3 normal, size_t index) {
                        vertices[initial_vertex + index].normal = normal;
                    });
            }

            // load UVs
            auto uv = primitive.findAttribute("TEXCOORD_0");
            if (uv != primitive.attributes.end()) {
                fastgltf::Accessor& vertex_uv_accessor = asset->accessors[uv->accessorIndex];
                fastgltf::iterateAccessorWithIndex<glm::vec2>(asset.get(), vertex_uv_accessor,
                    [&](glm::vec2 uv, size_t index) {
                        vertices[initial_vertex + index].uv_x = uv.x;
                        vertices[initial_vertex + index].uv_y = uv.y;
                    });
            }

            // load vertex colors
            auto colors = primitive.findAttribute("COLOR_0");
            if (colors != primitive.attributes.end()) {
                fastgltf::Accessor& vertex_color_accessor = asset->accessors[colors->accessorIndex];
                fastgltf::iterateAccessorWithIndex<glm::vec4>(asset.get(), vertex_color_accessor,
                    [&](glm::vec4 color, size_t index) {
                        vertices[initial_vertex + index].color = color;
                    });
            }
            new_mesh_asset.surfaces.push_back(new_surface);
        }

        // Display the vertex normals instead of the actual colors
        constexpr bool OverrideColors = true;
        if (OverrideColors) {
            for (MeshVertex& vertex : vertices) {
                vertex.color = glm::vec4(vertex.normal, 1.f);
            }
        }

        new_mesh_asset.GPU_mesh.upload_to_GPU(renderer, vertices, indices);

        meshes.emplace_back(std::make_shared<MeshAsset>(std::move(new_mesh_asset)));
    }

    return meshes;
}
