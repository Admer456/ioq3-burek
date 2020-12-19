#include "Maths/Vector.hpp"
#include "cg_local.hpp"
#include "WeaponFactory.hpp"
#include "BaseClientWeapon.hpp"

using namespace ClientEntities;

void BaseClientWeapon::Precache()
{
	WeaponInfo wi = GetWeaponInfo();

	renderEntity.hModel = trap_R_RegisterModel( wi.viewModel );
}

void BaseClientWeapon::WeaponFrame()
{
	renderEntity.oldframe = renderEntity.frame;
	renderEntity.frame++;

	if ( renderEntity.frame > 200 )
		renderEntity.frame = 0;
}
