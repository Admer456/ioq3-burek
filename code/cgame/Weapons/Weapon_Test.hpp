#pragma once

namespace ClientEntities
{
	class Weapon_Test : public BaseClientWeapon
	{
	public:
		WeaponInfo GetWeaponInfo() override;

		void OnPrimaryFire() override;
	};
}
