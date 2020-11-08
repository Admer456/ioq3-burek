#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Maths/Vector.hpp"

#include "BasePlayer.hpp"

using namespace Entities;

void BasePlayer::Spawn()
{
	// Do nothing
}

gclient_t* BasePlayer::GetClient()
{
	return client;
}

void BasePlayer::SetClient( const gclient_t* playerClient )
{
	client = const_cast<gclient_t*>( playerClient );
}

const Vector& BasePlayer::GetClientViewAngle() const
{
	return Vector( client->ps.viewangles );
}

void BasePlayer::SetClientViewAngle( const Vector& newAngle )
{
	int			i;

	// set the delta angle
	for ( i = 0; i < 3; i++ ) 
	{
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT( newAngle[i] );
		client->ps.delta_angles[i] = cmdAngle - client->pers.cmd.angles[i];
	}

	VectorCopy( newAngle, shared.s.angles );
	VectorCopy( shared.s.angles, client->ps.viewangles );
}

void BasePlayer::CopyToBodyQue()
{
	IEntity* body;
	int contents;

	gameImports->UnlinkEntity( this );

	contents = gameImports->PointContents( GetOrigin(), -1 );
	if ( contents & CONTENTS_NODROP )
	{
		return;
	}

	// Todo: do the rest
	// like body = gameWorld->CreateEntity<PlayerCorpse>();
}

void BasePlayer::AddEvent( int event, int eventParameter )
{
	int	bits = 0;

	if ( !event ) 
	{
		G_Printf( "G_AddEvent: zero event added for entity %i\n", GetEntityIndex() );
		return;
	}

	// clients need to add the event in playerState_t instead of entityState_t
	if ( client ) 
	{
		bits = client->ps.externalEvent & EV_EVENT_BITS;
		bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;
		client->ps.externalEvent = event | bits;
		client->ps.externalEventParm = eventParameter;
		client->ps.externalEventTime = level.time;
	}
	
	else 
	{
		bits = shared.s.event & EV_EVENT_BITS;
		bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;
		shared.s.event = event | bits;
		shared.s.eventParm = eventParameter;
	}
	
	eventTime = level.time;
}
