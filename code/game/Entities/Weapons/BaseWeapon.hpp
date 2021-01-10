#pragma once

#include "../shared/Weapons/WeaponIDs.hpp"

namespace Entities
{
	enum WeaponFlags : uint16_t
	{
		WFNone,
		WFReload,
		WFHolster,
		WFDraw,
		WFAkimbo
	};

	enum WeaponEvent : uint16_t
	{
		WE_DoPrimaryAttack,
		WE_DoSecondaryAttack,
		WE_DoTertiaryAttack,
		WE_DoReload,
		WE_DoDraw,
		WE_DoHolster,
	};

	class BasePlayer;

	class BaseWeapon : public BaseEntity
	{
	public:
		struct WeaponInfo
		{
			const char* worldModel;
			int weaponID;
		};
		virtual WeaponInfo GetWeaponInfo() = 0;
		virtual uint16_t GetWeaponFlags() = 0;

		DeclareEntityClass();

		void			Spawn() override;
		void			WeaponTouch( IEntity* other, trace_t* trace );

		BasePlayer*		GetPlayer();

		virtual void	PrimaryAttack() {}
		virtual void	SecondaryAttack() {}
		virtual void	TertiaryAttack() {}

		virtual void	Reload() {}

		virtual void	Draw() {}
		virtual void	Holster() {}

	protected:
		BasePlayer*		player;
	};
}