#pragma once

#include <chrono>

// A simple timer, when constructed, begins counting
// Use GetElapsed to retrieve the time elapsed since instantiation
/*
	E.g.
	Timer t;

	SomeExpensiveFunction();

	float time = t.GetElapsed( Timer::Milliseconds );
*/
class Timer final
{
public:
	enum TimeUnit
	{
		Seconds, Milliseconds, Microseconds
	};

	Timer();
	~Timer() = default;

	double GetElapsed( const TimeUnit& tu = Seconds ) const;
	double GetElapsedAndReset( const TimeUnit& tu = Seconds );
	void Reset();

private:
	std::chrono::time_point<std::chrono::system_clock> start;
};
