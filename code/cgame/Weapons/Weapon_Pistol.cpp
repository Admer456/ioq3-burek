#include "cg_local.hpp"
#include "View/View.hpp"
#include "../shared/Weapons/WeaponIDs.hpp"

#include "Weapon_Pistol.hpp"

using namespace ClientEntities;

DefineWeaponClass( Weapon_Pistol, WeaponID_Pistol );

BaseClientWeapon::WeaponInfo Weapon_Pistol::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.name = "Pistol";
	wi.viewModel = "models/weapons/v_pistol.iqm";
	wi.worldModel = "models/weapons/w_pistol.iqm";

	return wi;
}

void Weapon_Pistol::Precache()
{
	BaseClientWeapon::Precache();

	animIdle = renderEntity.GetAnimByName( "idle" );
	animIdleEmpty = renderEntity.GetAnimByName( "idle_empty" );
	animAttack[0] = renderEntity.GetAnimByName( "shot1" );
	animAttack[1] = renderEntity.GetAnimByName( "shot2" );
	animAttack[2] = renderEntity.GetAnimByName( "shot3" );
	animAttackEmpty = renderEntity.GetAnimByName( "shot_empty" );
	animAttackToEmpty = renderEntity.GetAnimByName( "shot_toempty" );
	animReload = renderEntity.GetAnimByName( "reload" );
	animReloadTactical = renderEntity.GetAnimByName( "reload_tactical" );

	renderEntity.StartAnimation( animIdle, true );
	nextIdle = GetNextAnimTime( animIdle );
}

void Weapon_Pistol::OnIdle()
{
	usercmd_t uc = GetClient()->GetUsercmd();
	animHandle anim = animIdle;

	if ( !(uc.interactionButtons & Interaction_PrimaryAttack) )
	{
		fireHeld = false;
	}

	if ( GetClient()->Time() >= nextIdle )
	{
		int ammoMag = GetAmmoInMag( WeaponID_Pistol );

		if ( !ammoMag )
			anim = animIdleEmpty;

		renderEntity.StartAnimation( anim, true );
		nextIdle = GetNextAnimTime( anim );
	}
}

void Weapon_Pistol::OnPrimaryFire()
{
	if ( GetClient()->Time() < nextPrimary )
		return;

	if ( fireHeld )
		return;

	fireHeld = true;

	int ammoMag = GetAmmoInMag( WeaponID_Pistol );
	animHandle anim = animAttack[0];

	if ( ammoMag > 1 )
	{
		anim = animAttack[ammoMag % 3];
	}
	else if ( ammoMag == 1 )
	{
		anim = animAttackToEmpty;
		nextPrimary = GetNextAnimTime( anim );
	}
	else
	{
		anim = animAttackEmpty;
	}

	if ( anim != animAttackEmpty )
		GetClient()->GetView()->AddPunch( 0.25f, Vector( -1.5f, -0.7f, 0.5f ) );

	renderEntity.StartAnimation( anim, true );
	nextIdle = nextReload = GetNextAnimTime( anim );
}

void Weapon_Pistol::OnReload()
{
	if ( GetClient()->Time() < nextReload )
		return;

	int ammoMag = GetAmmoInMag( WeaponID_Pistol );
	animHandle anim = (ammoMag >= 1) ? animReloadTactical : animReload;

	renderEntity.StartAnimation( anim, true );
	nextReload = nextIdle = nextPrimary = GetNextAnimTime( anim ) - 0.2f;
}
