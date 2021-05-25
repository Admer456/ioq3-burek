#pragma once

#include "../shared/Weapons/WeaponIDs.hpp"

namespace Entities
{
	enum WeaponFlags : uint16_t
	{
		WFNone,
		WFReload, // this weapon can reload
		WFHolster, // this weapon can holster
		WFDraw, // this weapon can draw
		WFAkimbo // this weapon can be picked up as akimbo
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
			const char* viewModel;
			const char* worldModel;
			int weaponID;
		};
		virtual WeaponInfo GetWeaponInfo() = 0;
		virtual uint16_t GetWeaponFlags() = 0;

		inline static sfxHandle_t PickupSound{};

		DeclareEntityClass();

		void			Spawn() override;
		void			WeaponTouch( IEntity* other, trace_t* trace );
		void			WeaponUse( IEntity* activator, IEntity* caller, float value );

		uint32_t		ObjectFlags() override { return OF_ImpulseUse; }

		BasePlayer*		GetPlayer();
		void			SetPlayer( BasePlayer* player );

		virtual void	WeaponFrame();

		virtual void	Idle() {}
		virtual void	PrimaryAttack() {}
		virtual void	SecondaryAttack() {}
		virtual void	TertiaryAttack() {}

		virtual void	Reload() {}

		virtual void	Draw() {}
		virtual void	Holster() {}

	protected:
		BasePlayer*		player;

		// "Timers"
		float			nextPrimary{};
		float			nextSecondary{};
		float			nextTertiary{};
		float			nextIdle{};
		float			nextReload{};
	};
}