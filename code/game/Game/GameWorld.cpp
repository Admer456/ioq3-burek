#include "Maths/Vector.hpp"
#include "g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/Worldspawn.hpp"
#include "Entities/BasePlayer.hpp"

#include "Entities/Info/InfoPlayerStart.hpp"
#include "Entities/Weapons/BaseWeapon.hpp"
#include "Entities/Weapons/Weapon_Fists.hpp"

#include <type_traits>
#include <array>

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

SpawnRegistry::SpawnRegistry()
{
	map.clear();
}

void SpawnRegistry::Add( Entities::IEntity* ent, bool isDynamic )
{
	EntityClassInfo* eci = ent->GetClassInfo();
	Vector origin = ent->GetOrigin();
	size_t id = isDynamic ? ent->GetEntityIndex() : ENTITYNUM_NONE;
	
	SpawnInfo spawnInfo{ id, ent->GetSpawnflags(), origin };
	
	map[eci].push_back( spawnInfo );
}

SpawnRegistry::SpawnInfo SpawnRegistry::GetRandomFurthest( const EntityClassInfo& eci, const Vector& avoidPoint, bool isBot )
{
	SpawnInfo* spot{ nullptr };
	vec3_t delta;
	float dist;
	float list_dist[GameWorld::MaxSpawnPoints];
	SpawnInfo* list_spot[GameWorld::MaxSpawnPoints];
	int	numSpots, rnd, i, j;

	numSpots = 0;

	std::vector<SpawnInfo> siv = map[const_cast<EntityClassInfo*>(&eci)];

	for ( SpawnInfo& si : siv )
	{
		spot = &si;

		// if ( SpotWouldTelefrag( si ) )
		// continue;

		if ( si.spawnPointFlags & FL_NO_BOTS && isBot ||
			 si.spawnPointFlags & FL_NO_HUMANS && !isBot )
		{
			// spot is not for this human/bot player
			continue;
		}

		VectorSubtract( si.spawnPointPosition, avoidPoint, delta );
		dist = VectorLength( delta );

		for ( i = 0; i < numSpots; i++ )
		{
			if ( dist > list_dist[i] )
			{
				if ( numSpots >= GameWorld::MaxSpawnPoints )
					numSpots = GameWorld::MaxSpawnPoints - 1;

				for ( j = numSpots; j > i; j-- )
				{
					list_dist[j] = list_dist[j - 1];
					list_spot[j] = list_spot[j - 1];
				}

				list_dist[i] = dist;
				list_spot[i] = spot;

				numSpots++;
				break;
			}
		}

		if ( i >= numSpots && numSpots < GameWorld::MaxSpawnPoints )
		{
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}

	if ( !numSpots )
	{
		engine->Print( "WARNING: Couldn't find a spawn point\n" );

		SpawnInfo SI;
		SI.spawnPointPosition = Vector::Zero;

		return SI;
	}

	else
	{
		// select a random spot from the spawn points furthest away
		rnd = random() * (numSpots - 1);

		return *list_spot[rnd];
	}
}

// Here I've placed two utility functions, temporarily, until we get a GameUtils.hpp thing
namespace Util
{
	void Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask )
	{
		return gameImports->Trace( results, start, mins, maxs, end, passEntityNum, contentMask );
	}

	int PointContents( const vec3_t point, int passEntityNum )
	{
		return gameImports->PointContents( point, passEntityNum );
	}
}

GameWorld::GameWorld()
{
	keyValueLibraries.clear();
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
	level.spawning = true;
	level.numSpawnVars = 0;

	if ( !ParseKeyValues() )
		engine->Error( "GameWorld::SpawnEntities: no entities" );

	SpawnWorldspawn();

	while ( ParseKeyValues() )
	{
		// Do nothing, just load stuff into keyValueLibraries
	}

	// For every keyvalue pair library in the map
	for ( auto& lib : keyValueLibraries )
	{
		// Get the unordered_map so we can read keyvalues
		std::string &className = lib["classname"];

		// Skip worldspawn
		if ( className == "worldspawn" )
			continue;

		// Spawn the entity from these keyvalues
		SpawnEntity( lib );
	}

	// Execute PostSpawn for each entity
	for ( Entities::IEntity*& ent : gEntities )
	{
		if ( nullptr == ent )
		{
			continue;
		}

		ent->PostSpawn();
	}

	level.spawning = false;
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
	ent->className = "worldspawn";
	ent->spawnArgs = worldLib;

	ent->ParseKeyvalues();
	ent->Spawn();

	auto nothingEnt = CreateEntity<BaseEntity>( ENTITYNUM_NONE );
	nothingEnt->GetShared()->ownerNum = ENTITYNUM_NONE;
	nothingEnt->className = "nothing";
	nothingEnt->spawnArgs = nullptr;
}

void GameWorld::SpawnEntity( KeyValueLibrary& map )
{
	using namespace Entities;
	
	// Look up class information by the map classname,
	// to get a function pointer to the entity's allocator function
	const char* className = map.GetMap()["classname"].c_str();
	EntityClassInfo* eci = EntityClassInfo::GetInfoByMapName( className );

	if ( nullptr == eci || nullptr == eci->AllocateInstance )
	{
		engine->Print( va( "WARNING: Cannot find spawner for entity class %s\n", className ) );
		return;
	}

	BaseEntity* ent = static_cast<BaseEntity*>( eci->AllocateInstance() );

	memset( ent->GetSharedEntity(), 0, sizeof( sharedEntity_t ) );

	if ( nullptr == ent )
	{
		engine->Error( "Tried to spawn a non-existing entity!\n" );
	}

	// THERE SHALL BE MORE ENTITIES
	level.num_entities++;

	// let the server system know that there are more entities
	gameImports->LocateGameData(
		nullptr, level.num_entities, 0,
		level.entities, level.numEntities, sizeof( IEntity* ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	ent->GetShared()->ownerNum = ENTITYNUM_NONE;

	ent->spawnArgs = &map;

	ent->ParseKeyvalues();
	ent->Precache();
	ent->Spawn();
}

Entities::IEntity* GameWorld::CreateTempEntity( const Vector& origin, int event )
{
	using namespace Entities;

	BaseEntity* ent = CreateEntity<BaseEntity>();
	Vector snapped = origin;
	
	ent->GetState()->eType = ET_EVENTS + event;

	//ent->className = "tempEntity";
	ent->eventTime = level.time;
	ent->freeAfterEvent = true;
	Macro_SnapVector( snapped ); // save network bandwidth
	ent->SetOrigin( snapped );

	gameImports->LinkEntity( ent );

	// THERE SHALL BE MORE ENTITIES
	level.num_entities++;

	// let the server system know that there are more entities
	gameImports->LocateGameData(
		nullptr, level.num_entities, 0,
		level.entities, level.numEntities, sizeof( IEntity* ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	return ent;
}

void GameWorld::EmitComplexEvent( const Vector& origin, const Vector& angles, const EventData& ed )
{
	entityState_t es = ed.ToEntityState();
	Vector anglesSnapped = angles.Snapped();
	auto ent = CreateTempEntity( origin, ed.id );

	es.number = ent->GetEntityIndex();
	es.eType = ET_EVENTS + ed.id;
	ent->GetShared()->plow = NetPlow_ForcePVS;

	// USUALLY, I would NEVER do this, but in this case, it makes sense
	memcpy( ent->GetState(), &es, sizeof( entityState_t ) );
	ent->SetOrigin( origin );
	ent->SetAngles( anglesSnapped );
	gameImports->LinkEntity( ent );
}

void GameWorld::FreeEntity( Entities::IEntity* ent )
{
	if ( ent )
	{
		gEntities[ent->GetEntityIndex()] = nullptr;
		delete ent;
		ent = nullptr;
	}
}

void GameWorld::TouchTriggers( Entities::IEntity* ent )
{
	int	i, num;
	std::vector<int> touch;
	Entities::IEntity* hit;
	Entities::BasePlayer* player;
	trace_t	trace;
	Vector mins, maxs, origin;
	static Vector range( 40, 40, 52 );

	touch.reserve( GameWorld::MaxEntities );

	player = dynamic_cast<Entities::BasePlayer*>( ent );

	if ( nullptr == player )
	{
		return;
	}

	// dead clients don't activate triggers!
	if ( player->GetClient()->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		return;
	}

	origin = player->GetClient()->ps.origin;

	mins = origin - range;
	maxs = origin + range;

	num = gameImports->EntitiesInBox( mins, maxs, touch.data(), MaxEntities );

	// can't use ent->absmin, because that has a one unit pad
	mins = origin + player->GetShared()->mins;
	maxs = origin + player->GetShared()->maxs;

	for ( i = 0; i < num; i++ ) 
	{
		hit = gEntities[touch.data()[i]];

		// If not a trigger_ entity, ignore
		if ( !(hit->GetShared()->contents & CONTENTS_TRIGGER) ) 
		{
			continue;
		}

		//// ignore most entities if a spectator
		//if ( player->GetClient()->sess.sessionTeam == TEAM_SPECTATOR ) 
		//{
		//	if ( hit->GetState()->eType != ET_TELEPORT_TRIGGER //&&
		//		 // this is ugly but adding a new ET_? type will
		//		 // most likely cause network incompatibilities
		//		 /*hit->touch != Touch_DoorTrigger*/ ) 
		//	{
		//		continue;
		//	}
		//}

		// use separate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->GetState()->eType == ET_ITEM ) 
		{
			if ( !BG_PlayerTouchesItem( &player->GetClient()->ps, hit->GetState(), level.time ) ) 
			{
				continue;
			}
		}
		else 
		{
			if ( !gameImports->EntityContact( mins, maxs, hit ) )
			{
				continue;
			}
		}

		memset( &trace, 0, sizeof( trace ) );

		hit->Touch( player, &trace );

		if ( (ent->GetShared()->svFlags & SVF_BOT) ) 
		{
			ent->Touch( hit, &trace );
		}
	}

	// if we didn't touch a jump pad this pmove frame
	if ( player->GetClient()->ps.jumppad_frame != player->GetClient()->ps.pmove_framecount ) 
	{
		player->GetClient()->ps.jumppad_frame = 0;
		player->GetClient()->ps.jumppad_ent = 0;
	}
}

Entities::IEntity* GameWorld::FindByName( const char* entityName, Entities::IEntity* lastEntity )
{
	size_t index = 0;

	if ( lastEntity )
		index = lastEntity->GetEntityIndex()+1;

	for ( ; index < MaxEntities; index++ )
	{
		Entities::IEntity* ent = gEntities[index];
		
		if ( nullptr == ent )
			continue;

		if ( !ent->GetName() )
			continue;

		if ( !strcmp( ent->GetName(), entityName ) )
		{
			return ent;
		}
	}

	return nullptr;
}

Entities::IEntity* GameWorld::FindByClassname( const char* className, Entities::IEntity* lastEntity )
{
	size_t index = 0;

	if ( lastEntity )
		index = lastEntity->GetEntityIndex();

	for ( ; index < MaxEntities; index++ )
	{
		Entities::IEntity* ent = gEntities[index];

		if ( nullptr == ent )
			continue;

		if ( !strcmp( ent->GetClassname(), className ) )
		{
			return ent;
		}
	}

	return nullptr;
}

Entities::IEntity* GameWorld::FindByKeyvalue( const char* name, const char* value, Entities::IEntity* lastEntity )
{
	size_t index = 0;

	if ( lastEntity )
		index = lastEntity->GetEntityIndex();

	for ( ; index < MaxEntities; index++ )
	{
		Entities::IEntity* ent = gEntities[index];

		if ( nullptr == ent )
			continue;

		if ( !strcmp( static_cast<Entities::BaseEntity*>(ent)->spawnArgs->GetCString( name, "" ), value ) )
		{
			return ent;
		}
	}

	return nullptr;
}

Entities::IEntity* GameWorld::FindByNameRandom( const char* entityName )
{
	static int seed = 0x92;

	std::vector<Entities::IEntity*> ents;
	ents.reserve( 8U );

	for ( size_t index = 0; index < MaxEntities; index++ )
	{
		Entities::IEntity* ent = gEntities[index];

		if ( nullptr == ent )
			continue;

		if ( !strcmp( ent->GetName(), entityName ) )
		{
			ents.push_back( ent );
		}
	}

	size_t index = Q_rand( &seed ) % ents.size();
	return gEntities[ents[index]->GetEntityIndex()];
}

Entities::IEntity* GameWorld::FindByClassnameRandom( const char* className )
{
	static int seed = 0x92;

	std::vector<Entities::IEntity*> ents;
	ents.reserve( 8U );

	for ( size_t index = 0; index < MaxEntities; index++ )
	{
		Entities::IEntity* ent = gEntities[index];

		if ( nullptr == ent )
			continue;

		if ( !strcmp( ent->GetClassname(), className ) )
		{
			ents.push_back( ent );
		}
	}

	size_t index = Q_rand( &seed ) % ents.size();
	return gEntities[ents[index]->GetEntityIndex()]; // am a little paranoid about returning ents[index], err...
}

std::vector<Entities::IEntity*> GameWorld::EntitiesInRadius( Vector position, float radius, bool bbox, bool unlinked ) const
{
	std::vector<Entities::IEntity*> ents;
	ents.reserve( 100 );

	for ( int i = 0; i < level.num_entities; i++ )
	{
		if ( nullptr == gEntities[i] )
			continue;

		Entities::IEntity* ent = gEntities[i];
		Vector origin = ent->GetCurrentOrigin();
		Vector mins = origin + ent->GetMins();
		Vector maxs = origin + ent->GetMaxs();

		if ( !unlinked && !ent->GetShared()->linked )
			continue;

		if ( bbox && (ent->GetMins().Length() || ent->GetMaxs().Length()) )
		{
			const Vector points[8] =
			{
				mins,
				Vector( mins.x, mins.y, maxs.z ),
				Vector( mins.x, maxs.y, mins.z ),
				Vector( mins.x, maxs.y, maxs.z ),
				Vector( maxs.x, mins.y, mins.z ),
				Vector( maxs.x, mins.y, maxs.z ),
				Vector( maxs.x, maxs.y, mins.z ),
				maxs
			};

			for ( int i = 0; i < 8; i++ )
			{
				if ( (position - points[i]).Length() <= radius )
				{
					ents.push_back( ent );
					break;
				}
			}
		}
		else
		{
			if ( (position - origin).Length() <= radius )
			{
				ents.push_back( ent );
			}
		}
	}

	return ents;
}

uint32_t GameWorld::PrecacheModel( const char* name )
{
	return FindConfigstringIndex( name, CS_MODELS, MAX_MODELS, true );
}

uint32_t GameWorld::PrecacheSound( const char* name )
{
	return FindConfigstringIndex( name, CS_SOUNDS, MAX_SOUNDS, true );
}

uint32_t GameWorld::PrecacheMaterial( const char* name )
{
	return FindConfigstringIndex( name, CS_SHADERS, MAX_CSSHADERS, true );
}

uint32_t GameWorld::FindConfigstringIndex( const char* name, uint32_t start, uint32_t max, bool create )
{
	int		i;
	char	s[MAX_STRING_CHARS];

	if ( !name || !name[0] ) 
	{
		return 0;
	}

	for ( i = 1; i < max; i++ ) 
	{
		gameImports->GetConfigString( start + i, s, sizeof( s ) );
		if ( !s[0] ) 
		{
			break;
		}
		
		if ( !strcmp( s, name ) ) 
		{
			return i;
		}
	}

	if ( !create ) 
	{
		return 0;
	}

	if ( i == max ) 
	{
		engine->Error( "GameWorld::FindConfigstringIndex: overflow" );
	}

	gameImports->SetConfigString( start + i, name );

	return i;
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
	SpawnRegistry::SpawnInfo info;
	Entities::IEntity* tent;
	int		flags;
	int		savedPing;
	int		accuracy_hits, accuracy_shots;
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];

	index = player->GetEntityIndex();
	client = player->GetClient();

	// Find a spawn point
	info = FindSpawnPoint<Entities::InfoPlayerStart>( client->ps.origin, false );

	info.spawnPointPosition.CopyToArray( player->GetShared()->currentOrigin );
	spawn_origin = info.spawnPointPosition;

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

	gameImports->GetUserInfo( index, userinfo, sizeof( userinfo ) );

	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );

	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) 
	{
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

	// health will count down towards max_health
	player->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] + 25;

	player->SetOrigin( spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;
	gameImports->GetUsercmd( client - level.clients, &player->GetClient()->pers.cmd );
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
			// Telefrag everything around self
			player->KillBox();

			// force the base weapon up
			client->ps.weapon = WeaponID_Fists;
			client->ps.weaponstate = WEAPON_READY;
			client->ps.ammo[WeaponID_Pistol] = 120;
			client->ps.ammoMag[WeaponID_Pistol] = 15;

			player->AddWeapon( gameWorld->CreateEntity<Entities::Weapon_Fists>() );

			// fire the targets of the spawn point
			if ( info.entityId != ENTITYNUM_NONE && gEntities[info.entityId] )
				gEntities[info.entityId]->UseTargets( player );

			// positively link the client, even if the command times are weird
			VectorCopy( player->GetClient()->ps.origin, player->GetShared()->currentOrigin );

			tent = CreateTempEntity( player->GetClient()->ps.origin, EV_PLAYER_TELEPORT_IN );
			tent->GetState()->clientNum = player->GetState()->clientNum;
			
			gameImports->LinkEntity( player );
		}
	}

	else 
	{
		// move players to intermission
		MoveClientToIntermission( player );
	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	player->GetClient()->pers.cmd.serverTime = level.time;

	ClientThink( player );

	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	if ( player->GetClient()->sess.spectatorState != SPECTATOR_FOLLOW ) 
	{
		ClientEndFrame( player );
	}

	// clear entity state values
	return BG_PlayerStateToEntityState( &client->ps, player->GetState(), qtrue );
}

void GameWorld::ClientRespawn( Entities::BasePlayer* player )
{
	player->CopyToBodyQue();
	player->takeDamage = true;
	SpawnClient( player );
}

void GameWorld::MoveClientToIntermission( Entities::BasePlayer* player )
{
	// take out of follow mode if needed
	if ( player->GetClient()->sess.spectatorState == SPECTATOR_FOLLOW )
	{
		player->StopFollowing();
	}

	FindIntermissionPoint();

	// move to the spot
	VectorCopy( level.intermission_origin, player->GetState()->origin );
	VectorCopy( level.intermission_origin, player->GetClient()->ps.origin );
	VectorCopy( level.intermission_angle, player->GetClient()->ps.viewangles );
	player->GetClient()->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset( player->GetClient()->ps.powerups, 0, sizeof( player->GetClient()->ps.powerups ) );

	player->GetClient()->ps.eFlags = 0;
	player->GetState()->eFlags = 0;
	player->GetState()->eType = ET_GENERAL;
	player->GetState()->modelindex = 0;
	player->GetState()->loopSound = 0;
	player->GetState()->event = 0;
	player->GetShared()->contents = 0;
}

void GameWorld::ClientThink( const uint16_t& clientNum )
{
	Entities::BasePlayer* player = static_cast<Entities::BasePlayer*>( gEntities[clientNum] );
	if ( player )
	{
		return ClientThink( player );
	}
}

void GameWorld::ClientThink( Entities::BasePlayer* player )
{
	gameImports->GetUsercmd( player->GetEntityIndex(), &player->GetClient()->pers.cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	player->GetClient()->lastCmdTime = level.time;

	if ( !(player->GetShared()->svFlags & SVF_BOT) && !g_synchronousClients.integer )
	{
		ClientThinkReal( player );
	}
}

void GameWorld::RunClient( Entities::BasePlayer* player )
{
	if ( !(player->GetShared()->svFlags & SVF_BOT) && !g_synchronousClients.integer )
		return;
	
	player->GetClient()->pers.cmd.serverTime = level.time;
	ClientThinkReal( player );
}

void GameWorld::ClientThinkReal( Entities::BasePlayer* player )
{
	gclient_t* client;
	pmove_t		pm;
	int			oldEventSequence;
	int			msec;
	usercmd_t* ucmd;

	client = player->GetClient();

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if ( client->pers.connected != CON_CONNECTED ) 
	{
		return;
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) 
	{
		ucmd->serverTime = level.time + 200;
	}

	if ( ucmd->serverTime < level.time - 1000 ) 
	{
		ucmd->serverTime = level.time - 1000;
	}

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) 
	{
		return;
	}

	if ( msec > 200 ) 
	{
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) 
	{
		gameImports->ConsoleVariable_Set( "pmove_msec", "8" );
		gameImports->ConsoleVariable_Update( &pmove_msec );
	}

	else if ( pmove_msec.integer > 33 ) 
	{
		gameImports->ConsoleVariable_Set( "pmove_msec", "33" );
		gameImports->ConsoleVariable_Update( &pmove_msec );
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) 
	{
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer - 1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	// check for exiting intermission
	if ( level.intermissiontime ) 
	{
		ClientIntermissionThink( player );
		return;
	}

	// spectators don't do much
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) 
	{
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
		{
			return;
		}

		SpectatorThink( player );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( player ) ) 
	{
		return;
	}

	// clear the rewards if time
	if ( level.time > client->rewardTime ) 
	{
		client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
	}

	if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	}

	else if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		client->ps.pm_type = PM_DEAD;
	}

	else 
	{
		client->ps.pm_type = PM_NORMAL;
	}

	client->ps.gravity = g_gravity.value;

	float speedLimit = client->ps.stats[STAT_HEALTH] / 100.0f;

	// set speed
	client->ps.speed = g_speed.value;

	// speedLimit can go from 0 to 1, map that to 0.5 and 1
	if ( speedLimit < 0.5f )
	{
		client->ps.speed *= (speedLimit + 0.5f);
	}

	if ( client->ps.powerups[PW_HASTE] ) 
	{
		client->ps.speed *= 1.3;
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset( &pm, 0, sizeof( pm ) );

	if ( player->flags & FL_FORCE_GESTURE ) 
	{
		player->flags &= ~FL_FORCE_GESTURE;
		player->GetClient()->pers.cmd.buttons |= Button_Gesture;
	}

	pm.ps = &client->ps;
	pm.cmd = *ucmd;

	if ( pm.ps->pm_type == PM_DEAD ) 
	{
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if ( player->GetShared()->svFlags & SVF_BOT ) 
	{
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	}
	else 
	{
		pm.tracemask = MASK_PLAYERSOLID;
	}

	pm.trace = Util::Trace;
	pm.pointcontents = Util::PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = (qboolean)((g_dmflags.integer & DF_NO_FOOTSTEPS) > 0);

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	VectorCopy( client->ps.origin, client->oldOrigin );

	Pmove( &pm );

	// save results of pmove
	if ( player->GetClient()->ps.eventSequence != oldEventSequence ) 
	{
		player->eventTime = level.time;
	}

	if ( g_smoothClients.integer ) 
	{
		BG_PlayerStateToEntityStateExtraPolate( &player->GetClient()->ps, player->GetState(), player->GetClient()->ps.commandTime, qtrue );
	}

	else 
	{
		BG_PlayerStateToEntityState( &player->GetClient()->ps, player->GetState(), qtrue );
	}

	SendPendingPredictableEvents( player );

	if ( !(player->GetClient()->ps.eFlags & EF_FIRING) ) 
	{
		client->fireHeld = qfalse; // for grapple
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( player->GetState()->pos.trBase, player->GetShared()->currentOrigin );

	VectorCopy( pm.mins, player->GetShared()->mins );
	VectorCopy( pm.maxs, player->GetShared()->maxs );

	player->waterLevel = pm.waterlevel;
	player->waterType = pm.watertype;

	// execute client events
	ClientEvents( player, oldEventSequence );

	Vector playerOrigin = player->GetClient()->ps.origin;
	Vector amax = playerOrigin + playerMaxs;
	Vector amin = playerOrigin + playerMins;

	// Set state origin for linking
	// https://github.com/Admer456/ioq3-burek/issues/34
	player->SetOrigin( playerOrigin );

	// link entity now, after any personal teleporters have been used
	gameImports->LinkEntity( player );

	// Unset it
	player->SetOrigin( Vector::Zero );

	amax.CopyToArray( player->GetShared()->absmax );
	amin.CopyToArray( player->GetShared()->absmin );

	if ( !player->GetClient()->noclip ) 
	{
		TouchTriggers( player );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( player->GetClient()->ps.origin, player->GetShared()->currentOrigin );

	////test for solid areas in the AAS file
	//BotTestAAS( player->GetShared()->currentOrigin );

	// touch other objects
	ClientImpacts( player, &pm );

	// save results of triggers and client events
	if ( player->GetClient()->ps.eventSequence != oldEventSequence ) 
	{
		player->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	client->oldInteractionButtons = client->interactionButtons;
	client->interactionButtons = ucmd->interactionButtons;
	client->latchedInteractionButtons |= client->interactionButtons & ~client->oldInteractionButtons;

	player->UpdateWeapon();

	if ( client->interactionButtons & Interaction_Use )
	{
		player->PlayerUse();
	}

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime ) 
		{
			// forcerespawn is to prevent users from waiting out powerups
			if ( g_forcerespawn.integer > 0 &&
				 (level.time - client->respawnTime) > g_forcerespawn.integer * 1000 ) 
			{
				ClientRespawn( player );
				return;
			}

			// pressing attack or use is the normal respawn method
			if ( ucmd->interactionButtons & (Interaction_PrimaryAttack | Interaction_Use | Interaction_UseItem) ) 
			{
				ClientRespawn( player );
			}
		}
		return;
	}

	// perform once-a-second actions
	ClientTimerActions( player, msec );
}

void GameWorld::ClientTimerActions( Entities::BasePlayer* player, int msec )
{
	gclient_t* client;

	client = player->GetClient();
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

		if ( player->health < 40 && player->health >= 1 )
		{
			player->health++;
			client->ps.stats[STAT_HEALTH]++;
		}

		// regenerate
		if ( client->ps.powerups[PW_REGEN] ) {
			if ( player->health < client->ps.stats[STAT_MAX_HEALTH] ) {
				player->health += 15;
				if ( player->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1 ) {
					player->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
				}
				player->AddEvent( EV_POWERUP_REGEN, 0 );
			}
			else if ( player->health < client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
				player->health += 5;
				if ( player->health > client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
					player->health = client->ps.stats[STAT_MAX_HEALTH] * 2;
				}
				player->AddEvent( EV_POWERUP_REGEN, 0 );
			}
		}
		else {
			// count down health when over max
			if ( player->health > client->ps.stats[STAT_MAX_HEALTH] ) {
				player->health--;
			}
		}

		// count down armor when over max
		if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
			client->ps.stats[STAT_ARMOR]--;
		}
	}
}

void GameWorld::ClientImpacts( Entities::BasePlayer* player, pmove_t* pm )
{
	int		i, j;
	trace_t	trace;
	Entities::IEntity* other;

	memset( &trace, 0, sizeof( trace ) );

	for ( i = 0; i < pm->numtouch; i++ ) 
	{
		for ( j = 0; j < i; j++ ) 
		{
			if ( pm->touchents[j] == pm->touchents[i] ) 
			{
				break;
			}
		}

		if ( j != i ) 
		{
			continue;	// duplicated
		}
		
		other = gEntities[pm->touchents[i]];

		if ( (player->GetShared()->svFlags & SVF_BOT) )
		{
			player->Touch( other, &trace );
		}

		other->Touch( player, &trace );
	}
}

void GameWorld::ClientEvents( Entities::BasePlayer* player, int oldEventSequence )
{
	using WeaponEvent = Entities::WeaponEvent;

	int		i, j;
	int		event;
	gclient_t* client;
	int		damage;
	vec3_t	origin, angles;
	//	qboolean	fired;
	gitem_t* item;
	//gentity_t* drop;

	client = player->GetClient();

	if ( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS ) 
	{
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}
	
	for ( i = oldEventSequence; i < client->ps.eventSequence; i++ ) 
	{
		event = client->ps.events[i & (MAX_PS_EVENTS - 1)];

		switch ( event ) {
		case EV_FALL_MEDIUM:
		case EV_FALL_FAR:
			if ( player->GetState()->eType != ET_PLAYER ) 
			{
				break;		// not in the player model
			}
			
			if ( g_dmflags.integer & DF_NO_FALLING ) 
			{
				break;
			}
			
			damage = player->GetClient()->ps.fallDamage;

			//player->pain_debounce_time = level.time + 200;	// no normal pain sound
			player->TakeDamage( nullptr, nullptr, 0, damage );
			break;

		case EV_FIRE_WEAPON:
			player->FireWeapon();
			break;

		case EV_WEAPON_PRIMARY:		player->SendWeaponEvent( WeaponEvent::WE_DoPrimaryAttack ); break;
		case EV_WEAPON_SECONDARY:	player->SendWeaponEvent( WeaponEvent::WE_DoSecondaryAttack ); break;
		case EV_WEAPON_TERTIARY:	player->SendWeaponEvent( WeaponEvent::WE_DoTertiaryAttack ); break;
		case EV_WEAPON_RELOAD:		player->SendWeaponEvent( WeaponEvent::WE_DoReload ); break;
		case EV_WEAPON_HOLSTER:		player->SendWeaponEvent( WeaponEvent::WE_DoHolster ); break;
		case EV_WEAPON_DRAW:		player->SendWeaponEvent( WeaponEvent::WE_DoDraw ); break;
		case EV_PLAYERUSE:			player->PlayerUse(); break;

			// TODO: Handle this event
		case EV_USE_ITEM1:		// teleporter
			// drop flags in CTF
			item = NULL;
			j = 0;

			if ( player->GetClient()->ps.powerups[PW_REDFLAG] ) 
			{
				item = BG_FindItemForPowerup( PW_REDFLAG );
				j = PW_REDFLAG;
			}
			
			else if ( player->GetClient()->ps.powerups[PW_BLUEFLAG] ) 
			{
				item = BG_FindItemForPowerup( PW_BLUEFLAG );
				j = PW_BLUEFLAG;
			}
			
			else if ( player->GetClient()->ps.powerups[PW_NEUTRALFLAG] ) 
			{
				item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
				j = PW_NEUTRALFLAG;
			}

			// TODO: Implement Drop_Item
			/*if ( item ) 
			{
				//drop = Drop_Item( player, item, 0 );
				// decide how many seconds it has left
				drop->count = (player->GetClient()->ps.powerups[j] - level.time) / 1000;
				if ( drop->count < 1 ) 
				{
					drop->count = 1;
				}

				player->GetClient()->ps.powerups[j] = 0;
			}*/

			//SelectSpawnPoint( player->GetClient()->ps.origin, origin, angles, qfalse );
			//TeleportPlayer( player, origin, angles );
			break;

		case EV_USE_ITEM2:		// medkit
			player->health = player->GetClient()->ps.stats[STAT_MAX_HEALTH] + 25;
			break;

		default:
			break;
		}
	}
}

bool GameWorld::ClientInactivityTimer( Entities::BasePlayer* player )
{
	gclient_t* client = player->GetClient();

	if ( !g_inactivity.integer ) 
	{
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = false;
	}
	
	else if ( client->pers.cmd.forwardmove ||
			  client->pers.cmd.rightmove ||
			  client->pers.cmd.upmove ||
			  (client->pers.cmd.buttons & Button_Attack) ) 
	{
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = false;
	}
	
	else if ( !client->pers.localClient ) 
	{
		if ( level.time > client->inactivityTime ) 
		{
			gameImports->DropClient( client - level.clients, "Dropped due to inactivity" );
			return false;
		}
		
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) 
		{
			client->inactivityWarning = true;
			gameImports->SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}

	return true;
}

void GameWorld::SendPendingPredictableEvents( Entities::BasePlayer* player )
{
	playerState_t* ps = &player->GetClient()->ps;
	Entities::IEntity* tempEnt = nullptr;
	int event;
	int seq;
	int extEvent;
	int number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		tempEnt = CreateTempEntity( ps->origin, event );
		number = tempEnt->GetEntityIndex();
		BG_PlayerStateToEntityState( ps, tempEnt->GetState(), qtrue );
		tempEnt->GetState()->number = number;
		tempEnt->GetState()->eType = ET_EVENTS + event;
		tempEnt->GetState()->eFlags |= EF_PLAYER_EVENT;
		tempEnt->GetState()->otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		tempEnt->GetShared()->svFlags |= SVF_NOTSINGLECLIENT;
		tempEnt->GetShared()->singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

void GameWorld::FindIntermissionPoint()
{
	Entities::IEntity* ent, *target;
	vec3_t dir;

	// find the intermission spot
	ent = FindByClassname( "info_player_intermission", nullptr );

	if ( !ent ) 
	{	// the map creator forgot to put in an intermission point...
		//SelectSpawnPoint( vec3_origin, level.intermission_origin, level.intermission_angle, qfalse );
	}
	else 
	{
		VectorCopy( ent->GetState()->origin, level.intermission_origin );
		VectorCopy( ent->GetState()->angles, level.intermission_angle );
		
		// if it has a target, look towards it
		if ( ent->GetTarget() ) 
		{
			target = FindByNameRandom( ent->GetTarget() );
			
			if ( target ) 
			{
				VectorSubtract( target->GetState()->origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}
}

void GameWorld::ClientIntermissionThink( Entities::BasePlayer* player )
{
	gclient_t* client = player->GetClient();

	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

	client->oldInteractionButtons = client->interactionButtons;
	client->interactionButtons = client->pers.cmd.interactionButtons;

	if ( client->interactionButtons & (Interaction_PrimaryAttack | Interaction_Use) & (client->oldInteractionButtons ^ client->interactionButtons) )
	{
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit = qtrue;
	}
}

void GameWorld::SpectatorThink( Entities::BasePlayer* player )
{
	pmove_t	pm;
	gclient_t* client;
	usercmd_t* ucmd;

	client = player->GetClient();
	ucmd = &client->pers.cmd;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW || !(client->ps.pm_flags & PMF_FOLLOW) ) 
	{
		if ( client->sess.spectatorState == SPECTATOR_FREE ) 
		{
			if ( client->noclip ) 
			{
				client->ps.pm_type = PM_NOCLIP;
			}

			else 
			{
				client->ps.pm_type = PM_SPECTATOR;
			}
		}

		else 
		{
			client->ps.pm_type = PM_FREEZE;
		}

		client->ps.speed = 400;	// faster than normal

		// set up for pmove
		memset( &pm, 0, sizeof( pm ) );
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = Util::Trace;
		pm.pointcontents = Util::PointContents;

		// perform a pmove
		Pmove( &pm );
		// save results of pmove
		VectorCopy( client->ps.origin, player->GetState()->origin );

		TouchTriggers( player );
		gameImports->UnlinkEntity( player );
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	client->oldInteractionButtons = client->interactionButtons;
	client->interactionButtons = ucmd->interactionButtons;

	// attack button cycles through spectators
	if ( (client->interactionButtons & Interaction_PrimaryAttack) && !(client->oldInteractionButtons & Interaction_PrimaryAttack) ) 
	{
		player->FollowCycle( 1 );
	}
}

void GameWorld::ClientEndFrame( const uint16_t& clientNum )
{
	Entities::BasePlayer* player = static_cast<Entities::BasePlayer*>(gEntities[clientNum]);
	if ( player )
	{
		return ClientEndFrame( player );
	}
}

void GameWorld::ClientEndFrame( Entities::BasePlayer* player )
{
	int	i;

	// TODO: Implement spectator thinking first, then SpectatorClientEndFrame
	if ( player->GetClient()->sess.sessionTeam == TEAM_SPECTATOR ) 
	{
		return SpectatorClientEndFrame( player );
	}

	// turn off any expired powerups
	for ( i = 0; i < MAX_POWERUPS; i++ ) 
	{
		if ( player->GetClient()->ps.powerups[i] < level.time )
		{
			player->GetClient()->ps.powerups[i] = 0;
		}
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) 
	{
		return;
	}

	// burn from lava, etc
	player->WorldEffects();

	// apply all the damage taken this frame
	player->ApplyDamage();

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - player->GetClient()->lastCmdTime > 1000 )
	{
		player->GetClient()->ps.eFlags |= EF_CONNECTION;
	}
	
	else 
	{
		player->GetClient()->ps.eFlags &= ~EF_CONNECTION;
	}

	player->GetClient()->ps.stats[STAT_HEALTH] = player->health;	// FIXME: get rid of ent->health...

	//G_SetClientSound( ent );

	// set the latest infor
	if ( g_smoothClients.integer ) 
	{
		BG_PlayerStateToEntityStateExtraPolate( &player->GetClient()->ps, player->GetState(), player->GetClient()->ps.commandTime, qtrue );
	}

	else 
	{
		BG_PlayerStateToEntityState( &player->GetClient()->ps, player->GetState(), qtrue );
	}

	SendPendingPredictableEvents( player );
}

void GameWorld::SpectatorClientEndFrame( Entities::BasePlayer* player )
{
	gclient_t* cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( player->GetClient()->sess.spectatorState == SPECTATOR_FOLLOW ) 
	{
		int		clientNum, flags;

		clientNum = player->GetClient()->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) 
		{
			clientNum = level.follow1;
		}
		
		else if ( clientNum == -2 ) 
		{
			clientNum = level.follow2;
		}
		
		if ( clientNum >= 0 ) 
		{
			cl = &level.clients[clientNum];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) 
			{
				flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (player->GetClient()->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				player->GetClient()->ps = cl->ps;
				player->GetClient()->ps.pm_flags |= PMF_FOLLOW;
				player->GetClient()->ps.eFlags = flags;
				return;
			}
		}

		if ( player->GetClient()->ps.pm_flags & PMF_FOLLOW ) 
		{
			// drop them to free spectators unless they are dedicated camera followers
			if ( player->GetClient()->sess.spectatorClient >= 0 ) 
			{
				player->GetClient()->sess.spectatorState = SPECTATOR_FREE;
			}

			//ClientBegin( player->GetEntityIndex() );
		}
	}

	if ( player->GetClient()->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
	{
		player->GetClient()->ps.pm_flags |= PMF_SCOREBOARD;
	}
	
	else 
	{
		player->GetClient()->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

SpawnRegistry* GameWorld::GetSpawnRegistry()
{
	return &spawnRegistry;
}

bool GameWorld::ParseKeyValues()
{
	char keyname[MAX_TOKEN_CHARS];
	char com_token[MAX_TOKEN_CHARS];

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	// Parse the opening brace
	if ( !engine->GetEntityToken( com_token, sizeof( com_token ) ) ) 
	{
		// End of spawn string
		return false;
	}
	
	if ( com_token[0] != '{' ) 
	{
		G_Error( "GameWorld::ParseKeyValues: found %s when expecting {", com_token );
	}

	KeyValueLibrary kvLibrary;

	// Go through all the key / value pairs
	while ( true ) 
	{
		// Parse key
		if ( !engine->GetEntityToken( keyname, sizeof( keyname ) ) ) 
		{
			engine->Error( "GameWorld::ParseKeyValues: EOF without closing brace" );
		}

		if ( keyname[0] == '}' ) 
		{
			break;
		}

		// Parse value	
		if ( !engine->GetEntityToken( com_token, sizeof( com_token ) ) ) 
		{
			engine->Error( "GameWorld::ParseKeyValues: EOF without closing brace" );
		}

		if ( com_token[0] == '}' ) 
		{
			engine->Error( "GameWorld::ParseKeyValues: closing brace without data" );
		}

		if ( level.numSpawnVars == MAX_SPAWN_VARS ) 
		{
			engine->Error( "GameWorld::ParseKeyValues: MAX_SPAWN_VARS" );
		}

		char* key = level.spawnVars[level.numSpawnVars][0] = AddKeyvalueToken( keyname );
		char* value = level.spawnVars[level.numSpawnVars][1] = AddKeyvalueToken( com_token );

		kvLibrary.AddKeyValue( key, value );

		level.numSpawnVars++;
	}

	keyValueLibraries.push_back( kvLibrary );
	return true;
}

char* GameWorld::AddKeyvalueToken( const char* string )
{
	int l;
	char* dest;

	l = strlen( string );

	if ( level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) 
	{
		engine->Error( "G_AddSpawnVarToken: MAX_SPAWN_VARS_CHARS" );
	}

	dest = level.spawnVarChars + level.numSpawnVarChars;
	memcpy( dest, string, l + 1 );

	level.numSpawnVarChars += l + 1;

	return dest;
}
