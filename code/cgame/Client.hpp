#pragma once

class ComplexEventHandler;
class VegetationSystem;
class LightManager;
class ParticleManager;

class Client final
{
public:
	Client();
	~Client();

	// Updates all systems for the given moment
	void		Update();
	// Called BEFORE every level change or subsystem restart
	void		PreReload();
	// Called AFTER every level change or subsystem restart
	void		PostReload();
	
	// Before all HUD elements are drawn
	void		PreDraw2D();
	// After all HUD elements have been drawn
	void		PostDraw2D();

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
	// Is the client currently paused?
	bool		IsPaused();
	// Mouse coordinates
	Vector				GetMouseXY();
	void				SetMouseXY( int x, int y );

	// Subsystem getters

	// Gets the local player's view
	ClientView* GetView();
	// Gets the handler for CE_ events
	ComplexEventHandler* GetEventHandler();
	// Gets the vegetation system
	VegetationSystem* GetVegetationSystem();
	// Gets the light manager
	LightManager* GetLightManager();
	// Gets the particle manager
	ParticleManager* GetParticleManager();

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
	VegetationSystem* vegetationSystem{ nullptr };
	LightManager* lightManager{ nullptr };
	ParticleManager* particleManager{ nullptr };

	Vector				mouseCoordinates{ Vector::Zero };

	// Model animation data for game models
	std::vector<Assets::ModelAnimation> anims[MAX_MODELS];
};

extern Client* GetClient();
