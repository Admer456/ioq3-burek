#pragma once

namespace Entities
{
	class Mercenary : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void			Spawn() override;
		void			Think() override;

		BaseEntity*		TestEntityPosition() override;

	protected:

		// Calculates a multiplier based on the input speed
		// speed can go over 127, in which case this function will return 1.0
		// smooth is not yet implemented
		float			SnapMovementSpeed( float speed, bool smooth = false );
		// Calculates forwardmove, rightmove and upmove
		usercmd_t		CalculateCmdForVelocity( Vector wishVelocity, bool canJump = false );
		// Calculates where and how to move
		void			MovementLogic();
		// Sets up some cmd and playerState vars
		void			CalculateMoveParameters();
		// Performs the movement
		void			Move();

	protected:
		Vector			viewAngles;
		usercmd_t		cmd;
		playerState_t	playerState; // for Pmove

		Vector			lookTarget; // what am I looking at?
		Vector			lastSeen; // where did I see my enemy last?

		Vector			moveTarget; // where to walk to?
		//MovementTaskState	moveTask; // what am I supposed to do in terms of movement?

		BaseEntity*		targetEntity; // my target which I might follow, kill, or assist etc.
	};
}