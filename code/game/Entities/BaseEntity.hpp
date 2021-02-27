#pragma once

class KeyValueElement;
class KeyValueLibrary;

#include <string>

namespace Entities
{
	class IEntity;

	// This is the base entity, which has all properties 
	// typically needed for Quake entities
	class BaseEntity : public IEntity
	{
	public:
		DeclareEntityClass();

		virtual void	Spawn() override; // Gets called *while* entities are spawning
		virtual void	PostSpawn() override { return; } // Gets called after all entities have spawned

		virtual void	Precache() override { return; } // Gets called before Spawn
		virtual void	ParseKeyvalues() override; // Gets called before Precache; you may parse keyvalues anywhere tho'

		virtual void	Think() override;
		virtual void	Use( IEntity* activator, IEntity* caller, float value ) override;
		virtual void	Touch( IEntity* other, trace_t* trace ) override;
		virtual void	Blocked( IEntity* other ) override { return; }
		virtual void	TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) override { return; }
		virtual void	Die( IEntity* killer ) override { return; }

		virtual void	Remove() override;

		virtual void	OnClientBegin( int clientNum ) { return; } // Called when the client joins and spawns
		virtual void	OnClientDisconnect( int clientNum ) { return; } // Called when the client disconnects

		virtual void	OnPlayerDie( int clientNum ) { return; } // Called when a player dies

		// All entities have an entity index
		unsigned int	GetEntityIndex() const
		{
			return entityIndex;
		}

		void			SetEntityIndex( const size_t& index )
		{
			entityIndex = index;
		}

		// Every entity will have its own sets of keyvalues, for example:
		// "classname" "func_rotating"
		// "speed" "200"
		// Each pair is stored in here
		KeyValueLibrary* spawnArgs;

		entityState_t*	GetState() override
		{
			return &shared.s;
		}

		entityShared_t*	GetShared() override
		{
			return &shared.r;
		}

		sharedEntity_t*	GetSharedEntity() override
		{
			return &shared;
		}

		// Getters and setters
		// name
		const char*		GetName() const override;

		// classname, for whatever reason
		const char*		GetClassname() const override;

		// target
		const char*		GetTarget() const override;

		// same as gameWorld->FindByName( GetTarget() );
		IEntity*		GetTargetEntity() const;

		// target, target1, target2, target3 ... targetN
		std::vector<IEntity*> GetTargetEntities() const;

		// returns the entity which is targeting this entity
		IEntity*		GetTargetOf() const;

		// returns all entities that are targeting this one
		std::vector<IEntity*> GetAllTargetOf() const;

		// origin
		Vector			GetOrigin() const override;
		void			SetOrigin( const Vector& newOrigin ) override;

		// currentOrigin
		Vector			GetCurrentOrigin() const override;
		void			SetCurrentOrigin( const Vector& newOrigin ) override;

		// angles
		Vector			GetAngles() const override;
		void			SetAngles( const Vector& newAngles ) override;

		// currentAngles
		Vector			GetCurrentAngles() const override;
		void			SetCurrentAngles( const Vector& newAngles ) override;

		// velocity
		Vector			GetVelocity() const override;
		void			SetVelocity( const Vector& newVelocity ) override;

		// mins
		Vector			GetMins() const override;

		// maxs
		Vector			GetMaxs() const override;

		// (mins + maxs)/2
		Vector			GetAverageOrigin() const override;

		// Gets and sets the model index
		int				GetModel();
		void			SetModel( const char* modelPath );

		// spawnflags
		// TODO: create a class to store bitfields in it
		const int&		GetSpawnflags() const override;
		void			SetSpawnflags( int newFlags ) override;

		// flags
		const int&		GetFlags() const override;
		void			SetFlags( int newFlags ) override;

		// Utilities
		// Triggers all entities that match the "target" field
		void			UseTargets( IEntity* activator ) override;

		// Triggers all entities whose targetname matches the parameter
		void			UseTargets( IEntity* activator, const char* targetName ) override;

		// Kills everything around this entity's bbox
		void			KillBox( bool onlyPlayers = false ) override;

		// Kills everything in size.xyz radius around this entity
		void			KillBox( const Vector& size, bool onlyPlayers = false ) override;

		// Checks if this entity is inside another entity
		// If yes, then it returns that entity, else nullptr
		virtual BaseEntity* TestEntityPosition();

		bool			TryPushingEntity( IEntity* check, Vector move, Vector amove );

		// Adds an event to this entity, is overridden by BasePlayer
		virtual void	AddEvent( int event, int eventParameter );

		// Class comparison
		bool			IsClass( const EntityClassInfo& eci ) override final;
		bool			IsSubclassOf( const EntityClassInfo& eci ) override final;

		// Model stuff
		animHandle		GetAnimByName( const char* name );
		
		std::vector<Assets::ModelAnimation> anims;

	public:
		std::string		className;
		std::string		targetName;
		std::string		target;

		// These are in entityState and entityShared
		//// Transform
		//Vector			origin{ Vector::Zero };
		//Vector			angles{ Vector::Zero };
		Vector			velocity{ Vector::Zero };
		Vector			angularVelocity{ Vector::Zero };

		float			speed{ 0 };
		Vector			movedir{ Vector::Zero };

		// Damage
		int				health{ 0 };
		bool			takeDamage{ false };

		// Movement
		int				clipMask{ 0 };	// Brushes with this content value will be collided against when moving. 
										// Items and corpses do not collide against players, for instance
		float			physicsBounce{ 0 };	// 1.0 = continuous bounce, 0.0 = no bounce
		bool			isPhysicsObject{ false };// if true, it can be pushed by movers and fall off edges

		// Events
		int				eventTime{ 0 };	// events will be cleared EVENT_VALID_MSEC after set
		bool			freeAfterEvent{ false };
		bool			unlinkAfterEvent{ false };
		bool			neverFree{ false }; // if true, FreeEntity will only unlink
		bool			complexEvent{ false };

		// Water
		byte			waterLevel{ 0 };
		byte			waterType{ 0 };

		// Thinking
		float			nextThink{ 0 };

		// Flags
		int				flags{ 0 };				// FL_* flags (q_shared.hpp)
		int				spawnFlags{ 0 };		// entity-specific SF_* flags
		int				spawnFlagsExtra{ 0 };	// extra 32 spawnflags

		BaseEntity* 	chain{ nullptr };		// For parenting

	protected:
		virtual bool	CheckAndClearEvents();

	protected:
		unsigned int	entityIndex{ EntityIndexNotSet };

		sharedEntity_t	shared;
	};
}
