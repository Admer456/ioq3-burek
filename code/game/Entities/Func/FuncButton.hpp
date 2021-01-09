#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;
	
	// A generic button
	// Triggers something when:
	// - used by the player
	// - shot (if health > 0)
	// - touched (if the flag is set)
	class FuncButton : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		constexpr static int SF_NoTouch = 1 << 0;
		constexpr static int SF_NoUse = 1 << 1;
		constexpr static int SF_DontMove = 1 << 2;

		void		Spawn() override;
		void		Precache() override;

		void		ButtonThink();
		void		ButtonUse( IEntity* activator, IEntity* caller, float value );
		void		ButtonTouch( IEntity* other, trace_t* trace );

	private:
		uint16_t	buttonSound{ 0 };
		float		resetTime{ 0.0f };
	};
}
