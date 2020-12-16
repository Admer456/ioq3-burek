#pragma once

namespace ClientEntities
{
	class BaseClientWeapon
	{
	public:
		struct WeaponInfo
		{
			const char* viewModel;
			const char* name;
		};

		virtual void WeaponFrame();

		virtual void OnPrimaryFire();
		virtual void OnSecondaryFire();
		virtual void OnTertiaryFire();

		virtual void OnReload();

		virtual void OnDraw();
		virtual void OnHolster();
	};
}