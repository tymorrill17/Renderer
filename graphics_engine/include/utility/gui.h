#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_vulkan.h"
#include "NonCopyable.h"
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

class Gui : public NonCopyable {
public:
	static Gui& getGui() {
		static Gui instance;
		return instance;
	}

	// @brief Let ImGui process the SDL Event that was caught
	// @param event - SDL Event to be processed
	void processInputs(SDL_Event* event);

	// @brief Add an ImGui widget to the window corresponding to windowName. Widgets will be located in their corresponding windows
	// @param windowName - Name of window where widget will be located
	// @param widget - ImGui function handle to call when constructWindow is called
	void addWidget(const std::string& windowName, const std::function<void()>& widget);

	// @brief constructs the windows stored in the widget dictionary right before rendering.
	void constructWindows();

private:
	std::unordered_map<std::string, std::vector<std::function<void()>>> _widgetDictionary;
};
