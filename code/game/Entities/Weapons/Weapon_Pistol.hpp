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

		void		Precache() override;

		void		Idle() override;
		void		PrimaryAttack() override;
		void		Reload() override;

		static inline qhandle_t BulletHoleDecals[6] = {};

	private:
		void		Shoot();
		void		TestThroughWall( trace_t* tr, Vector direction, float damage );
		void		AddBulletHole( Vector planeNormal, Vector bulletDirection, Vector origin, bool glass = false );

		int			testThroughWallCounter{ 0 };
		bool		fireHeld{ false };
	};
}