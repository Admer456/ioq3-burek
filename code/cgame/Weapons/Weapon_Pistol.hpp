#pragma once

namespace ClientEntities
{
	class Weapon_Pistol : public BaseClientWeapon
	{
	public:
		Weapon_Pistol() = default;

		WeaponInfo	GetWeaponInfo() override;

		void		Precache() override;

		void		OnIdle() override;
		void		OnPrimaryFire() override;
		//void		OnSecondaryFire() override;
		void		OnReload() override;

	private:
		Vector		CalculateLaserDotDirection();

		void		AddLaserDot( const Vector& startPosition, const Vector& direction, const int& bounce );

		animHandle	animIdle{};
		animHandle	animIdleEmpty{};
		animHandle	animAttack[3]{};
		animHandle	animAttackEmpty{};
		animHandle	animAttackToEmpty{};
		animHandle	animReload{};
		animHandle	animReloadTactical{};

		sfxHandle_t soundShoot{};
		sfxHandle_t soundShootEmpty{};

		qhandle_t	muzzleFlashSprite{};
		qhandle_t	laserDotSprite{};

		refEntity_t laser;
	
		bool		fireHeld{ false };
	};
}
