#include "Maths/Vector.hpp"
#include "cg_local.hpp"
#include "WeaponFactory.hpp"
#include "BaseClientWeapon.hpp"

using namespace ClientEntities;

void BaseClientWeapon::Init()
{
	WeaponInfo wi = GetWeaponInfo();

	//renderEntity.hModel = trap_R_RegisterModel( wi.viewModel );
}

void BaseClientWeapon::WeaponFrame()
{

}
