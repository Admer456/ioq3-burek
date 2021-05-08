#pragma once

namespace Particles
{
	class BaseParticle;

	class Smoke : public BaseParticle
	{
	public:
		static constexpr uint32_t SmokeFlag_FadingIn = 1 << 31U;

		// Smoke spawning utility
		static void CreateSmoke( qhandle_t smokeSprite, Vector origin, Vector velocity, 
								 Vector tintColour, float scale = 1.0f, float fadeInTime = 1.5f, 
								 float fadeOutTime = 5.0f, float life = 10.0f, bool expensiveCollision = false );

		static void	ForceEffect( BaseParticle* self, const float& time );

		static void	ParticleUpdate( BaseParticle* self, const float& time );
		static void	ParticleRender( BaseParticle* self, const float& time );
	};
}
