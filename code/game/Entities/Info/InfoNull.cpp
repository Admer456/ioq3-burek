#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "InfoNull.hpp"

using namespace Entities;

DefineEntityClass( "info_null", InfoNull, BaseEntity );

void InfoNull::Spawn()
{
	Remove();
}
