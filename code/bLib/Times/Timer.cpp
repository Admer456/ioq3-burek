#include "Timer.hpp"

using namespace std::chrono;

Timer::Timer()
{
	Reset();
}

double Timer::GetElapsed( const TimeUnit& tu ) const
{
	const auto now = system_clock::now();
	const duration<double> d = (now - start);
	const double seconds = d.count();

	switch ( tu )
	{
	default:
	case Timer::Seconds: return seconds;
	case Timer::Milliseconds: return seconds * 1000.0;
	case Timer::Microseconds: return seconds * 1000000.0;
	}
}

double Timer::GetElapsedAndReset( const TimeUnit& tu )
{
	double elapsed = GetElapsed( tu );
	Reset();

	return elapsed;
}

void Timer::Reset()
{
	start = system_clock::now();
}
