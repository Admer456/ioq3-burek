#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/BasePlayer.hpp"
#include "../qcommon/Assets/Models/ModelConfigData.hpp"

#include "FuncDynamic.hpp"

using namespace Entities;

DefineEntityClass( "func_dynamic", FuncDynamic, BaseEntity );

void FuncDynamic::Spawn()
{
	BaseEntity::Spawn();
}


