#pragma once

#include <vector>
#include <string>
#include <unordered_map>

class Vector;

class KeyValueLibrary final
{
	using KVMap = std::unordered_map<std::string, std::string>;

public:
	KeyValueLibrary() = default;
	~KeyValueLibrary();

	void			AddKeyValue( const char* key, const char* value );
	std::string&	GetValue( const char* keyName );
	KVMap&			GetMap() { return library; }

	// Templ8s didn't work here for some odd reason so I'm gonna specialise
	const char*		GetCString( const char* keyName, const char* defaultValue );
	float			GetFloat( const char* keyName, float defaultValue );
	int				GetInt( const char* keyName, int defaultValue );
	bool			GetBool( const char* keyName, int defaultValue );
	Vector			GetVector( const char* keyName, Vector defaultValue );

	std::string& operator [] ( const char* val )
	{
		return library[val];
	}

private:
	KVMap library;
};

class GameWorld final
{
	using KVMap = std::unordered_map<std::string, std::string>;

public:
	~GameWorld();

	// Deletes entities and stuff
	void			Shutdown();

	// Spawns worldspawn, goes on to do some more stuff
	void			SpawnEntities();

	// Dispatches the spawn of an individual entity, from keyvalues
	void			SpawnEntity( KeyValueLibrary& map );

	// Allocates an entity
	template<typename entityType>
	entityType*		CreateEntity();

	// Reads out keyvalues and populates the stuff
	void			ParseKeyValues();

	// After parsing, the game world will give each 
	// entity its respective list of keyvalue pairs
	void			AssignKeyValuesToEntities();

	// All the entity keyvalue pairs in the map
	// Each KV library represents one entity
	std::vector<KeyValueLibrary> keyValueLibraries;

public:
	constexpr static size_t MaxEntities = MAX_GENTITIES;
};

extern GameWorld* gameWorld;
