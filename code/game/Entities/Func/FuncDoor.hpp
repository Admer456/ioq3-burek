#pragma once

enum DoorStates
{
	Door_Closed = 0,
	Door_Opening,
	Door_Opened,
	Door_Closing
};

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;
	class BaseMover;

	/*
	* func_door
	* A generic sliding door.
	* 
	* Keyvalues:
	* expand -> how much the bbox will expand so the door opens before the player runs into it
	* distance -> how far the door is gonna travel
	* axis -> which direction the door will go to
	*/
	class FuncDoor : public BaseMover
	{
	public:
		DeclareEntityClass();

		constexpr static int SF_StartOpen = 1 << 0;
		constexpr static int SF_NoTouch = 1 << 1;
		constexpr static int SF_NoUse = 1 << 2;

		void		Spawn() override;

		void		Blocked( IEntity* other ) override;

		void		DoorThink();

		void		DoorUse( IEntity* activator, IEntity* caller, float value );
		void		DoorTouch( IEntity* other, trace_t* trace );

		virtual void OnClose();
		virtual void OnCloseFinished();

		virtual void OnOpen();
		virtual void OnOpenFinished();

		virtual void UpdateDoorState();

		virtual void Enable();
		virtual void Disable();
	
	protected:
		byte		doorState{ Door_Closed };
		float		expand{ 0.0f };
		float		distance{ 0.0f };
		Vector		axis{ Vector::Zero };

		Vector		closedPos{ Vector::Zero };
		Vector		openedPos{ Vector::Zero };
	};
}
