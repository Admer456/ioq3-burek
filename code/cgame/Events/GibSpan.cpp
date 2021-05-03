#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"
#include "View/ParticleManager.hpp"
#include "View/Particles/BaseGib.hpp"

class GibSpan : public EventParser
{
public:
	GibSpan( int ev );

	void RegisterAssets() override
	{
		for ( int i = 0; i < Material_Max * 3; i++ )
		{
			sounds[i] = trap_S_RegisterSound( GibSounds[i], false );
		}
	}

	void LaunchGib( Vector origin, Vector velocity, qhandle_t model, int materialType, sfxHandle_t sound = 0 )
	{
		using namespace Particles;

		BaseGib::CreateGib( model, origin, velocity, materialType, sound );
	}

	Vector CRandomVector( Vector max )
	{
		return Vector( crandom() * max.x, crandom() * max.y, crandom() * max.z );
	}

	Vector RandomVector( Vector max )
	{
		return Vector( random() * max.x, random() * max.y, random() * max.z );
	}

	void Parse( centity_t* cent, Vector position ) override
	{
		EventData ed( cent->currentState );

		Vector mins = ed.vparm;
		Vector maxs = ed.vparm2;

		// Shrinks the mixs and maxs by 15%
		Vector centre = (mins + maxs) / 2.0f;
		Vector bboxExtent = (maxs - centre) * 0.85f;
		mins = centre - bboxExtent;
		maxs = centre + bboxExtent;

		Vector delta = maxs - mins;

		int gibs = ed.parm;
		int gibModelVariation = ed.parm2;
		int materialType = ed.parm4;

		Vector angles = Vector( ed.fparm, ed.fparm2, 0 );
		Vector direction;
		Vector::AngleVectors( angles, &direction, nullptr, nullptr );

		float intensity = ed.fparm3;

		for ( int i = 0; i < gibs; i++ )
		{
			Vector r = RandomVector( delta );
			Vector pos = mins + r;

			Vector directionRandomised;

			if ( intensity >= 0.0f )
			{
				directionRandomised = direction + CRandomVector( Vector( 0.6, 0.6, 0.9 ) );
				directionRandomised *= intensity;
			}
			else
			{
				directionRandomised = CRandomVector( Vector( 1, 1, 2 ) );
				directionRandomised *= -intensity;
			}

			qhandle_t model = ed.model + (rand() % gibModelVariation);

			LaunchGib( pos, directionRandomised, cgs.gameModels[model], materialType, sounds[materialType*3 + rand()%3] );
		}
	}

private:
	constexpr static const char* GibSounds[Material_Max * 3]
	{
		"sound/debris/glass_gib1.wav",
		"sound/debris/glass_gib2.wav",
		"sound/debris/glass_gib3.wav",

		"sound/debris/wood_gib1.wav",
		"sound/debris/wood_gib2.wav",
		"sound/debris/wood_gib3.wav",

		"sound/debris/concrete_gib1.wav",
		"sound/debris/concrete_gib2.wav",
		"sound/debris/concrete_gib3.wav",

		"sound/debris/metal_gib1.wav",
		"sound/debris/metal_gib2.wav",
		"sound/debris/metal_gib3.wav"
	};

	sfxHandle_t sounds[Material_Max * 3];
};

RegisterEventParser( CE_GibSpan, GibSpan );