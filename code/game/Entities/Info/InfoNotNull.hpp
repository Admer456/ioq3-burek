#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class InfoNotNull : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
