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
		
		constexpr static uint32_t SF_TriggerOnly = 1 << 0;

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

		constexpr static const char* BreakSounds[Material_Max * 2]
		{
			"sound/gibs/glass1.wav",
			"sound/gibs/glass2.wav",

			"sound/gibs/wood1.wav",
			"sound/gibs/wood2.wav",

			"sound/gibs/concrete1.wav",
			"sound/gibs/concrete2.wav",

			"sound/gibs/metal1.wav",
			"sound/gibs/metal2.wav"
		};

		void	Spawn() override;
		void	Precache() override;

		void	Use( IEntity* activator, IEntity* caller, float value ) override;
		void	TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) override;

		void	Break();

	protected:
		byte	materialType{};

		qhandle_t gibModels[3];
		qhandle_t sounds[2];
	};
}
