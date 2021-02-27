#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class FuncAttach : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void PostSpawn() override;

		void Think() override;
		void Use( IEntity* activator, IEntity* caller, float value ) override;

	protected:
		IEntity* attached{ nullptr };
		IEntity* attachTo{ nullptr };
	};
}
