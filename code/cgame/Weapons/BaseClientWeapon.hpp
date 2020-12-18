#pragma once

namespace ClientEntities
{
	class BaseClientWeapon
	{
	public:
		struct WeaponInfo
		{
			const char* viewModel;
			qhandle_t viewModelHandle;

			const char* name;
		};

		virtual WeaponInfo GetWeaponInfo() = 0;

		virtual void	Init();

		virtual void	WeaponFrame();

		virtual void	OnPrimaryFire() {}
		virtual void	OnSecondaryFire() {}
		virtual void	OnTertiaryFire() {}

		virtual void	OnReload() {}

		virtual void	OnDraw() {}
		virtual void	OnHolster() {}
	};
}