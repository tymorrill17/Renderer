#include "renderer/shader.h"
#include "renderer/device.h"
#include "slang/slang-com-ptr.h"
#include "slang/slang.h"
#include "slang/slang-com-helper.h"
#include "utility/logger.h"
#include "vulkan/vulkan_core.h"
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#ifdef SHADER_DIR
// Global variable for the shaders folder
const std::string shaderDirectory(SHADER_DIR);
#endif // SHADER_DIR


// SHADER MANAGER ----------------------------------------------------------------------

static void reportDiagnostics(const std::string& shaderName, slang::IBlob* diagnostic) {
    if (diagnostic != nullptr) {
        Logger::logError("for " + shaderName + ":");
        Logger::logError(static_cast<const char*>(diagnostic->getBufferPointer()));
    }
}

ShaderManager::ShaderManager() {
    compileShaders();
}

ShaderManager::~ShaderManager() {
    // Destroy any objects created by slang before destroying
}

void ShaderManager::findShaders() {
    // We want to account for changes in files if we are recompiling, so clear the current list of shaders
    _shaders.clear();

    // Iterate over the recursive directory iterator to find all shader files in ${Project_Source}/shaders/directory
    for (const auto& entry : std::filesystem::recursive_directory_iterator(shaderDirectory)) {
        if (!entry.is_directory() && entry.path().extension() == ".slang") {
            _shaders.push_back(entry.path().stem().string()); // push back the stem (filename without extension)
        }
    }
}

void ShaderManager::compileShaders() {
    // A slang global session is simply a connection to the API (like a global context)
    SlangGlobalSessionDesc globalDesc{};
    if (slang::createGlobalSession(&globalDesc, _globalSession.writeRef()) != SLANG_OK) {
        Logger::logError("Failed to create a slang global session!");
    }

    // A session is a more localized context that maintains caching for reuse
    slang::SessionDesc sessionDesc{};

    // Define a target descriptor
    slang::TargetDesc targetDesc{
        .format  = SLANG_SPIRV,
        .profile = _globalSession->findProfile("spirv_1_6"),
    };
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // Enable compiler options. For now, we just want spir-v to be directly output from the compiler
    slang::CompilerOptionEntry options[2] = {
        {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
        {slang::CompilerOptionName::VulkanEmitReflection, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
    };
    sessionDesc.compilerOptionEntryCount = 2;
    sessionDesc.compilerOptionEntries = options;


    // Now create the session
    _globalSession->createSession(sessionDesc, _session.writeRef());

    Logger::logError("Finding Shaders and Creating Modules...");

    // Search the shaders directory for all the .slang files and convert them into modules
    findShaders();
    std::vector<Slang::ComPtr<slang::IModule>> slangModules;
    for (int nShader = 0; nShader < _shaders.size(); nShader++) {
        Slang::ComPtr<slang::IBlob> diagnostic;
        std::string filepath = shaderDirectory + _shaders[nShader];
        Slang::ComPtr<slang::IModule> module(_session->loadModule(filepath.c_str(), diagnostic.writeRef()));

        reportDiagnostics(_shaders[nShader], diagnostic);
        Logger::logError("filepath" + filepath);

        slangModules.push_back(module);
    }

    Logger::logError("Finding Entry Points...");

    // Get the entry points to each shader program. For now each will have a vertex and fragment, but
    // this should be generalized in the future
    Slang::ComPtr<slang::IBlob> diagnostic;
    for (int nShader = 0; nShader < slangModules.size(); nShader++) {
        const auto& module = slangModules[nShader];
        int entryPointCount = module->getDefinedEntryPointCount();
        std::vector<Slang::ComPtr<slang::IEntryPoint>> moduleEntryPoints;
        for (int nEntry = 0; nEntry < entryPointCount; nEntry++) {
            Slang::ComPtr<slang::IEntryPoint> entryPoint;
            module->getDefinedEntryPoint(nEntry, entryPoint.writeRef());
            if (!entryPoint) {
                Logger::logError("Error getting entry points from shader!");
            }
            moduleEntryPoints.push_back(entryPoint);
        }

        Logger::logError("Composing Programs...");

        Slang::ComPtr<slang::IComponentType> composedProgram;
        std::vector<slang::IComponentType*> programComponents;
        programComponents.push_back(module); // Push back the module component to the composedProgram
        // Since there may be multiple entry points per shader, the entry points multimap needs to return a range
        for (const auto& ep : moduleEntryPoints) {
            programComponents.push_back(ep);
        }

        SlangResult result = _session->createCompositeComponentType(programComponents.data(), programComponents.size(), composedProgram.writeRef(), diagnostic.writeRef());
        reportDiagnostics(_shaders[nShader], diagnostic);

        Logger::logError("Linking programs...");

        // Lastly, make sure there are no missing dependencies by linking the composed programs
        Slang::ComPtr<slang::IComponentType> linkedProgram;
        result = composedProgram->link(linkedProgram.writeRef(), diagnostic.writeRef());

        reportDiagnostics(_shaders[nShader], diagnostic);

        // Finally, we can get the compiled target code for each entry points
        slang::ProgramLayout* layout = linkedProgram->getLayout();
        if (!layout) {
            Logger::logError("No program layout for shader");
        }

        // Compile each entry point
        for (int nEntryPoint = 0; nEntryPoint < entryPointCount; nEntryPoint++) {
            Slang::ComPtr<slang::IBlob> diagnostic;

            // Compiled code gets stored into an IBlob
            Slang::ComPtr<slang::IBlob> spirvCode;
            linkedProgram->getEntryPointCode(nEntryPoint, 0, spirvCode.writeRef(), diagnostic.writeRef());

            slang::EntryPointReflection* entryReflect = layout->getEntryPointByIndex(nEntryPoint);
            if (!entryReflect) {
                Logger::logError("No entry point reflection for index: " + std::to_string(nEntryPoint));
            }
            std::string entryPointName = std::string(entryReflect->getName());
            Logger::logError("entry point: " + entryPointName);

            _compiledSPIRV.insert({entryPointName, spirvCode});

            std::ofstream file(entryPointName+"debug.spv", std::ios::binary);
            file.write(reinterpret_cast<const char*>(spirvCode->getBufferPointer()),
                    spirvCode->getBufferSize());
            file.close();
        }
    }
}

// SHADER ------------------------------------------------------------------------------

static bool isFilename(std::string potentialFile) {
    // I think the simplest way for now is to just check if the string has a file extension, since I don't
    // think an entry point can have a name with a "." in it
    std::filesystem::path filepath(potentialFile);
    if (!filepath.extension().empty()) {
        return true; // Then we have a file!
    }
    return false;
}

Shader::Shader(Device* device, ShaderManager* shaderManager, VkShaderStageFlagBits stageFlag, std::string shader)
	: _device(device),
    _shaderManager(shaderManager),
	_shaderStageFlag(stageFlag) {

    // First, check to see if "shader" is referring to an existing file
    if (isFilename(shader)) {
        buildShaderFromFile(shader);
    } else {
        buildShaderFromCode(shader);
    }
}

void Shader::buildShaderFromFile(std::string filepath) {
	// std::ios::ate -> puts stream curser at end
	// std::ios::binary -> opens file in binary mode
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
        Logger::logError("ERROR: Shader file does not exist: " + std::string(filepath));
	}

	// Since cursor is at the end, use it to find the size of the file, then copy the entire shader into a vector of uint32_t
	size_t filesize = (size_t)file.tellg(); // tellg() returns the position of the cursor
	std::vector<uint32_t> buffer(filesize / sizeof(uint32_t));
	file.seekg(0); // move cursor to beginning
	file.read((char*)buffer.data(), filesize); // load entire file into the buffer
	file.close();

	// Now we have the entire shader in the buffer and can load it to Vulkan
	VkShaderModuleCreateInfo createinfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = buffer.size() * sizeof(uint32_t), // codeSize has to be in byte
        .pCode = buffer.data()
	};

	if (vkCreateShaderModule(_device->handle(), &createinfo, nullptr, &_shaderModule) != VK_SUCCESS) {
        Logger::logError("Error: vkCreateShaderModule() failed while creating " + std::string(filepath));
	}
    Logger::log("Shader successfully loaded: " + filepath);
}

void Shader::buildShaderFromCode(std::string shader) {
	VkShaderModuleCreateInfo createinfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = _shaderManager->getShaderCodeLength(shader),
        .pCode = (uint32_t*)_shaderManager->getShaderCode(shader)
    };

	if (vkCreateShaderModule(_device->handle(), &createinfo, nullptr, &_shaderModule) != VK_SUCCESS) {
        Logger::logError("Error: vkCreateShaderModule() failed");
	}

    Logger::log("Shader successfully loaded: " + shader);
}

Shader::~Shader() {
    vkDestroyShaderModule(_device->handle(), _shaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader) {
	VkPipelineShaderStageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.stage = stage,
		.module = shader,
		.pName = "main" // entry point of the shader program
	};
	return createInfo;
}
