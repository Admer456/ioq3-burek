#include "g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Maths/Vector.hpp"
#include "GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/Worldspawn.hpp"
#include "Entities/BasePlayer.hpp"

#include "Entities/Info/InfoPlayerStart.hpp"

#include <type_traits>

GameWorld* gameWorld;

// Temporarily copied here from g_client.cpp
static vec3_t	playerMins = { -15, -15, -24 };
static vec3_t	playerMaxs = { 15, 15, 32 };

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
	SpawnWorldspawn();

	// For every keyvalue pair library in the map
	for ( auto& lib : keyValueLibraries )
	{
		// Get the unordered_map so we can read keyvalues
		std::string &className = lib["classname"];

		// Skip worldspawn
		if ( className == "worldspawn" )
			continue;

		// In this very early phase of my entity system,
		// we'll only spawn instances of func_nothing
		if ( className == "func_nothing" )
		{
			// Spawn the entity from these keyvalues
			SpawnEntity( lib );
		}
	}
}

void GameWorld::SpawnWorldspawn()
{
	using namespace Entities;

	KeyValueLibrary* worldLib = nullptr;

	for ( auto& lib : keyValueLibraries )
	{
		std::string& className = lib["classname"];

		if ( className == "worldspawn" )
		{
			worldLib = &lib;
			break;
		}
	}

	auto ent = CreateEntity<Worldspawn>( ENTITYNUM_WORLD );
	ent->GetShared()->ownerNum = ENTITYNUM_NONE;
	ent->spawnArgs = worldLib;

	ent->KeyValue();
	ent->Spawn();

	auto nothingEnt = CreateEntity<BaseQuakeEntity>( ENTITYNUM_NONE );
	nothingEnt->GetShared()->ownerNum = ENTITYNUM_NONE;
	nothingEnt->spawnArgs = nullptr;
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

	// THERE SHALL BE MORE ENTITIES
	level.num_entities++;

	// let the server system know that there are more entities
	gameImports->LocateGameData(
		reinterpret_cast<sharedEntity_t*>(level.gentities), level.num_entities, sizeof( gentity_t ),
		level.entities, level.numEntities, sizeof( IEntity* ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	ent->spawnArgs = &map;

	ent->KeyValue();
	ent->Spawn();
}

template<typename entityType>
entityType* GameWorld::CreateEntity()
{
	for ( unsigned int i = MAX_CLIENTS; i < MaxEntities; i++ )
	{
		// Also check for g_entities so we don't get any conflicts
		// on the client & server and whatnot
		if ( gEntities[i] == nullptr && !g_entities[i].inuse )
		{
			gEntities[i] = new entityType();
			gEntities[i]->SetEntityIndex( i );
			return static_cast<entityType*>( gEntities[i] );
		}
	}

	engine->Error( "Exceeded maximum number of entities\n" );
	return nullptr;
}

template<typename entityType>
entityType* GameWorld::CreateEntity( const uint16_t& index )
{
	if ( nullptr != gEntities[index] )
	{
		engine->Error( va( "Entity slot %d is taken, please consult your programmer\n", index ) );
		return nullptr;
	}

	gEntities[index] = new entityType();
	gEntities[index]->SetEntityIndex( index );
	gEntities[index]->GetState()->number = index;
	return static_cast<entityType*>( gEntities[index] );
}

void GameWorld::SpawnClient( Entities::BasePlayer* player )
{
	int		index;
	Vector	spawn_origin{Vector::Zero}, spawn_angles{Vector::Zero};
	gclient_t* client;
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	Entities::IEntity* spawnPoint;
	Entities::IEntity* tent;
	int		flags;
	int		savedPing;
	int		accuracy_hits, accuracy_shots;
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];

	index = player->GetEntityIndex();
	client = player->GetClient();

	// Find a spawn point
	spawnPoint = FindSpawnPoint<Entities::InfoPlayerStart>( client->ps.origin, false );

	VectorCopy( spawnPoint->GetShared()->currentOrigin, player->GetShared()->currentOrigin );
	
	// Toggle the teleport bit so the client doesn't lerp
	flags = client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// Clear everything but the persistent data
	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;

	for ( i = 0; i < MAX_PERSISTANT; i++ ) 
	{
		persistant[i] = client->ps.persistant[i];
	}
	
	eventSequence = client->ps.eventSequence;

	Com_Memset( client, 0, sizeof( *client ) );

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;

	for ( i = 0; i < MAX_PERSISTANT; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	trap_GetUserinfo( index, userinfo, sizeof( userinfo ) );
	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.eFlags = flags;

	player->GetState()->groundEntityNum = ENTITYNUM_NONE;
	player->SetClient( &level.clients[index] );
	player->takeDamage = qtrue;
	player->className = "player";
	player->GetShared()->contents = CONTENTS_BODY;
	player->clipMask = MASK_PLAYERSOLID;
	player->waterLevel = 0;
	player->waterType = 0;
	player->flags = 0;

	VectorCopy( playerMins, player->GetShared()->mins );
	VectorCopy( playerMaxs, player->GetShared()->maxs );

	client->ps.clientNum = index;

	client->ps.stats[STAT_WEAPONS] = (1 << WP_MACHINEGUN);
	if ( g_gametype.integer == GT_TEAM ) {
		client->ps.ammo[WP_MACHINEGUN] = 50;
	}
	else {
		client->ps.ammo[WP_MACHINEGUN] = 100;
	}

	client->ps.stats[STAT_WEAPONS] |= (1 << WP_GAUNTLET);
	client->ps.ammo[WP_GAUNTLET] = -1;
	client->ps.ammo[WP_GRAPPLING_HOOK] = -1;

	// health will count down towards max_health
	player->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] + 25;

	player->SetOrigin( spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd( client - level.clients, &player->GetClient()->pers.cmd );
	player->SetClientViewAngle( spawn_angles );
	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	// set default animations
	client->ps.torsoAnim = TORSO_STAND;
	client->ps.legsAnim = LEGS_IDLE;

	if ( !level.intermissiontime ) 
	{
		if ( player->GetClient()->sess.sessionTeam != TEAM_SPECTATOR ) 
		{
			// TODO: Implement KillBox for IEntities
			////G_KillBox( ent );

			// force the base weapon up
			client->ps.weapon = WP_MACHINEGUN;
			client->ps.weaponstate = WEAPON_READY;
			// fire the targets of the spawn point
			//G_UseTargets( spawnPoint, ent );
			// select the highest weapon number available, after any spawn given items have fired
			client->ps.weapon = 1;

			for ( i = WP_NUM_WEAPONS - 1; i > 0; i-- ) {
				if ( client->ps.stats[STAT_WEAPONS] & (1 << i) ) {
					client->ps.weapon = i;
					break;
				}
			}
			// positively link the client, even if the command times are weird
			VectorCopy( player->GetClient()->ps.origin, player->GetShared()->currentOrigin );

			//tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
			//tent->s.clientNum = ent->s.clientNum;
			//
			//trap_LinkEntity( ent );
		}
	}
	else {
		// move players to intermission

		// TODO: MoveClientToIntermission for IEntities
		////MoveClientToIntermission( ent );
	}
	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	player->GetClient()->pers.cmd.serverTime = level.time;

	// TODO: ClientThink in GameWorld for IEntities
	////ClientThink( ent - g_entities );

	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	if ( player->GetClient()->sess.spectatorState != SPECTATOR_FOLLOW ) {
		// TODO: ClientEndFrame for IEntities
		////ClientEndFrame( ent );
	}

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, player->GetState(), qtrue );
}

template<typename entityType>
entityType* GameWorld::FindSpawnPoint( Vector avoidPoint, bool isBot )
{
	for ( Entities::IEntity* ent : gEntities )
	{
		entityType* spawn = dynamic_cast<entityType*>( ent );
		if ( nullptr != spawn )
		{
			return spawn;
		}
	}

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
