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

		// Engine variables. We must get rid of them one day; they're here only
		// because I haven't adapted the actual engine to not use gentity_t
		// One day, the engine won't have client entities, server entities,
		// game entities etc., but rather, everything will go through IEntity
		entityState_t	state;
		entityShared_t	shared;

	public:
		entityShared_t*	GetEngineShared() override { return &shared; }
		entityState_t*	GetState() override { return &state; }

	public:
		float			nextThink{ 0 };

		int				spawnFlags{ 0 };		// 64 bits of
		int				spawnFlagsExtra{ 0 };	// spawnflags cuz' why not
	};

	// Test base entity, will move a brush up'n'down
	class BaseEntity_Test : public BaseQuakeEntity
	{
	public: 
		void			Spawn() override;
		void			MyThink();
	};
}
