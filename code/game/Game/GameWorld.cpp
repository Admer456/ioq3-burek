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

void GameWorld::TouchTriggers( Entities::IEntity* ent )
{

}

Entities::IEntity* GameWorld::FindByName( const char* entityName, Entities::IEntity* lastEntity )
{

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
		MoveClientToIntermission( player );
	}
	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	player->GetClient()->pers.cmd.serverTime = level.time;

	// TODO: ClientThink in GameWorld for IEntities
	ClientThink( player->GetEntityIndex() );

	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	if ( player->GetClient()->sess.spectatorState != SPECTATOR_FOLLOW ) 
	{
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
	if ( level.intermissiontime ) 
	{
		ClientIntermissionThink( player );
		return;
	}

	// spectators don't do much
	// TODO: Implement SpectatorThink
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

	//// check for the hit-scan gauntlet, don't let the action
	//// go through as an attack unless it actually hits something
	
	// gauntletHit will be handled elsewhere
	//if ( client->ps.weapon == WP_GAUNTLET && !(ucmd->buttons & BUTTON_TALK) &&
	//	 (ucmd->buttons & BUTTON_ATTACK) && client->ps.weaponTime <= 0 ) 
	//{
	//	pm.gauntletHit = CheckGauntletAttack( player );
	//}

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
	player->CopyToBodyQue();
	SpawnClient( player );
}

void GameWorld::ClientTimerActions( Entities::BasePlayer* player, int msec )
{
	gclient_t* client;

	client = player->GetClient();
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

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
	int		i, j;
	int		event;
	gclient_t* client;
	int		damage;
	vec3_t	origin, angles;
	//	qboolean	fired;
	gitem_t* item;
	gentity_t* drop;

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
			
			if ( event == EV_FALL_FAR ) 
			{
				damage = 10;
			}
			
			else 
			{
				damage = 5;
			}
			
			//player->pain_debounce_time = level.time + 200;	// no normal pain sound
			player->TakeDamage( nullptr, nullptr, 0, damage );
			break;

		case EV_FIRE_WEAPON:
			player->FireWeapon();
			break;

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

			if ( item ) 
			{
				//drop = Drop_Item( player, item, 0 );
				// decide how many seconds it has left
				drop->count = (player->GetClient()->ps.powerups[j] - level.time) / 1000;
				if ( drop->count < 1 ) 
				{
					drop->count = 1;
				}

				player->GetClient()->ps.powerups[j] = 0;
			}

			SelectSpawnPoint( player->GetClient()->ps.origin, origin, angles, qfalse );
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
			  (client->pers.cmd.buttons & BUTTON_ATTACK) ) 
	{
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = false;
	}
	
	else if ( !client->pers.localClient ) 
	{
		if ( level.time > client->inactivityTime ) 
		{
			trap_DropClient( client - level.clients, "Dropped due to inactivity" );
			return false;
		}
		
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) 
		{
			client->inactivityWarning = true;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}

	return true;
}

void GameWorld::SendPendingPredictableEvents( Entities::BasePlayer* player )
{
	playerState_t* ps = &player->GetClient()->ps;
	Entities::IEntity* tempEnt = nullptr;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		//// create temporary entity for event
		//tempEnt = G_TempEntity( ps->origin, event );
		//number = tempEnt->GetState()->number;
		//BG_PlayerStateToEntityState( ps, tempEnt->GetState(), qtrue );
		//tempEnt->GetState()->number = number;
		//tempEnt->GetState()->eType = ET_EVENTS + event;
		//tempEnt->GetState()->eFlags |= EF_PLAYER_EVENT;
		//tempEnt->GetState()->otherEntityNum = ps->clientNum;
		//// send to everyone except the client who generated the event
		//tempEnt->GetShared()->svFlags |= SVF_NOTSINGLECLIENT;
		//tempEnt->GetShared()->singleClient = ps->clientNum;
		//// set back external event
		//ps->externalEvent = extEvent;
	}
}

void GameWorld::MoveClientToIntermission( Entities::BasePlayer* player )
{
	// take out of follow mode if needed
	if ( player->GetClient()->sess.spectatorState == SPECTATOR_FOLLOW ) {
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

void GameWorld::ClientIntermissionThink( Entities::BasePlayer* player )
{
	gclient_t* client = player->GetClient();

	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

	if ( client->buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) & (client->oldbuttons ^ client->buttons) ) 
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
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove( &pm );
		// save results of pmove
		VectorCopy( client->ps.origin, player->GetState()->origin );

		TouchTriggers( player );
		gameImports->UnlinkEntity( player );
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	// attack button cycles through spectators
	if ( (client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK) ) 
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
	//P_WorldEffects( player );

	// apply all the damage taken this frame
	//P_DamageFeedback( ent );

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

			ClientBegin( player->GetClient() - level.clients );
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
