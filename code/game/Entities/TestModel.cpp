#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "BaseEntity.hpp"

#include "TestModel.hpp"

using namespace Entities;

DefineEntityClass( "test_model", TestModel, BaseQuakeEntity );

void TestModel::Spawn()
{
	BaseQuakeEntity::Spawn();

	const char* model = spawnArgs->GetCString( "model", "models/testmodel.iqm" );
	int modelIndex = G_ModelIndex( const_cast<char*>( model ) );

	GetState()->modelindex = modelIndex;
	gameImports->LinkEntity( this );
}
