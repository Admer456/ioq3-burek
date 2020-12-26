#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	// func_toggle, turns itself on/off when triggered
	class FuncToggle final : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		constexpr static int SF_StartOff = 1;

		void		Spawn() override;

		void		ToggleUse( IEntity* activator, IEntity* caller, float value );

	private:
		uint16_t	originalModelIndex{ 0 };
		bool		visible{ true };
	};
}
