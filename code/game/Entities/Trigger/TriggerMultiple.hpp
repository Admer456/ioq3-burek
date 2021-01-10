#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;
	class BaseTrigger;

	class TriggerMultiple : public BaseTrigger
	{
	public:
		DeclareEntityClass();

		void Spawn() override;

		void ResetThink();

		void MultipleTouch( IEntity* other, trace_t* trace );
		void MultipleUse( IEntity* activator, IEntity* caller, float value );
	
	private:
		float resetDelay{ 0 };
	};
}
