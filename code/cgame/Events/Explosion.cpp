#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"

class Explosion final : public EventParser
{
public:
	Explosion( int ev );

	void RegisterAssets() override
	{
		explosionMaterials[0] = trap_R_RegisterShader( "sprites/explo1" );
		explosionMaterials[1] = trap_R_RegisterShader( "sprites/explo2" );
		explosionMaterials[2] = trap_R_RegisterShader( "sprites/explo3" );

		explosionSounds[0] = trap_S_RegisterSound( "sound/debris/explo1.wav", false );
		explosionSounds[1] = trap_S_RegisterSound( "sound/debris/explo2.wav", false );
		explosionSounds[2] = trap_S_RegisterSound( "sound/debris/explo3.wav", false );
	}

	void Parse( centity_t* cent, Vector position ) override
	{
		EventData ed( cent->currentState );

		localEntity_t* le;

		float radius = ed.fparm;
		qhandle_t sprite = ed.parm;
		Vector direction = ed.vparm;
		sfxHandle_t sound = ed.sound;

		sprite = cgs.gameMaterials[sprite];
		sound = cgs.gameSounds[sound];

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
		trap_S_StartSound( cent->currentState.origin, -1, 0, sound );
	}

private:
	qhandle_t explosionMaterials[3];
	sfxHandle_t explosionSounds[3];
};

RegisterEventParser( CE_Explosion, Explosion );
