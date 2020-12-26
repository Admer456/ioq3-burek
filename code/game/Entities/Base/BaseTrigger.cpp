#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "BaseTrigger.hpp"

using namespace Entities;

DefineAbstractEntityClass( BaseTrigger, BaseQuakeEntity );

void BaseTrigger::Spawn()
{
	BaseQuakeEntity::Spawn();

	GetShared()->contents |= CONTENTS_TRIGGER;
}
