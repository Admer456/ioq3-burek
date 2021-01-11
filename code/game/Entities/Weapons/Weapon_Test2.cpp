#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BaseEntity.hpp"
#include "../BasePlayer.hpp"
#include "BaseWeapon.hpp"

#include "Weapon_Test2.hpp"

using namespace Entities;

DefineEntityClass( "weapon_test2", Weapon_Test2, BaseWeapon );

Weapon_Test2::WeaponInfo Weapon_Test2::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.worldModel = "models/weapons/w_test.iqm";
	wi.weaponID = WeaponID_Test2;

	return wi;
}

void Weapon_Test2::PrimaryAttack()
{

}
