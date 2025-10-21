#pragma once
#include <chrono>
#include <cmath>

class Timer {
public:
    void initialize();
	static Timer& get_timer() {
		static Timer instance;
		return instance;
	}
    void update();

	float frame_time;
    float fps_smoothing;
	float fps; // frames per second that are averaged over time

	std::chrono::steady_clock::time_point current_time;
	std::chrono::steady_clock::time_point new_time;
};
