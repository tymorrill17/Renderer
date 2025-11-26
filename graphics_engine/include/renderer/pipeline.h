#pragma once
#include "vulkan/vulkan.h"
#include "utility/logger.h"
#include "device.h"
#include "shader.h"
#include "vulkan/vulkan_core.h"

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
	VkPipelineVertexInputStateCreateInfo vertex_input_info{ .sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	VkPipelineInputAssemblyStateCreateInfo input_assembly{ .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	VkPipelineRasterizationStateCreateInfo rasterizer{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	VkPipelineMultisampleStateCreateInfo multisampling{ .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	VkPipelineDepthStencilStateCreateInfo depth_stencil{ .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	VkPipelineRenderingCreateInfo rendering_info{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	VkFormat color_attachment_format{ VK_FORMAT_UNDEFINED };

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
	std::vector<VkPushConstantRange> push_constant_ranges{};
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
	PipelineBuilder& set_vertex_input_state(VkPipelineVertexInputStateCreateInfo create_info);
	PipelineBuilder& add_descriptor(VkDescriptorSetLayout descriptor);
	PipelineBuilder& add_push_constant(VkPushConstantRange push_constant);
	static VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();

    Device* device;
    PipelineConfig config;
};
