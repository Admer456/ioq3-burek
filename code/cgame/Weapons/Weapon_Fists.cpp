#include "cg_local.hpp"
#include "../shared/Weapons/WeaponIDs.hpp"

#include "Weapon_Fists.hpp"

using namespace ClientEntities;

DefineWeaponClass( Weapon_Fists, WeaponID_Fists );

BaseClientWeapon::WeaponInfo Weapon_Fists::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.name = "Fists";
	wi.viewModel = "models/hands/v_hands.iqm";

	return wi;
}

void Weapon_Fists::Precache()
{
	BaseClientWeapon::Precache();

	animIdle = renderEntity.GetAnimByName( "idle" );
	animAttackLeft = renderEntity.GetAnimByName( "attack_left" );
	animAttackRight = renderEntity.GetAnimByName( "attack_right" );

	renderEntity.StartAnimation( animIdle, true );
}

void Weapon_Fists::OnIdle()
{
	renderEntity.StartAnimation( animIdle, true );

	nextIdle = cg.time * 0.001f + renderEntity.GetAnimData( animIdle ).Length();
}

void Weapon_Fists::OnPrimaryFire()
{
	if ( cg.time * 0.001f < nextPrimary )
		return;

	renderEntity.StartAnimation( animAttackLeft, true );

	nextPrimary = nextSecondary = cg.time * 0.001f + 0.5f;
	nextIdle = cg.time * 0.001f + renderEntity.GetAnimData( animAttackLeft ).Length();
}

void Weapon_Fists::OnSecondaryFire()
{
	if ( cg.time * 0.001f < nextSecondary )
		return;

	renderEntity.StartAnimation( animAttackRight, true );

	nextPrimary = nextSecondary = cg.time * 0.001f + 0.7f;
	nextIdle = cg.time * 0.001f + renderEntity.GetAnimData( animAttackRight ).Length();
}
