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

		bool		fireHeld{ false };
	};
}
