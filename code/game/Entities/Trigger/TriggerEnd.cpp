#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

namespace Entities
{
	class TriggerEnd : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override
		{
			SetModel( nullptr );
			gameImports->UnlinkEntity( this );
		}

		void Use( IEntity* activator, IEntity* caller, float value ) override
		{
			engine->SendConsoleCommand( EXEC_APPEND, "disconnect" );
		}
	};

	DefineEntityClass( "trigger_end", TriggerEnd, BaseEntity );
}
