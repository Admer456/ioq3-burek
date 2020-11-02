#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class Worldspawn final : public BaseQuakeEntity
	{
	public:
		void Spawn() override;
	};
}
