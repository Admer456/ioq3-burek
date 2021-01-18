#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class AudioMusicSet : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void	Spawn() override;
		void	Use( IEntity* activator, IEntity* caller, float value ) override;

	private:
		char	label[32];
	};
}
