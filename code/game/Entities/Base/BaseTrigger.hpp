#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class BaseTrigger : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
