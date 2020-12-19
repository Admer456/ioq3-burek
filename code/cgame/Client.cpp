#include "cg_local.hpp"

Client client;

ClientEntities::BaseClientWeapon* Client::GetCurrentWeapon()
{
	ClientEntities::BaseClientWeapon* weapon = gWeapons[cg.snap->ps.weapon];
	if ( weapon )
		return weapon;

	return nullptr;
}

Client* GetClient()
{
	return &client;
}
