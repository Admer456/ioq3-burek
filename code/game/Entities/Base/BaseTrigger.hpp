#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class BaseTrigger : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
