#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class AudioAmbient : public BaseEntity
	{
	public:
		static constexpr uint32_t SF_StartOff = 1 << 0U;

		DeclareEntityClass();

		void		Spawn() override;
		void		Precache() override;

		void		AmbientUse( IEntity* activator, IEntity* caller, float value );

	protected:
		qhandle_t	sound{ 0 };
	};
}
