#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class EnvSprite : public BaseEntity
	{
	public:
		DeclareEntityClass();

		constexpr static uint32_t SF_StartOff = 1 << 0;
		constexpr static uint32_t SF_Once = 1 << 1;

		void		Spawn() override;
		void		Precache() override;

		void		SpriteUse( IEntity* activator, IEntity* caller, float value );

		void		SpriteToggle();

	private:
		float		spriteRadius{};
		float		spriteOpacity{};
		uint32_t	spriteHandle{};
		bool		spriteState{ true };
	};
}
