#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Maths/Vector.hpp"

#include "BasePlayer.hpp"

extern char* ConcatArgs( int start );

using namespace Entities;

void BasePlayer::Command_Say( int mode, bool arg0 )
{
	char* p;

	if ( engine->ArgC() < 2 && !arg0 ) {
		return;
	}

	if ( arg0 )
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	//SanitizeChatText( p );

	// TODO: Implement GameWorld::Say
	//G_Say( ent, NULL, mode, p );
}

void BasePlayer::Command_Tell()
{

}

void BasePlayer::Command_Score()
{

}

void BasePlayer::Command_Give()
{

}

void BasePlayer::Command_God()
{
	const char* msg;

	//if ( !CheatsOk( ent ) ) 
	//{
	//	return;
	//}

	flags ^= FL_GODMODE;
	
	if ( !(flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gameImports->SendServerCommand( GetEntityIndex(), va( "print \"%s\"", msg ) );
}

void BasePlayer::Command_Notarget()
{
	const char* msg;

	//if ( !CheatsOk( ent ) ) 
	//{
	//	return;
	//}

	flags ^= FL_NOTARGET;

	if ( !(flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gameImports->SendServerCommand( GetEntityIndex(), va( "print \"%s\"", msg ) );
}

void BasePlayer::Command_Noclip()
{
	const char* msg;

	//if ( !CheatsOk( ent ) ) 
	//{
	//	return;
	//}

	if ( client->noclip )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	client->noclip = !client->noclip;

	gameImports->SendServerCommand( GetEntityIndex(), va( "print \"%s\"", msg ) );
}

void BasePlayer::Command_Kill()
{
	if ( client->sess.sessionTeam == TEAM_SPECTATOR )
		return;

	if ( health <= 0 )
		return;

	flags &= ~FL_GODMODE;
	client->ps.stats[STAT_HEALTH] = health = -999;

	TakeDamage( this, this, 0, 1000 );
}

void BasePlayer::Command_Team()
{

}

void BasePlayer::Command_Where()
{

}

void BasePlayer::Command_TeamTask()
{

}

void BasePlayer::Command_LevelShot()
{
	if ( !client->pers.localClient )
	{
		gameImports->SendServerCommand( GetEntityIndex(),
								"print \"The levelshot command must be executed by a local client\n\"" );
		return;
	}

	//if ( !CheatsOk( ent ) )
	//	return;

	// doesn't work in single player
	if ( g_gametype.integer == GT_SINGLE_PLAYER )
	{
		
		gameImports->SendServerCommand( GetEntityIndex(),
								"print \"Must not be in singleplayer mode for levelshot\n\"" );
		return;
	}

	//BeginIntermission();
	gameImports->SendServerCommand( GetEntityIndex(), "clientLevelShot" );
}

void BasePlayer::Command_Follow()
{

}

void BasePlayer::Command_CallVote()
{

}

void BasePlayer::Command_Vote()
{

}

void BasePlayer::Command_CallTeamVote()
{

}

void BasePlayer::Command_TeamVote()
{

}

void BasePlayer::Command_GameCommand()
{

}

void BasePlayer::Command_SetViewpos()
{

}

void BasePlayer::Command_Stats()
{

}

