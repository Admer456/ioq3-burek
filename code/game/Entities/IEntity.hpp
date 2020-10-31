#pragma once

#include <vector>

namespace Components
{
	class IComponent;
}

namespace Entities
{
	class IEntity;

	typedef void (IEntity::* thinkPointer     )( void );
	typedef void (IEntity::* usePointer       )( IEntity* activator, IEntity* caller, float value );
	typedef void (IEntity::* touchPointer     )( IEntity* other, void* trace );
	typedef void (IEntity::* blockedPointer   )( IEntity* other );
	typedef void (IEntity::* takeDamagePointer)( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage );
	typedef void (IEntity::* diePointer       )( IEntity* killer );

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
		
		// All entities have an entity index
		virtual unsigned int		GetEntityIndex() const = 0;
		virtual void				SetEntityIndex( const size_t& index ) = 0;

		constexpr static size_t		EntityIndexNotSet = 1 << 31U;

	public: // Modular function utilities. I know they're a little bit ugly, but they work
		template<typename function>
		inline void					SetThink( function f )
		{
			thinkFunction = static_cast<thinkPointer>( f );
		}

		template<typename function>
		inline void					SetUse( function f )
		{
			useFunction = static_cast<void (IEntity::*)(IEntity* activator, IEntity* caller, float value)>(f);
		}

		template<typename function>
		inline void					SetTouch( function f )
		{
			touchFunction = static_cast<void (IEntity::*)(IEntity* other)>(f);
		}

		// entityState and entityShared are something all ents have in common
		virtual entityState_t* GetState() = 0;
		virtual entityShared_t* GetShared() = 0;
		virtual sharedEntity_t* GetSharedEntity() = 0;

		// Component interface

		// Use this if you're 100% sure you know a comp exists
		template<typename componentType>
		componentType* GetComponent()
		{
			componentType* c = nullptr;

			if ( components.empty() )
				return nullptr; 

			for ( auto component : components )
			{
				if ( c = dynamic_cast<componentType*>(component) )
				{
					return c;
				}
			}

			return nullptr;
		}
		
		// Use this if you're 100% sure you need a new comp
		template<typename componentType>
		componentType* CreateComponent()
		{
			componentType* comp = new componentType();

			components.push_back( comp );

			return comp;
		}
		
		// Use this if unsure; be careful since it's the slowest
		template<typename componentType>
		componentType* GetOrCreateComponent()
		{
			componentType* comp;
			if ( !(comp = GetComponent<componentType>()) )
			{
				comp = CreateComponent<componentType>();
			}

			return comp;
		}

	protected: // Component stuff
		std::vector<Components::IComponent*> components;

	protected: // Callbacks
		void						(IEntity::* thinkFunction)( void );
		void						(IEntity::* useFunction)( IEntity* activator, IEntity* caller, float value );
		void						(IEntity::* touchFunction)( IEntity* other );
		void						(IEntity::* blockedFunction)( IEntity* other );
		void						(IEntity::* takeDamageFunction)( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage );
		void						(IEntity::* dieFunction)( IEntity* killer );
	};

	constexpr unsigned int IEntitySize = sizeof( IEntity );
}