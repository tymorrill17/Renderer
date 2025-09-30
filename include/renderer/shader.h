#pragma once
#include "NonCopyable.h"
#include "vulkan/vulkan.h"
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"
#include "device.h"
#include <cstdint>
#include <string>
#include <array>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

class ShaderManager : public NonCopyable {
public:
    ShaderManager();
    ~ShaderManager();

    // @brief Recursively search shaders directory for slang files
    void findShaders();

    // @brief compiles each shader in the program shaders directory
    void compileShaders();

    // @brief return the compiled shader code associated with shaderName from _compiledShaders
    inline const uint32_t* getShaderCode(std::string shaderName) { return static_cast<const uint32_t*>(_compiledSPIRV[shaderName]->getBufferPointer()); }
    inline uint32_t getShaderCodeLength(std::string shaderName) { return _compiledSPIRV[shaderName]->getBufferSize(); }

private:
    Slang::ComPtr<slang::IGlobalSession> _globalSession;
    Slang::ComPtr<slang::ISession> _session;

    std::vector<std::string> _shaders;
    std::map<std::string, Slang::ComPtr<slang::IBlob>> _compiledSPIRV;
};


class Shader : public NonCopyable {
public:
	Shader(Device* device, ShaderManager* shaderManager, VkShaderStageFlagBits stageFlag, std::string shader);
	~Shader();

    void buildShaderFromFile(std::string filepath);
    void buildShaderFromCode(std::string shader);

	inline VkShaderModule module() const { return _shaderModule; }
	inline VkShaderStageFlagBits stage() const { return _shaderStageFlag; }

	// @brief Populates a pipeline shader stage create info struct
	// @param flags - Bit flags to enable in the create info struct
	// @param shader - Shader module to be added to the pipeline
	static VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader);

private:
	Device* _device;
    ShaderManager* _shaderManager;
	VkShaderModule _shaderModule;
	VkShaderStageFlagBits _shaderStageFlag;

    void readShaderCode(const std::string& filepath);
};
