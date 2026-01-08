#pragma once

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "renderer/descriptor.h"
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

class Gui {
public:
	static Gui& get_gui() {
		static Gui instance;
		return instance;
	}

    void initialize(Renderer* renderer);
    void cleanup();

	void process_inputs(SDL_Event* event);
	void add_widget(const std::string& window_name, const std::function<void()>& widget);
	void construct_windows();

    void start_frame();
    void end_frame();

    void draw(Command* cmd);

	std::unordered_map<std::string, std::vector<std::function<void()>>> widget_dictionary;
    Renderer* renderer;
    DescriptorPool descriptor_pool;
};
