#pragma once

namespace ClientEntities
{
	class Weapon_Test2 : public BaseClientWeapon
	{
	public:
		WeaponInfo GetWeaponInfo() override;

		void OnPrimaryFire() override;
	};
}
