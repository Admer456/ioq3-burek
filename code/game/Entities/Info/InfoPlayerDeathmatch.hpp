#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class InfoPlayerDeathmatch : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
