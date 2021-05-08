#include "cg_local.hpp"
#include "Particles/BaseParticle.hpp"
#include "ParticleManager.hpp"

ParticleManager::ParticleForce::ParticleForce( Vector origin, float force, float falloff, float turbulence, float life )
{
	forceOrigin = origin;
	forcePower = force;
	forceFalloff = falloff;
	forceTurbulence = turbulence;
	forceStart = GetClient()->Time();
	forceLife = life;

	active = true;
}

void ParticleManager::ParticleForce::Update( const float& time )
{
	const float timeOfDeath = forceStart + forceLife;

	if ( time > timeOfDeath )
	{
		active = false;
	}
}

float ParticleManager::ParticleForce::GetForce( const Vector& positionOfTurbulence, const bool& decayByRadius ) const
{
	if ( !active )
	{
		return 0.0f;
	}

	const float timeUntilDeath = (forceStart + forceLife) - GetClient()->Time();
	float force = (timeUntilDeath / forceLife) * forcePower;

	// Turbulate it according to the position
	float turbulence = 0;

	// Skip turbulence if this force is a "clean" one
	if ( forceTurbulence != 0.0f )
	{
		turbulence = cosf( positionOfTurbulence.x * 0.3f ) * sinf( positionOfTurbulence.y * 0.34f ) * cosf( positionOfTurbulence.z * 0.37f );
	}

	// Apply
	force += force * turbulence * forceTurbulence;

	if ( decayByRadius )
	{
		float radius = (positionOfTurbulence - forceOrigin).Length();
		force /= (radius * radius) * forceFalloff;
		force *= 600.0f; // some sorta gravity constant - my approximation
	}

	return force;
}

uint32_t ParticleManager::AddParticle( Particles::BaseParticle& particle )
{
	for ( int i = 0; i < MaxParticles; i++ )
	{
		auto& p = particles[i];

		if ( !p.active )
		{
			p = particle;
			return i;
		}
	}

	return 1 << 31U;
}

uint32_t ParticleManager::AddForce( ParticleManager::ParticleForce& force )
{
	for ( int i = 0; i < ParticleForce::MaxForces; i++ )
	{
		auto& f = forces[i];

		if ( !f.active )
		{
			Particles::BaseParticle::ForceUpdateAll = true;

			f = force;
			return i;
		}
	}

	return 1 << 31U;
}

void ParticleManager::MemoryUsage()
{
	constexpr int sizeOfParticle = sizeof( Particles::BaseParticle );
	constexpr int sizeOfParticleArray = sizeof( particles );

	CG_Printf( "** ParticleManager **\n" );
	CG_Printf( "** Particle size: %4.3f kB\n", sizeOfParticle / 1024.0f );
	CG_Printf( "** Particle array count: %i particles\n", MaxParticles );
	CG_Printf( "** Particle array size: %4.3f MB\n", sizeOfParticleArray / 1024.0f / 1024.0f );
}

void ParticleManager::Update()
{
	static int forceUpdateCounter = 0;
	const float time = GetClient()->Time();

	// Update forces
	for ( auto& f : forces )
	{
		if ( f.active )
		{
			f.Update( time );
		}
	}

	// Count when to force update gibs and particles
	Particles::BaseParticle::ForceUpdateAll = false;
	if ( ++forceUpdateCounter == 100 )
	{
		forceUpdateCounter = 0;
		Particles::BaseParticle::ForceUpdateAll = true;
	}

	// Run physics for all particles and gibs
	int particleCount = 0;
	for ( auto& p : particles )
	{
		if ( p.active )
		{
			particleCount++;
			p.Update( time );
		}
	}

	//CG_Printf( "PARTICLE COUNT: %i\n", particleCount );
}

void ParticleManager::Render()
{
	const float time = GetClient()->Time();

	for ( auto& p : particles )
	{
		if ( p.active )
		{
			p.Render( time );
		}
	}
}
