#pragma once

class VegetationInstance;
class KeyValueLibrary;

class VegetationSystem final
{
public:
	// Loads all vegetation instances from the BSP
	void		Init();
	// Clears the vegetation instance list
	void		Clear();
	// Reloads all vegetation instances from the BSP
	void		Reload();
	// Renders all vegetation instances
	void		Render();
	// Adds a vegetation instance to the VI list
	void		Add( const VegetationInstance& instance );
	// Updates some local variables etc.
	void		Update();

	constexpr static size_t MaxVegetationInstances = 16384;

	static Vector playerEyePosition;
private:
	bool		ParseKeyvalues();

	VegetationInstance instances[MaxVegetationInstances];
	std::vector<KeyValueLibrary> libraries;
};
