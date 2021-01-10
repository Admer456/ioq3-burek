#pragma once

typedef struct centity_s centity_t;
class ComplexEventHandler;

class Client final
{
public:
	Client();

	// Retrieves the current weapon carried by the local player
	ClientEntities::BaseClientWeapon* GetCurrentWeapon();

	// Parses complex entity events
	void ParseComplexEvent( int id, centity_t* cent, Vector position );

private:
	ComplexEventHandler* complexEventHandler;
};

extern Client* GetClient();
