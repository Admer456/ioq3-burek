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

		BaseClientWeapon() = default;

		virtual WeaponInfo GetWeaponInfo() = 0;
		virtual RenderEntity& GetRenderEntity()
		{
			return renderEntity;
		}

		virtual void	Precache();

		virtual void	WeaponFrame();

		virtual void	OnIdle() {}

		virtual void	OnPrimaryFire() {}
		virtual void	OnSecondaryFire() {}
		virtual void	OnTertiaryFire() {}

		virtual void	OnReload() {}

		virtual void	OnDraw() {}
		virtual void	OnHolster() {}

	protected:
		RenderEntity	renderEntity;

		// "Timers"
		float			nextPrimary{};
		float			nextSecondary{};
		float			nextTertiary{};
		float			nextIdle{};
		float			nextReload{};
	};
}