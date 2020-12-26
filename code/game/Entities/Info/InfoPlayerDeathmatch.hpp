#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class InfoPlayerDeathmatch : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
