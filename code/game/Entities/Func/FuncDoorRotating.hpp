#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;
	class BaseMover;

	class FuncDoorRotating : public BaseMover
	{
	public:
		DeclareEntityClass();

		constexpr static uint32_t SF_StartOpen = 1 << 0;
		constexpr static uint32_t SF_TriggerOnly = 1 << 1;
		constexpr static uint32_t SF_Reverse = 1 << 2;

		void		Spawn() override;
		
		void		Blocked( IEntity* other ) override;

		void		DoorThink();
		void		DoorUse( IEntity* activator, IEntity* caller, float value );

		void		UpdateDoorState();

		void		OnClose();
		void		OnCloseFinished();

		void		OnOpen();
		void		OnOpenFinished();

		void		Enable();
		void		Disable();

	private:
		Vector		anglesStart{ Vector::Zero };
		Vector		anglesEnd{ Vector::Zero };

		float		rotationSpeed{};
		float		rotationAngle{};
		Vector		rotationDir{ Vector::Zero };

		byte		doorState{ Door_Closed };
	};
}
