#pragma once

#include <vector>

class EntityClassInfo;

namespace Components
{
	class IComponent;
}

namespace Entities
{
	class IEntity;

	typedef void (IEntity::* thinkPointer     )( void );
	typedef void (IEntity::* usePointer       )( IEntity* activator, IEntity* caller, float value );
	typedef void (IEntity::* touchPointer     )( IEntity* other, trace_t* trace );
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
		virtual void				Touch( IEntity* other, trace_t* trace ) = 0;
		virtual void				Blocked( IEntity* other ) = 0;
		virtual void				TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) = 0;
		virtual void				Die( IEntity* killer ) = 0;
		
		virtual void				Remove() = 0; // Mark for removal

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

			touchFunction = static_cast<void (IEntity::*)(IEntity* other, trace_t* trace)>(f);
		}

		// entityState and entityShared are something all ents have in common
		virtual entityState_t*		GetState() = 0;
		virtual entityShared_t*		GetShared() = 0;
		virtual sharedEntity_t*		GetSharedEntity() = 0;

		// Getters and setters
 		// name
		virtual const char*			GetName() const = 0;
		
		// classname, for whatever reason
		virtual const char*			GetClassname() const = 0;

		// target
		virtual const char*			GetTarget() const = 0;

		// origin
		virtual Vector				GetOrigin() const = 0;
		virtual void				SetOrigin( const Vector& newOrigin ) = 0;

		// currentOrigin (a.k.a. server origin)
		virtual Vector				GetCurrentOrigin() const = 0;
		virtual void				SetCurrentOrigin( const Vector& newOrigin ) = 0;

		// angles
		virtual Vector				GetAngles() const = 0;
		virtual void				SetAngles( const Vector& newAngles ) = 0;

		// currentAngles
		virtual Vector				GetCurrentAngles() const = 0;
		virtual void				SetCurrentAngles( const Vector& newAngles ) = 0;

		// velocity
		virtual Vector				GetVelocity() const = 0;
		virtual void				SetVelocity( const Vector& newVelocity ) = 0;

		// mins
		virtual Vector				GetMins() const = 0;

		// maxs
		virtual Vector				GetMaxs() const = 0;

		// (mins + maxs)/2
		virtual Vector				GetAverageOrigin() const = 0;

		// spawnflags
		// TODO: create a class to store bitfields in it
		virtual const int&			GetSpawnflags() const = 0;
		virtual void				SetSpawnflags( int newFlags ) = 0;

		// flags
		virtual const int&			GetFlags() const = 0;
		virtual void				SetFlags( int newFlags ) = 0;

		// Utilities
		// Triggers all entities that match the "target" field
		virtual void				UseTargets( IEntity* activator ) = 0;
		// Triggers all entities whose targetname matches the parameter
		virtual void				UseTargets( IEntity* activator, const char* targetName ) = 0;
		
		// Kills everything around this entity's bbox
		virtual void				KillBox( bool onlyPlayers = false ) = 0;
		// Kills everything in size.xyz radius around this entity
		virtual void				KillBox( const Vector& size, bool onlyPlayers = false ) = 0;

		// Class comparison
		virtual bool				IsClass( const EntityClassInfo& eci ) = 0;
		virtual bool				IsSubclassOf( const EntityClassInfo& eci ) = 0;
		virtual EntityClassInfo*	GetClassInfo() = 0;

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
		void						(IEntity::* touchFunction)( IEntity* other, trace_t* trace );
		void						(IEntity::* blockedFunction)( IEntity* other );
		void						(IEntity::* takeDamageFunction)( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage );
		void						(IEntity::* dieFunction)( IEntity* killer );
	};

	constexpr unsigned int IEntitySize = sizeof( IEntity );
}