#pragma once

namespace Entities
{
	class IEntity;
}

namespace Components
{
	class IComponent
	{
	public:
		virtual void	SetParent( Entities::IEntity* parent ) { this->parent = parent; }
		virtual Entities::IEntity* GetParent() { return parent; }

		virtual void	Spawn() = 0;
		virtual void	Think() = 0;

	protected:
		Entities::IEntity* parent;
	};

	class TestComponent : public IComponent
	{
	public:

		void			Spawn() override {}
		void			Think() override {}
	};
}
