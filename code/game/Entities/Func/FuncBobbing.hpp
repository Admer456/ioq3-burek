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
		float bobPhase{ 0.0f };
		float bobDuration{ 0.0f };
		float bobIntensity{ 0.0f };
		Vector bobAxis{ Vector::Zero };
	};
}
