#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BaseEntity.hpp"
#include "../BasePlayer.hpp"
#include "BaseWeapon.hpp"

#include "Weapon_Fists.hpp"

using namespace Entities;

DefineEntityClass_NoMapSpawn( Weapon_Fists, BaseWeapon );

BaseWeapon::WeaponInfo Weapon_Fists::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.worldModel = "models/hands/v_hands.iqm";
	wi.weaponID = WeaponID_Fists;

	return wi;
}

void Weapon_Fists::PrimaryAttack()
{
	engine->Print( "sw\n" );
}
