#include "utility/timer.h"

void Timer::initialize() {
    this->frame_time   = 0.0f;
    this->fps          = 0.0f;
    this->current_time = std::chrono::steady_clock::now();
}

void Timer::update() {
	new_time = std::chrono::steady_clock::now();

	frame_time = std::chrono::duration<float>(new_time - current_time).count();

    // Average the fps with the past calculation of it with some smoothing value
	float naive_fps = 1.0f / frame_time;
	fps = (fps * fps_smoothing) + (naive_fps * (1.0f - fps_smoothing));

	current_time = new_time;
}
