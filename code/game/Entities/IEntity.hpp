#pragma once

namespace Entities
{
	typedef void (IEntity::* thinkPointer     )( void );
	typedef void (IEntity::* usePointer       )( IEntity* activator, IEntity* caller, float value );
	typedef void (IEntity::* touchPointer     )( IEntity* other, void* trace );
	typedef void (IEntity::* blockedPointer   )( IEntity* other );
	typedef void (IEntity::* takeDamagePointer)( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage );
	typedef void (IEntity::* diePointer       )( IEntity* killer );

	//struct gentity_t; // Engine entity struct

	class IEntity
	{
	public:
		virtual void				Spawn() = 0;
		virtual void				Precache() = 0;
		virtual void				KeyValue() = 0;

		virtual void				Think() = 0;
		virtual void				Use( IEntity* activator, IEntity* caller, float value ) = 0;
		virtual void				Touch( IEntity* other ) = 0;
		virtual void				Blocked( IEntity* other ) = 0;
		virtual void				TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) = 0;
		virtual void				Die( IEntity* killer ) = 0;
		
	public: // Modular function utilities. I know they're a little bit ugly, but they work
		template< typename function >
		inline void					SetThink( function f )
		{
			thinkFunction = static_cast<void (IEntity::*)(void)>( f );
		}

		template< typename function >
		inline void					SetUse( function f )
		{
			useFunction = static_cast<void (IEntity::*)(IEntity* activator, IEntity* caller, float value)>(f);
		}

		template< typename function >
		inline void					SetTouch( function f )
		{
			touchFunction = static_cast<void (IEntity::*)(IEntity* other)>(f);
		}

		inline gentity_t*			GetEngineEntity()
		{
			return engineEntity;
		}

	protected:
		void						(IEntity::* thinkFunction)( void );
		void						(IEntity::* useFunction)( IEntity* activator, IEntity* caller, float value );
		void						(IEntity::* touchFunction)( IEntity* other );
		void						(IEntity::* blockedFunction)( IEntity* other );
		void						(IEntity::* takeDamageFunction)( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage );
		void						(IEntity::* dieFunction)( IEntity* killer );

		gentity_t*					engineEntity;
	};
}