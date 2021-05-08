#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "EnvMirror.hpp"

using namespace Entities;

DefineEntityClass( "env_mirror", EnvMirror, BaseEntity );

void EnvMirror::Spawn()
{
	GetShared()->mins << Vector::Zero;
	GetShared()->maxs << Vector::Zero;
	gameImports->LinkEntity( this );

	GetState()->origin2 << GetOrigin(); // portal surfaces use origin2 as their camera
	GetState()->eType = ET_PORTAL;
	GetShared()->svFlags = SVF_PORTAL;
}
