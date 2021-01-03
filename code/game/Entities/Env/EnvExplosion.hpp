#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class EnvExplosion : public BaseQuakeEntity
	{
	public:
		constexpr static int SF_Repeatable = 1 << 0;
		constexpr static int SF_NoDamage = 1 << 1;

		void Spawn() override;

		void Use( IEntity* activator, IEntity* caller, float value ) override;

		void Explode();

		// TODO: move to BaseQuakeEntity or utils
		void RadiusDamage( float damageAmount, float radius );
	
	protected:
		bool exploded{ false };
		float explosionDamage{ 0.0f };
		float explosionRadius{ 0.0f };

		// TODO: move to BaseQuakeEntity
		BaseQuakeEntity* activator{ nullptr };
	};
}
