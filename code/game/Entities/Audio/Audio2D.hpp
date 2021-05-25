#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class Audio2D : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void			Spawn() override;
		void			Use( IEntity* activator, IEntity* caller, float value ) override;

	protected:
		sfxHandle_t		sound{ 0 };
		bool			played{ false };
	};
}
