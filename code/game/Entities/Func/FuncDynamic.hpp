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

		void StartAnimation( const char* name );
		void IdleAnimation( const char* name );

	protected:
		qhandle_t actionAnimation{};
		qhandle_t idleAnimation{};
	};
}
