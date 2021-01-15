#include "../../q_shared.hpp"

using namespace Assets;

ModelAnimation::ModelAnimation()
{
	memset( name, 0, sizeof( name ) );
}

float ModelAnimation::Length()
{
	return static_cast<float>(numFrames) / (1000.0f / frameLerp);
}
