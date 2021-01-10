#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class InfoNotNull : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
