#pragma once

// Yes, planes are particles. Deal with it

namespace Particles
{
	class BaseParticle;

	class Airplane : public BaseParticle
	{
	public:
		static void CreateAirplane( Vector origin, float yaw );

		static void	ParticleUpdate( BaseParticle* self, const float& time );
		static void	ParticleRender( BaseParticle* self, const float& time );
	};
}
