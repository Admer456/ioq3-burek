#include "Game/g_local.hpp"

#include "IEntity.hpp"
#include "BaseEntity.hpp"

using namespace Entities;

void BaseEntity_Test::Spawn()
{
	SetThink( &BaseEntity_Test::MyThink );
}

void BaseEntity_Test::MyThink()
{
	static float cycle = 0.0f;
	static float originalZ = engineEntity->r.currentOrigin[2];
	static float iterator = 0.008f;

	cycle += iterator;
	if ( fabs( cycle ) > 1.000f )
		iterator *= -1.0f;

	engineEntity->s.origin[2] = originalZ + cycle;
	G_SetOrigin( engineEntity, engineEntity->s.origin );
}