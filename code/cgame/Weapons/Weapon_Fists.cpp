#include "cg_local.hpp"
#include "View/View.hpp"
#include "../shared/Weapons/WeaponIDs.hpp"

#include "Weapon_Fists.hpp"

using namespace ClientEntities;

DefineWeaponClass( Weapon_Fists, WeaponID_Fists );

BaseClientWeapon::WeaponInfo Weapon_Fists::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.name = "Fists";
	wi.viewModel = "models/hands/v_hands.iqm";
	wi.worldModel = nullptr;
	wi.worldModelHandle = 0;

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
	if ( !Client::IsLocalClient( currentPlayer ) )
		return;

	renderEntity.StartAnimation( animIdle, true );

	nextIdle = cg.time * 0.001f + renderEntity.GetAnimData( animIdle ).Length();
}

void Weapon_Fists::OnPrimaryFire()
{
	if ( cg.time * 0.001f < nextPrimary )
		return;

	if ( Client::IsLocalClient( currentPlayer ) )
	{
		CheckHit();

		renderEntity.StartAnimation( animAttackLeft, true );
		nextPrimary = nextSecondary = cg.time * 0.001f + 0.4f;
		nextIdle = cg.time * 0.001f + renderEntity.GetAnimData( animAttackLeft ).Length();
	}
}

void Weapon_Fists::OnSecondaryFire()
{
	if ( cg.time * 0.001f < nextSecondary )
		return;

	if ( Client::IsLocalClient( currentPlayer ) )
	{
		CheckHit( true );
		
		renderEntity.StartAnimation( animAttackRight, true );
		nextPrimary = nextSecondary = cg.time * 0.001f + 1.0f;
		nextIdle = cg.time * 0.001f + renderEntity.GetAnimData( animAttackRight ).Length();
	}
}

void Weapon_Fists::CheckHit( bool right )
{
	trace_t tr;
	Vector forward;
	Vector start, end;

	Vector::AngleVectors( cg.refdefViewAngles, &forward, nullptr, nullptr );
	start = Vector( cg.predictedPlayerState.origin ) + Vector( 0, 0, cg.predictedPlayerState.viewheight );
	end = start + forward * 64.0f;

	CG_Trace( &tr, start, nullptr, nullptr, end, cg.predictedPlayerEntity.currentState.number, MASK_SHOT );

	if ( tr.fraction == 1.0f )
		return;

	Vector punchShake;

	if ( !right )
		punchShake = (cg.time % 2) ? Vector( 0.5, 0.5, 0 ) : Vector( -0.3, -0.65, 0 );
	else 
		punchShake = Vector( 0.1, 0.1, 0.8 );

	GetClient()->GetView()->AddShake( 10.0f, 0.3f, punchShake );
}
