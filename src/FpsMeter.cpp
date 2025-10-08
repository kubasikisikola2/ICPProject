#include "FpsMeter.hpp"

FpsMeter::FpsMeter(std::chrono::duration<double> _interval = 1.0s){
	interval = _interval;
	fps = 0.0;
	last_time = std::chrono::steady_clock::now();
	frame_count = 0;
	updated = false;
}

void FpsMeter::update(void) {
	frame_count++;

	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<double> delta = now - last_time;

	if (delta > interval) {
		fps = static_cast<double>(frame_count) / delta.count();
		frame_count = 0;
		last_time = now;
		updated = true;
	}
	else {
		updated = false;
	}
}

void FpsMeter::reset(void) {
	last_time = std::chrono::steady_clock::now();
	frame_count = 0;
	updated = false;
}

void FpsMeter::set_interval(std::chrono::duration<double> _interval) {
	interval = _interval;
}

double FpsMeter::get_fps(void) {
	return fps;
}

bool FpsMeter::is_updated(void) {
	return updated;
}