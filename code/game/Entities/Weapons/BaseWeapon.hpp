#pragma once

namespace Entities
{
	class BasePlayer;

	class BaseWeapon : public BaseQuakeEntity
	{
	public:
		struct WeaponInfo
		{
			const char* worldModel;
			const char* viewModel;
		};
		virtual WeaponInfo GetWeaponInfo();

		DeclareEntityClass();

		void		Spawn() override;
		void		WeaponTouch( IEntity* other, trace_t* trace );

		BasePlayer* GetPlayer();

		virtual void PrimaryFire();
		virtual void SecondaryFire();
		virtual void TertiaryFire();

		virtual void Reload();

		virtual void Draw();
		virtual void Holster();

	protected:
		BasePlayer* player;
	};
}