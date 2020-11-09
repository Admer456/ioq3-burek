#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;
	class BasePlayer : public BaseQuakeEntity
	{
	public:
		void			Spawn() override;

	public: // "Properties"
		gclient_t*		GetClient();
		void			SetClient( const gclient_t* playerClient );

		const Vector&	GetClientViewAngle() const;
		void			SetClientViewAngle( const Vector& newAngle );

	public: // Misc

		// Aka spawn a dead body
		void			CopyToBodyQue();

		void			AddEvent( int event, int eventParameter );

		// Forcibly fire a weapon
		void			FireWeapon();

		void			SetTeam( const char* teamName );

		void			StopFollowing();
		void			FollowCycle( int dir );

	protected:
		gclient_s*		client{ nullptr };
	};
}
