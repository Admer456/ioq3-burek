#include "Maths/Vector.hpp"
#include "g_local.hpp"
#include "Entities/BaseEntity.hpp"
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

	client->ps.stats[STAT_WEAPONS] = (1 << WP_MACHINEGUN);

	if ( g_gametype.integer == GT_TEAM ) 
	{
		client->ps.ammo[WP_MACHINEGUN] = 50;
	}

	else 
	{
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
			client->ps.weapon = WP_MACHINEGUN;
			client->ps.weaponstate = WEAPON_READY;
			
			// fire the targets of the spawn point
			spawnPoint->UseTargets( player );

			// select the highest weapon number available, after any spawn given items have fired
			client->ps.weapon = 1;

			for ( i = WP_NUM_WEAPONS - 1; i > 0; i-- ) 
			{
				if ( client->ps.stats[STAT_WEAPONS] & (1 << i) ) 
				{
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

	else 
	{
		// move players to intermission

		// TODO: MoveClientToIntermission for IEntities
		////MoveClientToIntermission( ent );
	}
	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	player->GetClient()->pers.cmd.serverTime = level.time;

	// TODO: ClientThink in GameWorld for IEntities
	ClientThink( player->GetEntityIndex() );

	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	if ( player->GetClient()->sess.spectatorState != SPECTATOR_FOLLOW ) {
		ClientEndFrame( player );
	}

	// clear entity state values
	return BG_PlayerStateToEntityState( &client->ps, player->GetState(), qtrue );
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
	player->GetClient()->lastCmdTime = level.time;

	if ( !g_synchronousClients.integer )
	{
		ClientThinkReal( player );
	}
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
		trap_Cvar_Set( "pmove_msec", "8" );
		trap_Cvar_Update( &pmove_msec );
	}

	else if ( pmove_msec.integer > 33 ) 
	{
		trap_Cvar_Set( "pmove_msec", "33" );
		trap_Cvar_Update( &pmove_msec );
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) 
	{
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer - 1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	// TODO: implement intermission think
	//if ( level.intermissiontime ) 
	//{
	//	ClientIntermissionThink( client );
	//	return;
	//}

	// spectators don't do much
	// TODO: Implement SpectatorThink
	//if ( client->sess.sessionTeam == TEAM_SPECTATOR ) 
	//{
	//	if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
	//{
	//		return;
	//	}
	//	SpectatorThink( player, ucmd );
	//	return;
	//}

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

	// set speed
	client->ps.speed = g_speed.value;

	if ( client->ps.powerups[PW_HASTE] ) 
	{
		client->ps.speed *= 1.3;
	}

	// Let go of the hook if we aren't firing
	if ( client->ps.weapon == WP_GRAPPLING_HOOK &&
		 client->hook && !(ucmd->buttons & BUTTON_ATTACK) ) 
	{
		Weapon_HookFree( client->hook );
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset( &pm, 0, sizeof( pm ) );

	// check for the hit-scan gauntlet, don't let the action
	// go through as an attack unless it actually hits something
	if ( client->ps.weapon == WP_GAUNTLET && !(ucmd->buttons & BUTTON_TALK) &&
		 (ucmd->buttons & BUTTON_ATTACK) && client->ps.weaponTime <= 0 ) 
	{
		pm.gauntletHit = CheckGauntletAttack( player );
	}

	if ( player->flags & FL_FORCE_GESTURE ) 
	{
		player->flags &= ~FL_FORCE_GESTURE;
		player->GetClient()->pers.cmd.buttons |= BUTTON_GESTURE;
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
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
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
		client->fireHeld = qfalse;		// for grapple
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( player->GetState()->pos.trBase, player->GetShared()->currentOrigin );

	VectorCopy( pm.mins, player->GetShared()->mins );
	VectorCopy( pm.maxs, player->GetShared()->maxs );

	player->waterLevel = pm.waterlevel;
	player->waterType = pm.watertype;

	// execute client events
	ClientEvents( player, oldEventSequence );

	// link entity now, after any personal teleporters have been used
	gameImports->LinkEntity( player );

	if ( !player->GetClient()->noclip ) 
	{
		TouchTriggers( player );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( player->GetClient()->ps.origin, player->GetShared()->currentOrigin );

	//test for solid areas in the AAS file
	BotTestAAS( player->GetShared()->currentOrigin );

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
			if ( ucmd->buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) ) 
			{
				ClientRespawn( player );
			}
		}
		return;
	}

	// perform once-a-second actions
	ClientTimerActions( player, msec );
}

void GameWorld::ClientRespawn( Entities::BasePlayer* player )
{

}

void GameWorld::ClientTimerActions( Entities::BasePlayer* player, int msec )
{

}

void GameWorld::ClientImpacts( Entities::BasePlayer* player, pmove_t* pm )
{

}

void GameWorld::ClientEvents( Entities::BasePlayer* player, int oldEventSequence )
{

}

bool GameWorld::ClientInactivityTimer( Entities::BasePlayer* player )
{
	return false;
}

void GameWorld::SendPendingPredictableEvents( Entities::BasePlayer* player )
{

}

unsigned int GameWorld::CheckGauntletAttack( Entities::BasePlayer* player )
{
	return 0;
}



void GameWorld::ClientEndFrame( const uint16_t& clientNum )
{

}

void GameWorld::ClientEndFrame( Entities::BasePlayer* player )
{
	return ClientEndFrame( player->GetEntityIndex() );
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
