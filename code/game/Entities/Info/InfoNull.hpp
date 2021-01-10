#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class InfoNull : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
