
#pragma once
#include <chrono> 

using namespace std::chrono_literals;

class FpsMeter {
public:

	FpsMeter(std::chrono::duration<double> _interval);
	double get_fps(void);
	bool is_updated(void);
	void update(void);
	void reset(void);
	void set_interval(std::chrono::duration<double> _interval);
private:
	double fps;
	std::chrono::time_point<std::chrono::steady_clock> last_time;
	std::chrono::duration<double> interval;
	size_t frame_count;
	bool updated;
};