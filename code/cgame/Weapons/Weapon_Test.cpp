#include "cg_local.hpp"
#include "WeaponFactory.hpp"
#include "BaseClientWeapon.hpp"
#include "../shared/Weapons/WeaponIDs.hpp"

#include "Weapon_Test.hpp"

using namespace ClientEntities;

DefineWeaponClass( Weapon_Test, WeaponID_Test );

BaseClientWeapon::WeaponInfo ClientEntities::Weapon_Test::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.name = "Test weapon";
	wi.viewModel = "models/weapons/v_test.iqm";

	return wi;
}

void ClientEntities::Weapon_Test::OnPrimaryFire()
{
	Com_Printf( "Client weapon: Firing!\n" );
}
