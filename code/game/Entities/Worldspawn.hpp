#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class Worldspawn final : public BaseEntity
	{
	public:
		void Spawn() override;
	};
}
