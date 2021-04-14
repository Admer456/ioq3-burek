#include "cg_local.hpp"
#include "Collections/KeyValueLibrary.hpp"
#include "VegetationInstance.hpp"
#include "VegetationSystem.hpp"

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
		if ( !Q_stricmpn( lib["classname"].c_str(), "veg", 3 ) )
		{
			Vector origin = lib.GetVector( "origin", Vector::Zero );
			Vector angles = lib.GetVector( "angles", Vector::Zero );
			const char* model = lib.GetCString( "model", "" );
			qhandle_t modelid = trap_R_RegisterModel( model );
			float scale = lib.GetFloat( "scale", -1.0f );
			float distance = lib.GetFloat( "distance", 6000.0f );
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
	}
}

void VegetationSystem::Render()
{
	for ( size_t i = 0; i < MaxVegetationInstances; i++ )
	{
		if ( instances[i].active )
			instances[i].Render();
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
