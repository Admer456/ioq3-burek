#include "cg_local.hpp"
#include "WeaponFactory.hpp"
#include "BaseClientWeapon.hpp"
#include "../shared/Weapons/WeaponIDs.hpp"

#include "Weapon_Test2.hpp"

using namespace ClientEntities;

DefineWeaponClass( Weapon_Test2, WeaponID_Test2 );

BaseClientWeapon::WeaponInfo ClientEntities::Weapon_Test2::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.name = "Another test weapon";
	wi.viewModel = "models/weapons/v_test2.iqm";

	return wi;
}

void Weapon_Test2::OnPrimaryFire()
{
	Com_Printf( "Client weapon: Firing!\n" );
}