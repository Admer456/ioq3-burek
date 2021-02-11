#pragma once

class ComplexEventHandler;

class Client final
{
public:
	Client();
	~Client();

	// Updates all systems for the given moment
	void		Update();

	// Retrieves the current weapon carried by the local player
	ClientEntities::BaseClientWeapon* GetCurrentWeapon();
	// Parses CE_ events
	void		ParseComplexEvent( int id, centity_t* cent, Vector position );
	// Strips the extension off modelName, and parses the respective .mcfg file
	void		RegisterModelConfigData( int id, const char* modelName );
	
	// Gets a list of model animation data for the given game model
	std::vector<Assets::ModelAnimation>& GetAnimationsForModel( qhandle_t modelindex );
	// Returns the current client time
	float		Time() const;
	// Gets the latest usercmd
	usercmd_t	GetUsercmd() const;

	// Subsystem getters

	// Gets the local player's view
	ClientView* GetView();
	// Gets the handler for CE_ events
	ComplexEventHandler* GetEventHandler();

	// Is this entity the local client?
	static bool IsLocalClient( centity_t* cent );
	// Executes a weapon event for a given predicted entity
	static void ExecuteWeaponEvent( int id, centity_t* cent );

	// Loads the map's .mus file, if it exists
	void		InitDynamicMusic( const char* mapName );
	// Fades out the current song and fades in the new one if necessary
	// label = the left name in .mus entries
	void		UpdateDynamicMusic( const char* label );

private:
	ComplexEventHandler* complexEventHandler{ nullptr };
	ClientView* view{ nullptr };

	// Model animation data for game models
	std::vector<Assets::ModelAnimation> anims[MAX_MODELS];
};

extern Client* GetClient();
