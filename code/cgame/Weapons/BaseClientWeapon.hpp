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

		refEntity_t		GetRenderModel() { return renderEntity; }
		virtual WeaponInfo GetWeaponInfo() = 0;

		virtual void	Init();

		virtual void	WeaponFrame();

		virtual void	OnPrimaryFire() {}
		virtual void	OnSecondaryFire() {}
		virtual void	OnTertiaryFire() {}

		virtual void	OnReload() {}

		virtual void	OnDraw() {}
		virtual void	OnHolster() {}

	protected:
		refEntity_t		renderEntity;
	};
}