#pragma once

using animHandle = unsigned short;
constexpr animHandle AnimHandleNotFound = 0xffff;

namespace Assets
{
	class ModelAnimation
	{
	public:
		ModelAnimation();

		// Animation length in seconds
		float	Length();

		char	name[32];			// So we can look up by names
		short	firstFrame{ 0 };
		short	numFrames{ 0 };
		short	loopFrames{ 0 };	// 0 to numFrames
		short	frameLerp{ 0 };		// msec between frames
		short	initialLerp{ 0 };	// msec to get to first frame
		bool	reversed{ false };	// True if animation is reversed

		constexpr static size_t MaxAnimations = 0xffff;
	};
}
