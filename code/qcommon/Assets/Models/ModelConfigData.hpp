#pragma once

#include <vector>

namespace Assets
{
	class ModelAnimation;

	// Collects data from a .mcfg file
	// Right now, it only parses animation data,
	// but in the future, it'll support hitboxes, frame events etc.
	class ModelConfigData final
	{
	public:
		static std::vector<ModelAnimation> GetAnimations( const char* modelFile );
		static int GetNumAnimations( const char* modelFile );
	};
}
