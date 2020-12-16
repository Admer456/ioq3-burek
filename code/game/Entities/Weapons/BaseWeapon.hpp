#pragma once

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

	class BasePlayer;

	class BaseWeapon : public BaseQuakeEntity
	{
	public:
		struct WeaponInfo
		{
			const char* worldModel;
		};
		virtual WeaponInfo GetWeaponInfo() = 0;
		virtual uint16_t GetWeaponFlags() = 0;

		DeclareEntityClass();

		void		Spawn() override;
		void		WeaponTouch( IEntity* other, trace_t* trace );

		BasePlayer* GetPlayer();

		virtual void PrimaryFire() {}
		virtual void SecondaryFire() {}
		virtual void TertiaryFire() {}

		virtual void Reload() {}

		virtual void Draw() {}
		virtual void Holster() {}

	protected:
		BasePlayer* player;
	};
}