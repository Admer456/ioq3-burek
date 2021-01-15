#include "Maths/Vector.hpp"
#include "cg_local.hpp"

using namespace ClientEntities;

void BaseClientWeapon::Precache()
{
	memset( &renderEntity, 0, sizeof( renderEntity ) );

	WeaponInfo wi = GetWeaponInfo();

	renderEntity = RenderEntity( wi.viewModel );
}

void BaseClientWeapon::WeaponFrame()
{
	renderEntity.Update( cg.frametime * 0.001f );

	if ( cg.time * 0.001f > nextIdle )
		OnIdle();
}
