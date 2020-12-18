#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BaseEntity.hpp"
#include "../BasePlayer.hpp"
#include "BaseWeapon.hpp"

#include "Weapon_Test.hpp"

using namespace Entities;

DefineEntityClass( "weapon_test", Weapon_Test, BaseWeapon );

Weapon_Test::WeaponInfo Weapon_Test::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.worldModel = "models/weapons/w_test.iqm";
	wi.weaponID = WeaponID_Test;

	return wi;
}

void Weapon_Test::PrimaryAttack()
{
	engine->Print( "Server Weapon_Test::PrimaryFire\n" );
}
