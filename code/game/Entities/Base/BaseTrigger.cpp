#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "BaseTrigger.hpp"

using namespace Entities;

DefineAbstractEntityClass( BaseTrigger, BaseEntity );

void BaseTrigger::Spawn()
{
	BaseEntity::Spawn();

	GetShared()->contents |= CONTENTS_TRIGGER;
	GetShared()->svFlags = SVF_NOCLIENT;

	gameImports->LinkEntity( this );

	GetState()->clipFlags |= ClipFlag_Trigger;
}
