#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class InfoNull : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
