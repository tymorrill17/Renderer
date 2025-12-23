#pragma once
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "device.h"
#include "shader.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <span>

// PIPELINE ----------------------------------------------------------------------------------------------------------------------------

class Pipeline {
public:
    void cleanup();

    Device* device;
    VkPipeline handle;
    VkPipelineLayout layout;
};

namespace PipelineLayout {
	VkPipelineLayoutCreateInfo pipeline_layout_create_info(
        const std::vector<VkDescriptorSetLayout>& set_layouts = {},
        const std::vector<VkPushConstantRange>& push_constant_ranges = {}
    );
	VkPipelineLayout create_pipeline_layout(Device* device, VkPipelineLayoutCreateInfo create_info);
};

// PIPELINE BUILDER --------------------------------------------------------------------------------------------------------------------

struct PipelineConfig {
	// Shaders
	std::vector<VkPipelineShaderStageCreateInfo> shader_modules;

	// Pipeline State
	VkPipelineInputAssemblyStateCreateInfo input_assembly{ .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	VkPipelineRasterizationStateCreateInfo rasterizer{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	VkPipelineMultisampleStateCreateInfo multisampling{ .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	VkPipelineDepthStencilStateCreateInfo depth_stencil{ .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	VkPipelineRenderingCreateInfo rendering_info{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	VkFormat color_attachment_format{ VK_FORMAT_UNDEFINED };

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
	std::vector<VkPushConstantRange> push_constant_ranges{};
    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions{};
    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions{};
};

class PipelineBuilder {
public:
    void initialize(Device* device);

    void clear();
    Pipeline build();
    PipelineBuilder& set_config(PipelineConfig config);
    PipelineBuilder& set_shader(Shader shader);
	PipelineBuilder& set_input_topology(VkPrimitiveTopology topology);
	PipelineBuilder& set_polygon_mode(VkPolygonMode mode);
	PipelineBuilder& set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);
	PipelineBuilder& set_multisampling(VkSampleCountFlagBits sample_count);
	PipelineBuilder& set_blending(bool enable);
	PipelineBuilder& set_color_attachment_format(VkFormat format);
	PipelineBuilder& set_depth_attachment_format(VkFormat format);
	PipelineBuilder& set_depth_test(VkCompareOp compare_op = VK_COMPARE_OP_NEVER);
	PipelineBuilder& add_descriptor(VkDescriptorSetLayout descriptor);
	PipelineBuilder& add_push_constant(VkPushConstantRange push_constant);
    PipelineBuilder& add_vertex_binding_description(VkVertexInputBindingDescription binding_description);
    PipelineBuilder& add_vertex_attribute_description(VkVertexInputAttributeDescription attribute_description);

	static VkVertexInputBindingDescription vertex_input_binding_description(uint32_t binding, uint32_t stride, VkVertexInputRate vertex_input_rate);
	static VkVertexInputAttributeDescription vertex_input_attribute_description(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);

    Device* device;
    PipelineConfig config;
};
