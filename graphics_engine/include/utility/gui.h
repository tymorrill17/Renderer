#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_vulkan.h"
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

	void process_inputs(SDL_Event* event);
	void add_widget(const std::string& window_name, const std::function<void()>& widget);
	void construct_windows();

	std::unordered_map<std::string, std::vector<std::function<void()>>> widget_dictionary;
};
