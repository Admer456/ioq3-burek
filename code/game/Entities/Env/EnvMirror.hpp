#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class EnvMirror final : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void Spawn() override;
	};
}
