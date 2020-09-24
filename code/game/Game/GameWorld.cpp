#include "g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "../qcommon/Maths/Vector.hpp"
#include "GameWorld.hpp"
#include "../qcommon/IEngineExports.h"

#include <type_traits>

GameWorld* gameWorld;

KeyValueLibrary::~KeyValueLibrary()
{
	library.clear();
}

void KeyValueLibrary::AddKeyValue( const char* key, const char* value )
{
	library[std::string(key)] = value;
}

std::string& KeyValueLibrary::GetValue( const char* keyName )
{
	return library[keyName];
}

const char* KeyValueLibrary::GetCString( const char* keyName, const char* defaultValue )
{
	const char* keyValue = library[keyName].c_str();

	if ( keyValue && keyValue[0] )
	{
		return keyValue;
	}
	else
	{
		return defaultValue;
	}
}

float KeyValueLibrary::GetFloat( const char* keyName, float defaultValue )
{
	const char* keyValue = library[keyName].c_str();

	if ( keyValue && keyValue[0] )
	{
		return atof( keyValue );
	}
	else
	{
		return defaultValue;
	}
}

int KeyValueLibrary::GetInt( const char* keyName, int defaultValue )
{
	const char* keyValue = library[keyName].c_str();

	if ( keyValue && keyValue[0] )
	{
		return atoi( keyValue );
	}
	else
	{
		return defaultValue;
	}
}

bool KeyValueLibrary::GetBool( const char* keyName, int defaultValue )
{
	const char* keyValue = library[keyName].c_str();

	if ( keyValue && keyValue[0] )
	{
		return atoi( keyValue ) != 0;
	}
	else
	{
		return defaultValue;
	}
}

Vector KeyValueLibrary::GetVector( const char* keyName, Vector defaultValue )
{
	const char* keyValue = library[keyName].c_str();
	if ( keyValue && keyValue[0] )
	{
		Vector v = defaultValue;
		sscanf( keyValue, "%f %f %f", &v.x, &v.y, &v.z );
		return v;
	}
	else
	{
		return defaultValue;
	}
}

GameWorld::~GameWorld()
{
	Shutdown();
}

void GameWorld::Shutdown()
{
	// Delete every entity
	for ( auto& ent : gEntities )
	{
		if ( ent )
		{
			delete ent;
		}
	}

	// Dunno what else lmao
}

void GameWorld::SpawnEntities()
{
	// For every keyvalue pair library in the map
	for ( auto& lib : keyValueLibraries )
	{
		// Get the unordered_map so we can read keyvalues
		std::string &className = lib["classname"];

		// In this very early phase of my entity system,
		// we'll only spawn instances of func_nothing
		if ( className == "func_nothing" )
		{
			// Spawn the entity from these keyvalues
			SpawnEntity( lib );
		}
	}
}

void GameWorld::SpawnEntity( KeyValueLibrary& map )
{
	using namespace Entities;

	// Until we make an entity factory, we'll only spawn BaseQuakeEntity
	auto ent = CreateEntity<BaseQuakeEntity>();

	if ( nullptr == ent )
	{
		engine->Error( "Tried to spawn a non-existing entity!\n" );
	}

	ent->spawnArgs = &map;

	ent->KeyValue();
	ent->Spawn();
}

template<typename entityType>
entityType* GameWorld::CreateEntity()
{
	for ( int i = 0; i < MaxEntities; i++ )
	{
		// Also check for g_entities so we don't get any conflicts
		// on the client & server and whatnot
		if ( gEntities[i] == nullptr && !g_entities[i].inuse )
		{
			gEntities[i] = new entityType();
			return static_cast<entityType*>( gEntities[i] );
		}
	}

	engine->Error( "Exceeded maximum number of entities\n" );
	return nullptr;
}

void GameWorld::ParseKeyValues()
{

}

void GameWorld::AssignKeyValuesToEntities()
{
	for ( auto& ent : gEntities )
	{
		if ( ent == nullptr )
		{
			continue;
		}

		ent->KeyValue();
	}
}
