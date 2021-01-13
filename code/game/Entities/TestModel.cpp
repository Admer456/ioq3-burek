#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "BaseEntity.hpp"

#include "TestModel.hpp"

using namespace Entities;

DefineEntityClass( "test_model", TestModel, BaseEntity );

void TestModel::Spawn()
{
	BaseEntity::Spawn();

	SetModel( spawnArgs->GetCString( "model", "models/testmodel.iqm" ) );

	gameImports->LinkEntity( this );
}
