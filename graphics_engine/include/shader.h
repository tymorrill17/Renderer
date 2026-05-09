#pragma once
#include "vulkan/vulkan.h"
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"
#include "device.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <string>
#include <array>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

class ShaderManager {
public:
    void initialize();

    void find_shaders();
    void compile_shaders();

    inline const uint32_t* get_shader_code(const std::string& shader_name) { return static_cast<const uint32_t*>(compiled_spirv[shader_name]->getBufferPointer()); }
    inline uint32_t get_shader_code_length(const std::string& shader_name) { return compiled_spirv[shader_name]->getBufferSize(); }

    std::vector<std::string> found_shaders;
    std::map<std::string, Slang::ComPtr<slang::IBlob>> compiled_spirv;
};

class Shader {
public:
    void initialize(Device* device, ShaderManager* shader_manager, VkShaderStageFlagBits stage_flag, const std::string& shader);
    void cleanup();

    void build_shader_from_file(const std::string& filepath);
    void build_shader_from_code(const std::string& shader_name);

	static VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader);

    Device* device;
    ShaderManager* shader_manager;
    VkShaderModule module;
    VkShaderStageFlagBits stage;
};
