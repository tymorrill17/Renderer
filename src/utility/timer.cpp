#include "utility/timer.h"

Timer::Timer() :
	_frameTime(0.0f),
	_fps(0.0f),
	_currentTime(std::chrono::steady_clock::now()) {}

void Timer::update() {
	// get the current time
	_newTime = std::chrono::steady_clock::now();

	// get the duration
	_frameTime = std::chrono::duration<float>(_newTime - _currentTime).count();

    // Average the fps with the past calculation of it with some smoothing value
	float naivefps = 1.0f / _frameTime;
	float smoothing = 0.9f;
	_fps = (_fps * smoothing) + (naivefps * (1.0f - smoothing));

	_currentTime = _newTime;
}
