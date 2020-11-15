#pragma once

class KeyValueElement;
class KeyValueLibrary;

namespace Entities
{
	class IEntity;

	// This is the base entity, which has all properties 
	// typically needed for Quake entities
	class BaseQuakeEntity : public IEntity
	{
	public:
		virtual void	Spawn() override;
		virtual void	Precache() override { return; }
		
		virtual void	PreKeyValue();
		virtual void	PostKeyValue();
		virtual void	KeyValue() override;

		virtual void	Think() override;
		virtual void	Use( IEntity* activator, IEntity* caller, float value ) override { return; }
		virtual void	Touch( IEntity* other, trace_t* trace ) override { return; }
		virtual void	Blocked( IEntity* other ) override { return; }
		virtual void	TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) override { return; }
		virtual void	Die( IEntity* killer ) override { return; }

		// All entities have an entity index
		unsigned int	GetEntityIndex() const
		{
			return entityIndex;
		}

		void			SetEntityIndex( const size_t& index )
		{
			entityIndex = index;
		}

		// Every entity that has custom keyvalues will need to declare this too
		// THese are basically some sort of keyvalue handlers, and are used to
		// set & get keyvalues
		static KeyValueElement keyValues[];

		// Every entity will have its own sets of keyvalues,
		// for example:
		// "classname" "func_rotating"
		// "speed" "200"
		// Each pair is stored in here
		KeyValueLibrary* spawnArgs;

		entityState_t* GetState() override
		{
			return &shared.s;
		}

		entityShared_t* GetShared() override
		{
			return &shared.r;
		}

		sharedEntity_t* GetSharedEntity() override
		{
			return &shared;
		}

		// Getters and setters
		// name
		const char*		GetName() const;

		// classname, for whatever reason
		const char*		GetClassname() const;

		// target
		const char*		GetTarget() const;

		// origin
		const Vector&	GetOrigin() const;
		void			SetOrigin( const Vector& newOrigin );

		// angles
		const Vector&	GetAngles() const;
		void			SetAngles( const Vector& newAngles );

		// velocity
		const Vector&	GetVelocity() const;
		void			SetVelocity( const Vector& newVelocity );

		// mins
		const Vector&	GetMins() const;

		// maxs
		const Vector&	GetMaxs() const;

		// (mins + maxs)/2
		const Vector&	GetAverageOrigin() const;

		// spawnflags
		// TODO: create a class to store bitfields in it
		const int&		GetSpawnflags() const;
		void			SetSpawnflags( int flags );

		// Utilities
		// Triggers all entities that match the "target" field
		void			UseTargets( IEntity* activator ) override;

		// Triggers all entities whose targetname matches the parameter
		void			UseTargets( IEntity* activator, const char* targetName ) override;

		// Kills everything around this entity's bbox
		void			KillBox( bool onlyPlayers = false ) override;

		// Kills everything in size.xyz radius around this entity
		void			KillBox( const Vector& size, bool onlyPlayers = false ) override;

	public:
		std::string		className;
		std::string		targetName;
		std::string		target;

		// Transform
		Vector			origin{ Vector::Zero };
		Vector			angles{ Vector::Zero };
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

		// Water
		byte			waterLevel{ 0 };
		byte			waterType{ 0 };
		
		// Thinking
		float			nextThink{ 0 };

		// Flags
		int				flags{ 0 };				// FL_* variables
		int				spawnFlags{ 0 };		// 64 bits of
		int				spawnFlagsExtra{ 0 };	// spawnflags cuz' why not
	
	protected:
		unsigned int	entityIndex{ EntityIndexNotSet };

		sharedEntity_t	shared;
	};

	// Test base entity, will move a brush up'n'down
	class BaseEntity_Test : public BaseQuakeEntity
	{
	public: 
		void			Spawn() override;
		void			MyThink();
	};
}
