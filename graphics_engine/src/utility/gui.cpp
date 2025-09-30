#include "utility/gui.h"

void Gui::processInputs(SDL_Event* event) {
	ImGui_ImplSDL2_ProcessEvent(event);
}

void Gui::addWidget(const std::string& windowName, const std::function<void()>& widget) {
	_widgetDictionary[windowName].push_back(widget);
}

void Gui::constructWindows() {
	for (auto& [windowName, widgets] : _widgetDictionary) {
		ImGui::Begin(windowName.c_str());
		for (auto& widget : widgets) {
			widget();
		}
		ImGui::End();
	}
	_widgetDictionary.clear();
}