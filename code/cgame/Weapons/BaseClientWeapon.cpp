#include "Maths/Vector.hpp"
#include "cg_local.hpp"

using namespace ClientEntities;

centity_t* BaseClientWeapon::currentPlayer = nullptr;

// ===================
// BaseClientWeapon::GetNextAnimTime
// ===================
float BaseClientWeapon::GetNextAnimTime( const animHandle& animation )
{
	return GetClient()->Time() + renderEntity.GetAnimData( animation ).Length();
}

// ===================
// BaseClientWeapon::GetAmmoInMag
// ===================
int BaseClientWeapon::GetAmmoInMag( int id )
{
	return cg.predictedPlayerState.ammoMag[id];
}

// ===================
// BaseClientWeapon::GetAmmo
// ===================
int BaseClientWeapon::GetAmmo( int id )
{
	return cg.predictedPlayerState.ammo[id];
}

// ===================
// BaseClientWeapon::Precache
// ===================
void BaseClientWeapon::Precache()
{
	memset( &renderEntity, 0, sizeof( renderEntity ) );

	WeaponInfo wi = GetWeaponInfo();

	renderEntity = RenderEntity( wi.viewModel );
	wi.viewModelHandle = renderEntity.GetRefEntity().hModel;
}

// ===================
// BaseClientWeapon::WeaponFrame
// ===================
void BaseClientWeapon::WeaponFrame()
{
	if ( !Client::IsLocalClient( currentPlayer ) )
		return;

	renderEntity.Update( cg.frametime * 0.001f );

	OnIdle();
}
