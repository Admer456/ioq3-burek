#pragma once

namespace ClientEntities
{
	class Weapon_Fists : public BaseClientWeapon
	{
	public:
		Weapon_Fists() = default;

		WeaponInfo GetWeaponInfo() override;

		void Precache() override;
		
		void OnIdle() override;
		void OnPrimaryFire() override;
		void OnSecondaryFire() override;

		void CheckHit();

	private:
		animHandle	animIdle{};
		animHandle	animAttackLeft{};
		animHandle	animAttackRight{};
	};
}
