#pragma once

class ComplexEventHandler;

class Client final
{
public:
	Client();
	~Client();

	// Retrieves the current weapon carried by the local player
	ClientEntities::BaseClientWeapon* GetCurrentWeapon();

	// Parses complex entity events
	void		ParseComplexEvent( int id, centity_t* cent, Vector position );
	// Strips the extension off modelName, and parses the respective .mcfg file
	void		RegisterModelConfigData( int id, const char* modelName );
	// Gets a list of model animation data for the given game model
	std::vector<Assets::ModelAnimation>& GetAnimationsForModel( qhandle_t modelindex );
	// Returns the current client time
	float		Time() const;

	ClientView* GetView() { return view; }

	static bool IsLocalClient( centity_t* cent );
	static void ExecuteWeaponEvent( int id, centity_t* cent );

private:
	ComplexEventHandler* complexEventHandler{ nullptr };
	ClientView* view{ nullptr };

	// Model animation data for game models
	std::vector<Assets::ModelAnimation> anims[MAX_MODELS];
};

extern Client* GetClient();
