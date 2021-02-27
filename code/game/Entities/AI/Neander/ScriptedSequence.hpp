#pragma once

namespace Entities
{
	class ScriptedSequence final : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
		void PostSpawn() override;

		void SequenceUse( IEntity* activator, IEntity* caller, float value );

	protected:
		BaseEntity* targetMonster;
	};
}
