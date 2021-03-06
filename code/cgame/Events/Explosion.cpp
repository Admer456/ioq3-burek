#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"
#include "View/ParticleManager.hpp"
#include "View/Particles/BaseGib.hpp"
#include "View/Particles/Smoke.hpp"

class Explosion final : public EventParser
{
public:
	Explosion( int ev );

	void RegisterAssets() override
	{
		explosionMaterials[0] = trap_R_RegisterShader( "sprites/explo1" );
		explosionMaterials[1] = trap_R_RegisterShader( "sprites/explo2" );
		explosionMaterials[2] = trap_R_RegisterShader( "sprites/explo3" );

		smokeMaterial = trap_R_RegisterShader( "sprites/smoke1" );
		smokeLength = trap_R_AnimationLength( smokeMaterial );

		explosionSounds[0] = trap_S_RegisterSound( "sound/debris/explode1.wav", false );
		explosionSounds[1] = trap_S_RegisterSound( "sound/debris/explode2.wav", false );
		explosionSounds[2] = trap_S_RegisterSound( "sound/debris/explode3.wav", false );
	}

	Vector CRandomVector( Vector max )
	{
		return Vector( crandom() * max.x, crandom() * max.y, crandom() * max.z );
	}

	void Parse( centity_t* cent, Vector position ) override
	{
		EventData ed( cent->currentState );

		localEntity_t* le; // explosion
		localEntity_t* sle; // smoke
		refEntity_t* re;

		float radius = ed.fparm;
		qhandle_t sprite = ed.parm;
		Vector direction = ed.vparm;
		sfxHandle_t sound = ed.sound;

		sprite = cgs.gameMaterials[sprite];
		sound = cgs.gameSounds[sound];

		if ( !radius )
			radius = 256.0f;

		if ( !ed.parm && !ed.parm2 )
			sprite = explosionMaterials[rand() % 3];

		if ( !ed.sound )
			sound = explosionSounds[rand() % 3];

		if ( ed.parm2 )
		{
			sprite = explosionMaterials[ed.parm2];
			sound = explosionSounds[ed.parm2];
		}

		int spriteAnimationLength = trap_R_AnimationLength( sprite );

		le = CG_MakeExplosion( cent->currentState.origin, direction, 0, sprite, spriteAnimationLength, true );
		trap_S_StartSound( cent->currentState.origin, 0, CHAN_AUTO, sound );
		trap_S_StartSound( cent->currentState.origin, 0, CHAN_AUTO, sound );
		trap_S_StartSound( cent->currentState.origin, 0, CHAN_AUTO, sound ); // Megahack: make the explosion sound 3x louder

		le->light = radius * 1.5f;
		Vector( 1.0f, 0.75f, 0.25f ).CopyToArray( le->lightColor );

		re = &le->refEntity;
		le->radius = radius * 0.5f;
		re->rotation = 0;

		for ( int i = 0; i < ((int)radius / 24); i++ )
		{
			Vector r = CRandomVector( Vector( radius * 0.6f, radius * 0.6f, radius * 0.6f ) );
			Vector randomisedOrigin = Vector( cent->currentState.origin ) + r;
			
			/*
			Vector ambient, direct, dir;
			trap_R_LightForPoint( p, ambient, direct, dir );
			
			ambient /= 255.0f;
			direct /= 255.0f;

			sle = CG_MakeExplosion( randomisedOrigin, direction, 0, smokeMaterial, smokeLength, true );
			sle->radius = radius * (0.8f + crandom()*0.6f);
			sle->refEntity.rotation = 0;
			sle->color[0] = direct[0];// + ambient[0];
			sle->color[1] = direct[1];// + ambient[1];
			sle->color[2] = direct[2];// + ambient[2];
			sle->color[3] = 3.0f;
			*/

			Particles::Smoke::CreateSmoke( cgs.media.smokeSprites[rand() % 3], // material
										   randomisedOrigin, Vector::Zero, // origin and velocity
										   Vector::Identity * 0.7f,// tint colour
										   1.5f + random() * 0.5f, // scale
										   0.5f + random() * 0.5f, // fade in time
										   5.0f + random() * 2.5f, // fade out time
										   10.0f + random() * 5.0f ); // life
		}

		// Shake the local client if nearby
		Vector viewOrigin = Vector( cg.refdef.vieworg );
		position = cent->currentState.origin;
		float playerDistance = (position - viewOrigin).Length();
		float tolerance = radius * 5.0f + 1.0f;

		if ( playerDistance < tolerance )
		{
			float shakeAmount = (1.0f - (playerDistance / tolerance)) * 8.0f;

			Vector r = CRandomVector( Vector( 1, 1, 1 ) ) * shakeAmount;
			GetClient()->GetView()->AddShake( 2.5f, 0.2f, r * -1.0f );
			GetClient()->GetView()->AddShake( 12.0f, 0.5f, r );
			GetClient()->GetView()->AddShake( 16.0f, 1.2f, r * 0.05f );
		}

		// Affect any nearby particles with a force
		auto force = ParticleManager::ParticleForce( Vector( cent->currentState.origin ) - Vector( 0, 0, radius * 0.333f ), 35.0f * radius, 0.3f, 2.0f, 0.05f );
		GetClient()->GetParticleManager()->AddForce( force );
	}

private:
	qhandle_t	explosionMaterials[3];
	qhandle_t	smokeMaterial;
	int			smokeLength;
	sfxHandle_t explosionSounds[3];
};

RegisterEventParser( CE_Explosion, Explosion );
