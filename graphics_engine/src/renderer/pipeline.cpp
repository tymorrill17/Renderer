#include "renderer/pipeline.h"
#include <iostream>
#include <utility>

// PIPELINE  ------------------------------------------------------------------------------------------------------------------------------------

void Pipeline::cleanup() {
    vkDestroyPipelineLayout(device->logical_device, layout, nullptr);
    vkDestroyPipeline(device->logical_device, handle, nullptr);
}

// PIPELINE BUILDER -----------------------------------------------------------------------------------------------------------------------------

void PipelineBuilder::initialize(Device* device) {
    this->device = device;
	clear();
}

Pipeline PipelineBuilder::build() {

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = (uint32_t)config.vertex_binding_descriptions.size(),
        .pVertexBindingDescriptions      = config.vertex_binding_descriptions.data(),
        .vertexAttributeDescriptionCount = (uint32_t)config.vertex_attribute_descriptions.size(),
        .pVertexAttributeDescriptions    = config.vertex_attribute_descriptions.data(),
    };

    VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Setup dummy color blending. Not using transparent objects yet.
    VkPipelineColorBlendStateCreateInfo color_blending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &config.color_blend_attachment
    };

    // Dynamic states allow us to specify these things at command recording instead of pipeline creation
    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = &state[0]
    };

    VkPipelineLayout layout = PipelineLayout::create_pipeline_layout(
        device,
        PipelineLayout::pipeline_layout_create_info(config.descriptor_set_layouts, config.push_constant_ranges)
    );

    // Build the pipeline
    VkGraphicsPipelineCreateInfo pipeline_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &config.rendering_info,
        .stageCount = static_cast<uint32_t>(config.shader_modules.size()),
        .pStages = config.shader_modules.data(),
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &config.input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &config.rasterizer,
        .pMultisampleState = &config.multisampling,
        .pDepthStencilState = &config.depth_stencil,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = layout,
        .renderPass = nullptr // Using dynamic rendering
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device->logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
        Logger::logError("Failed to create pipeline");
    }

    Pipeline new_pipeline;
    new_pipeline.device = device;
    new_pipeline.handle = pipeline;
    new_pipeline.layout = layout;

    return new_pipeline;
}

void PipelineBuilder::clear() {
    config.shader_modules.clear();
    config.input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    config.rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    config.color_blend_attachment = {};
    config.multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    config.depth_stencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    config.rendering_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    config.color_attachment_format = VK_FORMAT_UNDEFINED;
    config.descriptor_set_layouts.clear();
    config.push_constant_ranges.clear();
    config.vertex_binding_descriptions.clear();
    config.vertex_attribute_descriptions.clear();
}

PipelineBuilder& PipelineBuilder::set_config(PipelineConfig config) {
    clear();
    this->config = config;
    return *this;
}

// Shaders

PipelineBuilder& PipelineBuilder::set_shader(Shader shader) {
    config.shader_modules.push_back(Shader::pipeline_shader_stage_create_info(shader.stage, shader.module));
    return *this;
}

// Pipeline State

PipelineBuilder& PipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
    config.input_assembly.topology = topology;
    config.input_assembly.primitiveRestartEnable = VK_FALSE; // Not using for now
    return *this;
}

PipelineBuilder& PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
    config.rasterizer.polygonMode = mode;
    config.rasterizer.lineWidth = 1.0f; // Setting this to a default of 1.0
    return *this;
}

PipelineBuilder& PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face) {
    config.rasterizer.cullMode = cull_mode;
    config.rasterizer.frontFace = front_face;
    return *this;
}

PipelineBuilder& PipelineBuilder::set_multisampling(VkSampleCountFlagBits sample_count) {
    // TODO: Defaulting to none until I learn more about this
    config.multisampling.sampleShadingEnable = VK_FALSE;
    config.multisampling.rasterizationSamples = sample_count;
    config.multisampling.minSampleShading = 1.0f;
    config.multisampling.pSampleMask = nullptr;
    config.multisampling.alphaToCoverageEnable = VK_FALSE;
    config.multisampling.alphaToOneEnable = VK_FALSE;
    return *this;
}

PipelineBuilder& PipelineBuilder::set_blending(bool enable) {
    config.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    config.color_blend_attachment.blendEnable = enable;
    return *this;
}

PipelineBuilder& PipelineBuilder::set_color_attachment_format(VkFormat format) {
    config.color_attachment_format = format;
    config.rendering_info.colorAttachmentCount = 1;
    config.rendering_info.pColorAttachmentFormats = &config.color_attachment_format;
    return *this;
}

PipelineBuilder& PipelineBuilder::set_depth_attachment_format(VkFormat format) {
    config.rendering_info.depthAttachmentFormat = format;
    return *this;
}

PipelineBuilder& PipelineBuilder::set_depth_test(VkCompareOp compare_op) {
    config.depth_stencil.depthTestEnable = compare_op == VK_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
    config.depth_stencil.depthWriteEnable = compare_op == VK_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
    config.depth_stencil.depthCompareOp = compare_op;
    config.depth_stencil.depthBoundsTestEnable = VK_FALSE;
    config.depth_stencil.minDepthBounds = 0.0f;
    config.depth_stencil.maxDepthBounds = 1.0f;
    config.depth_stencil.stencilTestEnable = VK_FALSE;
    config.depth_stencil.front = {};
    config.depth_stencil.back = {};
    return *this;
}

// Pipeline Layout

// TODO: @Cleanup make sure passing a span by value is acceptable
PipelineBuilder& PipelineBuilder::add_descriptor(VkDescriptorSetLayout descriptor) {
    config.descriptor_set_layouts.push_back(descriptor);
    return *this;
}

PipelineBuilder& PipelineBuilder::add_push_constant(VkPushConstantRange push_constant) {
    config.push_constant_ranges.push_back(push_constant);
    return *this;
}

PipelineBuilder& PipelineBuilder::add_vertex_binding_description(VkVertexInputBindingDescription binding_description) {
    config.vertex_binding_descriptions.push_back(binding_description);
    return *this;
}

PipelineBuilder& PipelineBuilder::add_vertex_attribute_description(VkVertexInputAttributeDescription attribute_description) {
    config.vertex_attribute_descriptions.push_back(attribute_description);
    return *this;
}

VkVertexInputBindingDescription PipelineBuilder::vertex_input_binding_description(uint32_t binding, uint32_t stride, VkVertexInputRate vertex_input_rate) {
    VkVertexInputBindingDescription binding_description{
        .binding   = binding,
        .stride    = stride,
        .inputRate = vertex_input_rate
    };
    return binding_description;
}

VkVertexInputAttributeDescription PipelineBuilder::vertex_input_attribute_description(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) {
    VkVertexInputAttributeDescription attribute_description{
        .location = location,
        .binding = binding,
        .format = format,
        .offset = offset
    };
    return attribute_description;
}

//-------------------------- Pipeline Layout --------------------------------//

VkPipelineLayoutCreateInfo PipelineLayout::pipeline_layout_create_info(const std::vector<VkDescriptorSetLayout>& set_layouts, const std::vector<VkPushConstantRange>& push_constant_ranges) {
    VkPipelineLayoutCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
            .pSetLayouts = set_layouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size()),
            .pPushConstantRanges = push_constant_ranges.data()
    };
    return create_info;
}

VkPipelineLayout PipelineLayout::create_pipeline_layout(Device* device, VkPipelineLayoutCreateInfo create_info) {
    VkPipelineLayout pipeline_layout;
    if (vkCreatePipelineLayout(device->logical_device, &create_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
        Logger::logError("Failed to create pipeline layout!");
    }
    return pipeline_layout;
}


