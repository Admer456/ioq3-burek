#pragma once

#include "Particles/BaseParticle.hpp"

class ParticleManager final
{
public:
	class ParticleForce final
	{
	public:
		ParticleForce() = default;
		ParticleForce( Vector origin, float force = 500.0f, float falloff = 1.0f, float turbulence = 0.0f, float life = 0.5f );
		~ParticleForce() = default;

		void	Update( const float& time );
		float	GetForce( const Vector& positionOfTurbulence, const bool& decayByRadius = true ) const;

		static constexpr size_t MaxForces = 16U;

		bool	active{ false };
		Vector	forceOrigin{ Vector::Zero };
		float	forcePower{ 0.0f };
		float	forceFalloff{ 0.0f }; // smaller the falloff, bigger the force effect
		float	forceTurbulence{ 0.0f }; // randomly displace the force's effect

		float	forceStart{ 0.0f };
		float	forceLife{ 0.0f };
	};

	template< typename T >
	uint32_t	AddParticle( qhandle_t material, Vector origin, Vector angles, Vector velocity,
						 Vector angularVelocity, float scale, float life )
	{
		Particles::BaseParticle particle;
		particle.Create( material, origin, angles, velocity, angularVelocity, scale, GetClient()->Time(), life, T::ParticleUpdate, T::ParticleRender );
		return AddParticle( particle );
	}

	// Simplified version
	template< typename T >
	uint32_t	AddParticle( qhandle_t material, Vector origin, float scale, float life )
	{
		Particles::BaseParticle particle;
		particle.Create( material, origin, Vector::Zero, Vector::Zero, Vector::Zero, scale, GetClient()->Time(), life, T::ParticleUpdate, T::ParticleRender );
		return AddParticle( particle );
	}

	uint32_t AddParticle( Particles::BaseParticle& particle );
	uint32_t AddForce( ParticleForce& force );

	Particles::BaseParticle& GetParticle( uint32_t particleId )
	{
		return particles[particleId];
	}

	ParticleForce* GetForces()
	{
		return forces;
	}

	void	MemoryUsage();

	void	Update();
	void	Render();

	static constexpr size_t MaxParticles = 16384U;

private:
	Particles::BaseParticle particles[MaxParticles];
	ParticleForce forces[ParticleForce::MaxForces];
};
