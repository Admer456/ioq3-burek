#pragma once

// Yes, gibs are particles. Deal with it

enum BreakMaterialTypes
{
	Material_Glass,
	Material_Wood,
	Material_Concrete,
	Material_Metal,

	Material_Max
};

namespace Particles
{
	class BaseParticle;

	class BaseGib : public BaseParticle
	{
	public:
		// Gib spawning utility
		static void CreateGib( qhandle_t model, Vector origin, Vector velocity, int materialType, sfxHandle_t sound, bool expensiveCollision = false );

		static constexpr uint32_t GibFlag_Shrink = 1 << 29U;
		static constexpr uint32_t GibFlag_DidntMoveLastFrame = 1 << 30U;
		static constexpr uint32_t GibFlag_Sleeping = 1 << 31U;

		static void ParticleForceAffect( BaseParticle* self, const float& time );

		static void	ParticleUpdate( BaseParticle* self, const float& time );
		static void	ParticleRender( BaseParticle* self, const float& time );
	};
}
