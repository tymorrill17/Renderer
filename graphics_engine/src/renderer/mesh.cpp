#include "renderer/mesh.h"
#include "utility/allocator.h"
#include "utility/logger.h"
#include <cmath>

void GPUMeshBuffers::upload_to_buffers(Renderer* renderer, std::span<MeshVertex> vertices, std::span<uint32_t> indices)
{
    // TODO: @Error do better error handling here
    if (renderer == nullptr) return;

	const size_t vertex_buffer_size = vertices.size() * sizeof(MeshVertex);
	const size_t index_buffer_size = indices.size() * sizeof(uint32_t);

	// Create vertex buffer
	this->vertex_buffer = renderer->create_buffer(
        vertex_buffer_size,
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY
    );

	// Find and store the adress of the vertex buffer
	VkBufferDeviceAddressInfo buffer_device_address_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = this->vertex_buffer.handle
    };
	this->vertex_buffer_address = vkGetBufferDeviceAddress(renderer->device.logical_device, &buffer_device_address_info);

	// Create index buffer
    this->index_buffer = renderer->create_buffer(
        index_buffer_size,
        1,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY
    );

    // Since we created the vertex and index buffers on GPU-only memory, we use a staging_buffer that uses CPU-only memory to copy data to the GPU buffers
	Buffer staging_buffer = renderer->create_buffer(vertex_buffer_size + index_buffer_size, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    staging_buffer.map();
    staging_buffer.write_data(vertices.data(), vertex_buffer_size);
    staging_buffer.write_data(indices.data(), index_buffer_size, vertex_buffer_size);

	renderer->immediate_command.run_command([&](VkCommandBuffer cmd) {
		VkBufferCopy vertex_copy{ 0 };
		vertex_copy.dstOffset = 0;
		vertex_copy.srcOffset = 0;
		vertex_copy.size = vertex_buffer_size;

		vkCmdCopyBuffer(cmd, staging_buffer.handle, this->vertex_buffer.handle, 1, &vertex_copy);

		VkBufferCopy index_copy{ 0 };
		index_copy.dstOffset = 0;
		index_copy.srcOffset = vertex_buffer_size;
		index_copy.size = index_buffer_size;

		vkCmdCopyBuffer(cmd, staging_buffer.handle, this->index_buffer.handle, 1, &index_copy);
	});

    staging_buffer.unmap();
    staging_buffer.cleanup();
}

void GPUMeshBuffers::cleanup() {
    vertex_buffer.cleanup();
    index_buffer.cleanup();
}

// ------------------------- Mesh -------------------------------------

void Mesh::upload_to_GPU(Renderer* renderer) {
    gpu_buffers.upload_to_buffers(renderer, vertices, indices);
}

// ------------------------- Primitive Shapes -------------------------

Mesh PrimitiveShapes::Rectangle(float width, float height) {
    Mesh new_shape;
    const float half_x_len = (float)width / 2.0f;
    const float half_y_len = (float)height / 2.0f;

    new_shape.vertices.resize(4); // 4 vertices in a rectangle
    new_shape.indices.resize(6);  // 4 vertices require 2 triangles => 6 indices

    new_shape.vertices[0].position = { half_x_len, -half_y_len, 0.0f };
	new_shape.vertices[1].position = { half_x_len,  half_y_len, 0.0f };
	new_shape.vertices[2].position = {-half_x_len, -half_y_len, 0.0f };
	new_shape.vertices[3].position = {-half_x_len,  half_y_len, 0.0f };

	new_shape.vertices[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	new_shape.vertices[1].color = { 0.5f, 0.5f, 0.5f, 1.0f };
	new_shape.vertices[2].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	new_shape.vertices[3].color = { 0.0f, 1.0f, 0.0f, 1.0f };

	new_shape.indices[0] = 0;
	new_shape.indices[1] = 1;
	new_shape.indices[2] = 2;

	new_shape.indices[3] = 2;
	new_shape.indices[4] = 1;
	new_shape.indices[5] = 3;
    return new_shape;
}




