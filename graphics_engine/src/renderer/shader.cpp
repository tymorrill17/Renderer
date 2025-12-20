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
static const std::string shader_directory{SHADER_DIR};
#endif // SHADER_DIR


// SHADER MANAGER ----------------------------------------------------------------------

static void reportDiagnostics(const std::string& shader_name, slang::IBlob* diagnostic) {
    if (diagnostic != nullptr) {
        Logger::logError("for " + shader_name + ":");
        Logger::logError(static_cast<const char*>(diagnostic->getBufferPointer()));
    }
}

void ShaderManager::initialize() {
    compile_shaders();
}

void ShaderManager::find_shaders() {

    // We want to account for changes in files if we are recompiling, so clear the current list of shaders
    found_shaders.clear();

    // Iterate over the recursive directory iterator to find all shader files in ${Project_Source}/shaders/directory
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(shader_directory)) {
            if (!entry.is_directory() && entry.path().extension() == ".slang") {
                found_shaders.push_back(entry.path().stem().string()); // push back the stem (filename without extension)
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        std::cerr << "Path: " << e.path1() << '\n';
    }
}

void ShaderManager::compile_shaders() {

    Slang::ComPtr<slang::IGlobalSession> global_session;

    // A slang global session is simply a connection to the API (like a global context)
    SlangGlobalSessionDesc global_desc{};
    if (slang::createGlobalSession(&global_desc, global_session.writeRef()) != SLANG_OK) {
        Logger::logError("Failed to create a slang global session!");
    }

    // A session is a more localized context that maintains caching for reuse
    slang::SessionDesc session_desc{};

    // Define a target descriptor
    slang::TargetDesc target_desc{
        .format  = SLANG_SPIRV,
        .profile = global_session->findProfile("spirv_1_6"),
    };
    session_desc.targetCount = 1;
    session_desc.targets = &target_desc;

    // Enable compiler options. For now, we just want spir-v to be directly output from the compiler
    slang::CompilerOptionEntry options[2] = {
        {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
        {slang::CompilerOptionName::VulkanEmitReflection, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
    };
    session_desc.compilerOptionEntryCount = 2;
    session_desc.compilerOptionEntries = options;

    Slang::ComPtr<slang::ISession> session;

    // Now create the session
    global_session->createSession(session_desc, session.writeRef());

    Logger::logError("Finding Shaders and Creating Modules...");

    // Search the shaders directory for all the .slang files and convert them into modules
    find_shaders();

    if (found_shaders.size() < 1) {
        Logger::log("No shaders found in shader directory!");
        return;
    }

    std::vector<Slang::ComPtr<slang::IModule>> slang_modules;
    for (int i_shader = 0; i_shader < found_shaders.size(); i_shader++) {
        Slang::ComPtr<slang::IBlob> diagnostic;
        std::string filepath = shader_directory + found_shaders[i_shader];
        Slang::ComPtr<slang::IModule> module(session->loadModule(filepath.c_str(), diagnostic.writeRef()));

        reportDiagnostics(found_shaders[i_shader], diagnostic);
        Logger::logError("filepath" + filepath);

        slang_modules.push_back(module);
    }

    Logger::logError("Finding Entry Points...");

    // Get the entry points to each shader program. For now each will have a vertex and fragment, but
    // this should be generalized in the future
    Slang::ComPtr<slang::IBlob> diagnostic;
    for (int i_shader = 0; i_shader < slang_modules.size(); i_shader++) {
        const auto& module = slang_modules[i_shader];
        int entry_point_count = module->getDefinedEntryPointCount();
        std::vector<Slang::ComPtr<slang::IEntryPoint>> module_entry_points;
        for (int i_entry_point = 0; i_entry_point < entry_point_count; i_entry_point++) {
            Slang::ComPtr<slang::IEntryPoint> entry_point;
            module->getDefinedEntryPoint(i_entry_point, entry_point.writeRef());
            if (!entry_point) {
                Logger::logError("Error getting entry points from shader!");
            }
            module_entry_points.push_back(entry_point);
        }

        Logger::logError("Composing Programs...");

        Slang::ComPtr<slang::IComponentType> composed_programs;
        std::vector<slang::IComponentType*> program_components;
        program_components.push_back(module); // Push back the module component to the composedProgram
        // Since there may be multiple entry points per shader, the entry points multimap needs to return a range
        for (const auto& ep : module_entry_points) {
            program_components.push_back(ep);
        }

        SlangResult result = session->createCompositeComponentType(program_components.data(), program_components.size(), composed_programs.writeRef(), diagnostic.writeRef());
        reportDiagnostics(found_shaders[i_shader], diagnostic);

        Logger::logError("Linking programs...");

        // Lastly, make sure there are no missing dependencies by linking the composed programs
        Slang::ComPtr<slang::IComponentType> linked_program;
        result = composed_programs->link(linked_program.writeRef(), diagnostic.writeRef());

        reportDiagnostics(found_shaders[i_shader], diagnostic);

        // Finally, we can get the compiled target code for each entry points
        slang::ProgramLayout* layout = linked_program->getLayout();
        if (!layout) {
            Logger::logError("No program layout for shader");
        }

        // Compile each entry point
        for (int i_entry_point = 0; i_entry_point < entry_point_count; i_entry_point++) {
            Slang::ComPtr<slang::IBlob> diagnostic;

            // Compiled code gets stored into an IBlob
            Slang::ComPtr<slang::IBlob> spirv_code;
            linked_program->getEntryPointCode(i_entry_point, 0, spirv_code.writeRef(), diagnostic.writeRef());

            slang::EntryPointReflection* entry_point_reflect = layout->getEntryPointByIndex(i_entry_point);
            if (!entry_point_reflect) {
                Logger::logError("No entry point reflection for index: " + std::to_string(i_entry_point));
            }
            std::string entry_point_name = std::string(entry_point_reflect->getName());
            Logger::logError("entry point: " + entry_point_name);

            compiled_spirv.insert({entry_point_name, spirv_code});

//            std::ofstream file(entry_point_name+"debug.spv", std::ios::binary);
//            file.write(reinterpret_cast<const char*>(spirv_code->getBufferPointer()),
//                    spirv_code->getBufferSize());
//            file.close();
        }
    }
}

// SHADER ------------------------------------------------------------------------------

static bool is_filename(std::string potential_file) {
    // I think the simplest way for now is to just check if the string has a file extension, since I don't
    // think an entry point can have a name with a "." in it
    std::filesystem::path filepath(potential_file);
    if (!filepath.extension().empty()) {
        return true; // Then we have a file!
    }
    return false;
}

void Shader::initialize(Device* device, ShaderManager* shader_manager, VkShaderStageFlagBits stage_flag, const std::string& shader) {

    this->device = device;
    this->shader_manager = shader_manager;
    this->stage = stage_flag;

    // First, check to see if "shader" is referring to an existing file
    if (is_filename(shader)) {
        build_shader_from_file(shader);
    } else {
        build_shader_from_code(shader);
    }
}

void Shader::build_shader_from_file(const std::string& filepath) {
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
	VkShaderModuleCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = buffer.size() * sizeof(uint32_t), // codeSize has to be in byte
        .pCode = buffer.data()
	};

	if (vkCreateShaderModule(device->logical_device, &create_info, nullptr, &module) != VK_SUCCESS) {
        Logger::logError("Error: vkCreateShaderModule() failed while creating " + std::string(filepath));
	}
    Logger::log("Shader successfully loaded: " + filepath);
}

void Shader::build_shader_from_code(const std::string& shader_name) {
	VkShaderModuleCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = shader_manager->get_shader_code_length(shader_name),
        .pCode = shader_manager->get_shader_code(shader_name)
    };

	if (vkCreateShaderModule(device->logical_device, &create_info, nullptr, &module) != VK_SUCCESS) {
        Logger::logError("Error: vkCreateShaderModule() failed");
	}

    Logger::log("Shader successfully loaded: " + shader_name);
}

void Shader::cleanup() {
    vkDestroyShaderModule(device->logical_device, module, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader) {
	VkPipelineShaderStageCreateInfo create_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.stage = stage,
		.module = shader,
		.pName = "main" // entry point of the shader program
	};
	return create_info;
}
