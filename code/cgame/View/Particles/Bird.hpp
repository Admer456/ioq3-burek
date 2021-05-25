#pragma once

// Yes, birds are particles. Deal with it

namespace Particles
{
	class BaseParticle;

	class Bird : public BaseParticle
	{
	public:
		static void CreateBird( Vector origin, Vector birthPlace );

		static void ParticleUpdate( BaseParticle* self, const float& time );
		static void ParticleRender( BaseParticle* self, const float& time );
	};
}
