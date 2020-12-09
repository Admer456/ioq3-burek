#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class InfoPlayerStart : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
