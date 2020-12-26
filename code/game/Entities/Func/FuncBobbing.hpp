#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;
	class BaseMover;

	class FuncBobbing : public BaseMover
	{
	public:
		DeclareEntityClass();

		void Spawn() override;

		void Think() override;

	private:
		float bobIntensity{ 0.0f };
	};
}
