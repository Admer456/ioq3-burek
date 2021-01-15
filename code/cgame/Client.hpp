#pragma once

class ComplexEventHandler;

class Client final
{
public:
	Client();

	// Retrieves the current weapon carried by the local player
	ClientEntities::BaseClientWeapon* GetCurrentWeapon();

	// Parses complex entity events
	void ParseComplexEvent( int id, centity_t* cent, Vector position );

	void RegisterModelConfigData( int id, const char* modelName );

	std::vector<Assets::ModelAnimation>& GetAnimationsForModel( qhandle_t modelindex );

private:
	ComplexEventHandler* complexEventHandler;

	// Model animation data for game models
	std::vector<Assets::ModelAnimation> anims[MAX_MODELS];
};

extern Client* GetClient();
