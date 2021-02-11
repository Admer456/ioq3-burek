#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;
	class BaseWeapon;

	class Weapon_Pistol final : public BaseWeapon
	{
	public:
		DeclareEntityClass();

		WeaponInfo	GetWeaponInfo() override;
		uint16_t	GetWeaponFlags() { return WFReload; }

		void		Idle() override;

		void		PrimaryAttack() override;
		//void		SecondaryAttack() override;

		void		Reload() override;

	private:
		void		Shoot();

		bool		fireHeld{ false };
	};
}