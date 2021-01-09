#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;
	class BaseMover;

	class FuncRotating : public BaseMover
	{
	public:
		DeclareEntityClass();

		void Spawn() override;

	private:
		Vector rotationDir{ Vector::Zero };
	};
}
