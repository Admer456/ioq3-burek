#include "cg_local.hpp"
#include "Events/ComplexEventHandler.hpp"

Client client;

Client::Client()
{
	complexEventHandler = new ComplexEventHandler();
}

ClientEntities::BaseClientWeapon* Client::GetCurrentWeapon()
{
	ClientEntities::BaseClientWeapon* weapon = gWeapons[cg.snap->ps.weapon];
	if ( weapon )
		return weapon;

	return nullptr;
}

void Client::ParseComplexEvent( int id, centity_t* cent, Vector position )
{
	if ( !complexEventHandler->ParseComplexEvent( id, cent, position ) )
		CG_Error( "Unknown complex event %i, from ent %i\n", id, cent->currentState.number );
}

Client* GetClient()
{
	return &client;
}
