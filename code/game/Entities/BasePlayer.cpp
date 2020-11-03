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
	for ( i = 0; i < 3; i++ ) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT( newAngle[i] );
		client->ps.delta_angles[i] = cmdAngle - client->pers.cmd.angles[i];
	}
	VectorCopy( newAngle, shared.s.angles );
	VectorCopy( shared.s.angles, client->ps.viewangles );
}


