#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class FuncDynamic : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;

		void Use( IEntity* activator, IEntity* caller, float value ) override;
		void AnimationThink();

		void StartAnimation( const char* name );
		void IdleAnimation( const char* name );

	protected:
		animHandle actionAnimation{};
		animHandle idleAnimation{};
	};
}
