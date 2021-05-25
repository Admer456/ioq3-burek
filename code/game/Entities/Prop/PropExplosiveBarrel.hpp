#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class PropExplosiveBarrel : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
		void Precache() override;

		void Use( IEntity* activator, IEntity* caller, float value ) override;
		void TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) override;
	
	protected:
		void Explode();

		// TODO: move to BaseEntity or utils
		void RadiusDamage( float damageAmount, float radius );

		// TODO: move to BaseEntity
		BaseEntity* activator{ nullptr };

		qhandle_t gibModels[3]{};
		sfxHandle_t explosionSound{};
	};
}
