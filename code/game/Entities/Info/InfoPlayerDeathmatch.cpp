#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Maths/Vector.hpp"
#include "Components/IComponent.hpp"

#include "InfoPlayerDeathmatch.hpp"

using namespace Entities;

DefineEntityClass( "info_player_deathmatch", InfoPlayerDeathmatch, BaseEntity );

void InfoPlayerDeathmatch::Spawn()
{
	// Get origin vector and jam it into entityState
	Vector origin = spawnArgs->GetVector( "origin", Vector::Zero );
	SetOrigin( origin );

	// Add this spawnpoint's location to the registry
	gameWorld->GetSpawnRegistry()->Add( this );

	// Don't waste any entity slots
	Remove();
}
