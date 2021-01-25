#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class LogicTimer : public BaseEntity
	{
	public:
		DeclareEntityClass();

		constexpr static uint32_t SF_StartOn = 1 << 0;
		constexpr static uint32_t SF_Impulse = 1 << 1; // logic_timer is toggled by default, this makes it pulse

		void		Spawn() override;

		void		TimerUse( IEntity* activator, IEntity* caller, float value );
		void		TimerThink();
		float		CalculateDelay();

	protected:
		float		delayMin; // if delayMax > delayMin, then a random
		float		delayMax; // interval will be used

		IEntity*	activator;
	};
}
