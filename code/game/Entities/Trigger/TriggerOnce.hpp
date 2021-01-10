#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;
	class BaseTrigger;
	
	class TriggerOnce : public BaseTrigger
	{
	public:
		DeclareEntityClass();

		void Spawn() override;

		void OnceTouch( IEntity* other, trace_t* trace );
		void OnceUse( IEntity* activator, IEntity* caller, float value );
	};
}
