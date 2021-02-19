#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "../AI_Common.hpp"
#include "Node.hpp"
#include "NavNode.hpp"

using namespace Entities;

DefineEntityClass( "ai_node_nav", NavNode, Node );
