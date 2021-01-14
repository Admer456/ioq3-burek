#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;
	class BaseWeapon;

	class Weapon_Fists final : public BaseWeapon
	{
	public:
		DeclareEntityClass();

		WeaponInfo	GetWeaponInfo() override;
		uint16_t	GetWeaponFlags() { return WFNone; }

		void		PrimaryAttack() override;
	};
}
