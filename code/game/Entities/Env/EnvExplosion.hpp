#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class EnvExplosion : public BaseEntity
	{
	public:
		DeclareEntityClass();

		constexpr static int SF_Repeatable = 1 << 0;
		constexpr static int SF_NoDamage = 1 << 1;

		void Spawn() override;
		void Precache() override;

		void Use( IEntity* activator, IEntity* caller, float value ) override;

		void Explode();

		// TODO: move to BaseEntity or utils
		void RadiusDamage( float damageAmount, float radius );
	
	protected:
		bool exploded{ false };
		float explosionDamage{ 0.0f };
		float explosionRadius{ 0.0f };

		sfxHandle_t explosionSound{ 0U };

		// TODO: move to BaseEntity
		BaseEntity* activator{ nullptr };
	};
}
