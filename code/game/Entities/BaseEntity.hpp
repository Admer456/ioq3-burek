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
		virtual void	Touch( IEntity* other ) override { return; }
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

	public:
		float			nextThink{ 0 };

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
