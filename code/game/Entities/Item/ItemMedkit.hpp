#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class ItemMedkit : public BaseEntity
	{
	public:
		DeclareEntityClass();

		enum Type
		{
			Standard,
			Burek,
			Cevapi
		};

		inline static sfxHandle_t HealingSound{ 0U };
		inline static sfxHandle_t EatingSound{ 0U };

		static constexpr int HealAmount = 30;

		void		Spawn() override;
		void		Precache() override;

		void		HealingTouch( IEntity* other, trace_t* trace );
		void		HealingUse( IEntity* activator, IEntity* caller, float value );

		uint32_t	ObjectFlags() override { return OF_ImpulseUse; }

	protected:
		bool		isFood{ false };
	};
}
