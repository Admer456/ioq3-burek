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

		void		Precache() override;

		void		PrimaryAttack() override;
		void		SecondaryAttack() override;

		static inline sfxHandle_t PunchSounds[6]{};

	protected:
		void		Swing( float damage );
	};
}
