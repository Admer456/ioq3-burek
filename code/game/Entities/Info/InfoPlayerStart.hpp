#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class InfoPlayerStart : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
