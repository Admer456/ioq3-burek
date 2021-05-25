#include "cg_local.hpp"
#include "Collections/KeyValueLibrary.hpp"
#include "VegetationInstance.hpp"
#include "VegetationSystem.hpp"
#include "Times/Timer.hpp"
#include "View/ParticleManager.hpp"
#include "View/Particles/Bird.hpp"

Vector VegetationSystem::playerEyePosition = Vector::Zero;

void VegetationSystem::Init()
{
	Reload();
}

void VegetationSystem::Clear()
{
	for ( size_t i = 0; i < MaxVegetationInstances; i++ )
	{
		instances[i].Clear();
	}

	libraries.clear();
}

void VegetationSystem::Reload()
{
	Clear();

	if ( !ParseKeyvalues() )
		CG_Error( "VegetationSystem::Reload: no entities" );

	while ( ParseKeyvalues() )
	{
		// Do nothing, just load stuff
	}

	for ( KeyValueLibrary& lib : libraries )
	{
		const char* className = lib["classname"].c_str();

		if ( !Q_stricmpn( className, "veg", 3 ) )
		{
			if ( !Q_stricmp( className, "veg_generic" ) )
				ParseGeneric( lib );
			else if ( !Q_stricmp( className, "veg_sprayer" ) )
				ParseSprayer( lib );
		}
	}
}

void VegetationSystem::Render()
{
	Timer profileTimer( false );
	float profileTime{ 0.0f };

	if ( cg_profileVegetation.integer )
	{
		profileTimer.Reset();
	}

	for ( size_t i = 0; i < MaxVegetationInstances; i++ )
	{
		if ( instances[i].active )
			instances[i].Render();
	}

	if ( cg_profileVegetation.integer )
	{
		profileTime = profileTimer.GetElapsed( Timer::Milliseconds );
		CG_Printf( "VegetationSystem::Render %3.3f ms\n", profileTime );
	}
}

void VegetationSystem::Add( const VegetationInstance& instance )
{
	for ( size_t i = 0; i < MaxVegetationInstances; i++ )
	{
		if ( !instances[i].active )
		{
			instances[i] = instance;
			return;
		}
	}
}

void VegetationSystem::Update()
{
	playerEyePosition = GetClient()->GetView()->GetViewOrigin();
}

bool VegetationSystem::ParseKeyvalues()
{
	char keyname[MAX_TOKEN_CHARS];
	char com_token[MAX_TOKEN_CHARS];

	// Parse the opening brace
	if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) )
	{
		// End of spawn string
		return false;
	}

	if ( com_token[0] != '{' )
	{
		CG_Error( "GameWorld::ParseKeyValues: found %s when expecting {", com_token );
	}

	KeyValueLibrary lib;

	// Go through all the key / value pairs
	while ( true )
	{
		// Parse key
		if ( !trap_GetEntityToken( keyname, sizeof( keyname ) ) )
		{
			CG_Error( "GameWorld::ParseKeyValues: EOF without closing brace" );
		}

		if ( keyname[0] == '}' )
		{
			break;
		}

		// Parse value	
		if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) )
		{
			CG_Error( "GameWorld::ParseKeyValues: EOF without closing brace" );
		}

		if ( com_token[0] == '}' )
		{
			CG_Error( "GameWorld::ParseKeyValues: closing brace without data" );
		}

		lib.AddKeyValue( keyname, com_token );
	}

	libraries.push_back( lib );

	return true;
}

void VegetationSystem::ParseGeneric( KeyValueLibrary& lib )
{
	Vector origin = lib.GetVector( "origin", Vector::Zero );
	Vector angles = lib.GetVector( "angles", Vector::Zero );
	const char* model = lib.GetCString( "model", "" );
	qhandle_t modelid = trap_R_RegisterModel( model );
	float scale = lib.GetFloat( "scale", -1.0f );
	float distance = lib.GetFloat( "distance", 5000.0f );
	float wind = lib.GetFloat( "wind", 16.0f );

	if ( angles == Vector::Zero )
	{
		angles.y = lib.GetFloat( "angle", 0.0f );

		if ( angles.y == -1.0f )
		{
			angles.y = random() * 359.999f;
		}
	}

	if ( scale < 0.0f )
	{
		scale = 0.8f + (random() * -scale * 0.5f);
	}

	if ( model != 0 )
	{
		VegetationInstance vi = VegetationInstance( modelid, origin, angles, scale, distance, wind );
		Add( vi );
	}
}

void VegetationSystem::ParseSprayer( KeyValueLibrary& lib )
{
	Vector origin = lib.GetVector( "origin", Vector::Zero );
	Vector angles = lib.GetVector( "angles", Vector::Zero );

	const char* model1 = lib.GetCString( "model", "" );
	const char* model2 = lib.GetCString( "model1", "" );
	const char* model3 = lib.GetCString( "model2", "" );
	const char* model4 = lib.GetCString( "model3", "" );
	qhandle_t models[] =
	{
		trap_R_RegisterModel( model1 ),
		trap_R_RegisterModel( model2 ),
		trap_R_RegisterModel( model3 ),
		trap_R_RegisterModel( model4 )
	};

	// Quickly count how many models we got
	int modelCount = 0;
	for ( const qhandle_t& m : models )
	{
		if ( m )
			modelCount++;
		else
			break;
	}

	if ( !modelCount )
		return;

	float scale = lib.GetFloat( "scale", -1.0f );
	float distance = lib.GetFloat( "distance", 5000.0f );
	float wind = lib.GetFloat( "wind", 16.0f );

	float radius = lib.GetInt( "radius", 200.0f );
	float density = lib.GetFloat( "density", 1.0f ); // instances per metre squared
	int howMany = (radius * density) / 40.0f; // 40 units is approx. 1m, so we gotta divide by that

	for ( int i = 0; i < howMany; i++ )
	{
		Vector randomVector = Vector( crandom(), crandom(), 0 );

		Vector rOrigin = origin + randomVector * radius;
		Vector rAngles = angles;
		float rScale = scale;

		if ( angles == Vector::Zero )
		{
			rAngles.y = lib.GetFloat( "angle", 0.0f );

			if ( rAngles.y == -1.0f )
			{
				rAngles.y = random() * 359.999f;
			}
		}

		if ( scale < 0.0f )
		{
			rScale = 0.8f + (random() * -scale * 0.5f);
		}
		
		VegetationInstance vi = VegetationInstance( models[i % modelCount], rOrigin, rAngles, rScale, distance, wind );
		Add( vi );
	}

	for ( int i = 0; i < howMany / 5; i++ )
	{
		Vector randomVector = Vector( crandom(), crandom(), 0 );
		Vector rOrigin = origin + randomVector * radius;

		Particles::Bird::CreateBird( rOrigin, origin - Vector( 0, 0, 128 ) );
	}
}
