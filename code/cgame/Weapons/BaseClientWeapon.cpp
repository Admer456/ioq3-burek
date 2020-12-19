#include "Maths/Vector.hpp"
#include "cg_local.hpp"
#include "RenderEntity.hpp"
#include "WeaponFactory.hpp"
#include "BaseClientWeapon.hpp"

using namespace ClientEntities;

void BaseClientWeapon::Precache()
{
	memset( &renderEntity, 0, sizeof( renderEntity ) );

	WeaponInfo wi = GetWeaponInfo();

	renderEntity.hModel = trap_R_RegisterModel( wi.viewModel );
}

void BaseClientWeapon::WeaponFrame()
{
	renderEntity.backlerp = 0;
	renderEntity.frame++;
	//renderEntity.oldframe = 91;

	if ( renderEntity.frame > 91 )
		renderEntity.frame = 0;

	renderEntity.oldframe = renderEntity.frame;

	//if ( renderEntity.oldframe > 91 )
	//	renderEntity.oldframe = 0;
}
