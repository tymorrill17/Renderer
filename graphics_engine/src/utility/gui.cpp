#include "utility/gui.h"

void Gui::process_inputs(SDL_Event* event) {
	ImGui_ImplSDL2_ProcessEvent(event);
}

void Gui::add_widget(const std::string& window_name, const std::function<void()>& widget) {
	widget_dictionary[window_name].push_back(widget);
}

void Gui::construct_windows() {
	for (auto& [window_name, widgets] : widget_dictionary) {
		ImGui::Begin(window_name.c_str());
		for (auto& widget : widgets) {
			widget();
		}
		ImGui::End();
	}
	widget_dictionary.clear();
}
