#pragma once
#include <chrono>
#include <cmath>
#include "NonCopyable.h"

class Timer : public NonCopyable {
public:
	// @brief To be called every frame. Updates the frametime and avg fps counter
	void update();

	// @brief Get the static instance of the timer
	static Timer& getTimer() {
		static Timer instance;
		return instance;
	}

	inline float frameTime() { return _frameTime; }
	inline float framesPerSecond() { return _fps; }

private:
	Timer();

	float _frameTime;
	float _fps; // frames per second that are averaged over time

	std::chrono::steady_clock::time_point _currentTime;
	std::chrono::steady_clock::time_point _newTime;
};
