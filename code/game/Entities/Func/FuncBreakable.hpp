#pragma once

enum BreakMaterialTypes
{
	Material_Glass,
	Material_Wood,
	Material_Concrete,
	Material_Metal,

	Material_Max
};

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class FuncBreakable : public BaseEntity
	{
	public:
		DeclareEntityClass();
		
		// Cannot take damage from weapons etc., only destroy on trigger
		constexpr static uint32_t SF_TriggerOnly = 1 << 0;
		// Twitch when damaged
		constexpr static uint32_t SF_Twitch = 1 << 1;
		// Gib direction = attack direction
		constexpr static uint32_t SF_AttackDirection = 1 << 2;

		constexpr static const char* BreakGibs[Material_Max*3]
		{
			"models/gibs/glass1.iqm",
			"models/gibs/glass2.iqm",
			"models/gibs/glass3.iqm",

			"models/gibs/wood1.iqm",
			"models/gibs/wood2.iqm",
			"models/gibs/wood3.iqm",

			"models/gibs/concrete1.iqm",
			"models/gibs/concrete2.iqm",
			"models/gibs/concrete3.iqm",

			"models/gibs/metal1.iqm",
			"models/gibs/metal2.iqm",
			"models/gibs/metal3.iqm",
		};

		constexpr static const char* BreakSounds[Material_Max * 3]
		{
			"sound/debris/glass_bust1.wav",
			"sound/debris/glass_bust2.wav",
			"sound/debris/glass_bust3.wav",

			"sound/debris/wood_bust1.wav",
			"sound/debris/wood_bust2.wav",
			"sound/debris/wood_bust3.wav",

			"sound/debris/concrete_bust1.wav",
			"sound/debris/concrete_bust2.wav",
			"sound/debris/concrete_bust3.wav",

			"sound/debris/metal_bust1.wav",
			"sound/debris/metal_bust2.wav",
			"sound/debris/metal_bust3.wav"
		};

		void		Spawn() override;
		void		Precache() override;

		void		Use( IEntity* activator, IEntity* caller, float value ) override;
		void		TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) override;

		void		Break( IEntity* breaker );

	protected:
		Vector		gibDirection{ Vector::Zero };
		byte		materialType{};

		qhandle_t	gibModels[3];
		qhandle_t	sounds[3];

		qhandle_t	currentModel{};
	};
}
