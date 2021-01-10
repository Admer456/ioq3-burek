#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"

class GibSpan : public EventParser
{
public:
	GibSpan( int ev );

	void LaunchGib( Vector origin, Vector velocity, qhandle_t model )
	{
		localEntity_t* le;
		refEntity_t* re;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + 30000 + random() * 10000;

		origin.CopyToArray( re->origin );
		origin.CopyToArray( le->pos.trBase );

		AxisCopy( axisDefault, re->axis );
		re->hModel = model;

		le->pos.trType = TR_GRAVITY;
		velocity.CopyToArray( le->pos.trDelta );
		le->pos.trTime = cg.time;
		le->bounceFactor = 0.6f;

		float randomScale = 2.0f + crandom()*1.5f;

		for ( int i = 0; i < 3; i++ )
			for ( int j = 0; j < 3; j++ )
				re->axis[i][j] *= randomScale;
	}

	Vector CRandomVector( Vector max )
	{
		return Vector( crandom() * max.x, crandom() * max.y, crandom() * max.z );
	}

	Vector RandomVector( Vector max )
	{
		return Vector( random() * max.x, random() * max.y, random() * max.z );
	}

	void Parse( centity_t* cent, Vector position )
	{
		EventData ed( cent->currentState );

		Vector mins = ed.vparm;
		Vector maxs = ed.vparm2;
		Vector delta = maxs - mins;

		int gibs = ed.parm;
		int gibModelVariation = ed.parm2;

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
				directionRandomised = direction + CRandomVector( Vector( 0.2, 0.2, 0.2 ) );
				directionRandomised *= intensity;
			}
			else
			{
				directionRandomised = CRandomVector( Vector( 1, 1, 1 ) );
				directionRandomised *= -intensity;
			}

			qhandle_t model = ed.model + (rand() % gibModelVariation);

			LaunchGib( pos, directionRandomised, cgs.gameModels[model] );
		}
	}
};

RegisterEventParser( CE_GibSpan, GibSpan );