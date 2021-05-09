#include "cg_local.hpp"
#include "View/View.hpp"
#include "../shared/Weapons/WeaponIDs.hpp"
#include "View/LightManager.hpp"
#include "View/ParticleManager.hpp"

#include "Weapon_Pistol.hpp"

using namespace ClientEntities;

DefineWeaponClass( Weapon_Pistol, WeaponID_Pistol );

BaseClientWeapon::WeaponInfo Weapon_Pistol::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.name = "Pistol";
	wi.viewModel = "models/weapons/v_pistol.iqm";
	wi.worldModel = "models/weapons/w_pistol.iqm";

	return wi;
}

void Weapon_Pistol::Precache()
{
	BaseClientWeapon::Precache();

	animIdle = renderEntity.GetAnimByName( "idle" );
	animIdleEmpty = renderEntity.GetAnimByName( "idle_empty" );
	animAttack[0] = renderEntity.GetAnimByName( "shot1" );
	animAttack[1] = renderEntity.GetAnimByName( "shot2" );
	animAttack[2] = renderEntity.GetAnimByName( "shot3" );
	animAttackEmpty = renderEntity.GetAnimByName( "shot_empty" );
	animAttackToEmpty = renderEntity.GetAnimByName( "shot_toempty" );
	animReload = renderEntity.GetAnimByName( "reload" );
	animReloadTactical = renderEntity.GetAnimByName( "reload_tactical" );

	soundShoot[0] = trap_S_RegisterSound( "sound/weapons/pistol_shot1.wav", false );
	soundShoot[1] = trap_S_RegisterSound( "sound/weapons/pistol_shot2.wav", false );
	soundShoot[2] = trap_S_RegisterSound( "sound/weapons/pistol_shot3.wav", false );
	soundShootEmpty = trap_S_RegisterSound( "sound/weapons/empty.wav", false );

	soundLaserOn = trap_S_RegisterSound( "sound/weapons/laseron.wav", false );
	soundLaserOff = trap_S_RegisterSound( "sound/weapons/laseroff.wav", false );

	muzzleFlashSprite = trap_R_RegisterShaderNoMip( "sprites/muzzleflash1" );
	laserDotSprite = trap_R_RegisterShaderNoMip( "sprites/laserdot" );

	renderEntity.StartAnimation( animIdle, true );
	nextIdle = GetNextAnimTime( animIdle );

	memset( &laser, 0, sizeof( laser ) );

	laser.reType = RT_SPRITE;
	laser.customShader = laserDotSprite;
	laser.hModel = laserDotSprite;
	laser.radius = 3.0f;
}

void Weapon_Pistol::OnIdle()
{
	static bool laserOn = false;
	bool laserCurrentlyOn = (cg.predictedPlayerState.weaponFlags & PlayerWeaponFlags::PistolLaser) != 0;

	usercmd_t uc = GetClient()->GetUsercmd();
	animHandle anim = animIdle;

	if ( !(uc.interactionButtons & Interaction_PrimaryAttack) )
	{
		fireHeld = false;
	}

	if ( GetClient()->Time() >= nextIdle )
	{
		int ammoMag = GetAmmoInMag( WeaponID_Pistol );

		if ( !ammoMag )
			anim = animIdleEmpty;

		renderEntity.StartAnimation( anim, true );
		nextIdle = GetNextAnimTime( anim );
	}

	Vector viewOrigin = GetClient()->GetView()->GetViewOrigin();
	Vector laserDirection = CalculateLaserDotDirection();

	// Laser turn on
	if ( laserCurrentlyOn && !laserOn )
	{
		laserOn = true;
		trap_S_StartLocalSound( soundLaserOn, CHAN_AUTO );
	}
	// Laser turn off
	else if ( !laserCurrentlyOn && laserOn )
	{
		laserOn = false;
		trap_S_StartLocalSound( soundLaserOff, CHAN_AUTO );
	}

	// Affect the render entity on the viewmodel and add a laser dot
	if ( laserCurrentlyOn && laserOn )
	{
		AddLaserDot( viewOrigin, laserDirection, 0 );
		renderEntity.GetRefEntity().shaderRenderMask = 0;
	}
	else
	{
		renderEntity.GetRefEntity().shaderRenderMask = 1;
	}
}

void Weapon_Pistol::OnPrimaryFire()
{
	if ( GetClient()->Time() < nextPrimary )
		return;

	if ( fireHeld )
		return;

	fireHeld = true;

	int ammoMag = GetAmmoInMag( WeaponID_Pistol );
	animHandle anim = animAttack[0];
	sfxHandle_t sound = soundShoot[rand() % 3];
	LightManager* lightManager = GetClient()->GetLightManager();
	ParticleManager* particleManager = GetClient()->GetParticleManager();

	if ( ammoMag > 1 )
	{
		anim = animAttack[ammoMag % 3];
		
	}
	else if ( ammoMag == 1 )
	{
		anim = animAttackToEmpty;
	}
	else
	{
		anim = animAttackEmpty;
		sound = soundShootEmpty;
		nextPrimary = GetClient()->Time() + 0.1f;
	}

	trap_S_StartLocalSound( sound, CHAN_WEAPON );

	ClientView* view = GetClient()->GetView();
	Vector forward, right, up;
	Vector::AngleVectors( view->GetViewAngles(), &forward, &right, &up );

	if ( anim != animAttackEmpty )
	{
		view->AddPunch( 0.12f, Vector( -0.9f,  0.0f, 0.0f ) );
		view->AddPunch( 0.27f, Vector(  0.2f,  0.4f,-0.1f ) );
		view->AddPunch( 0.35f, Vector(  0.0f, -0.1f, 0.2f ) );

		view->AddShake( 8.0f, 0.125f * 4.0f, forward * -0.5f );

		Vector lightOrigin = view->GetViewOrigin() + forward * 1.0f;
		Vector muzzleOrigin = view->GetViewOrigin() + forward * 20.0f + right * 8.0f - up * 4.5f;

		Light shootLight = Light( lightOrigin, Vector( 1.0f, 0.85f, 0.63f ) * 0.02f, 300.0f, 0.08f );
		lightManager->AddLight( shootLight );

		particleManager->AddParticle<Particles::BaseParticle>( muzzleFlashSprite, muzzleOrigin, 3.0f, 0.02f );
	}

	renderEntity.StartAnimation( anim, true );
	nextIdle = nextReload = GetNextAnimTime( anim );
}

void Weapon_Pistol::OnReload()
{
	if ( GetClient()->Time() < nextReload )
		return;

	int ammoMag = GetAmmoInMag( WeaponID_Pistol );
	animHandle anim = (ammoMag >= 1) ? animReloadTactical : animReload;

	renderEntity.StartAnimation( anim, true );
	nextReload = nextIdle = nextPrimary = GetClient()->Time() + 2.66f;

	GetClient()->GetView()->AddPunch( 2.5f, Vector( -1.0f, 1.0f, 0.0f ) );
	GetClient()->GetView()->AddShake( 0.5f, 1.0f, Vector( 0.0f, 0.0f, -4.0f ) );
}

Vector Weapon_Pistol::CalculateLaserDotDirection()
{
	ClientView* cv = GetClient()->GetView();
	Vector forward, right, up;
	Vector::AngleVectors( cv->GetViewAngles(), &forward, &right, &up );

	const float speed = Vector( cg.predictedPlayerState.velocity ).Length();

	animHandle currentAnimation = renderEntity.GetCurrentAnimation();

	// Idle rocking of the laser dot
	static Vector smoothRandom = Vector::Zero;
	Vector randomDirection = Vector( crandom(), crandom(), crandom() ) * 0.007f;
	smoothRandom = smoothRandom * 0.8f + randomDirection * 0.2f;

	forward += smoothRandom;

	// If not idling, then fiddle with the laser's origin by animation
	if ( currentAnimation != animIdle && currentAnimation != animIdleEmpty )
	{
		orientation_t orient;
		int currentFrame = renderEntity.GetRefEntity().oldframe;
		int nextFrame = renderEntity.GetRefEntity().frame;
		int frameLerp = renderEntity.GetRefEntity().backlerp;

		trap_R_LerpTag( &orient, renderEntity.GetRefEntity().hModel, currentFrame, nextFrame, frameLerp, "gunSlide" );

		Vector boneForward = orient.axis[1];

		forward += boneForward.z * up;
		forward += -boneForward.y * right;

		forward.Normalize();
	}

	static Vector blendDeltaViewWeapon = Vector::Zero;
	Vector deltaViewWeapon = cv->GetWeaponOriginOffset();
	deltaViewWeapon.z *= -1.0f;

	blendDeltaViewWeapon = blendDeltaViewWeapon * 0.7f + deltaViewWeapon * 0.3f;

	forward += blendDeltaViewWeapon * 0.04f;

	return forward;
}

void Weapon_Pistol::AddLaserDot( const Vector& startPosition, const Vector& direction, const int& bounce )
{
	if ( bounce > 5 )
	{
		return;
	}

	trace_t tr;
	CG_Trace( &tr, startPosition, Vector::Zero, Vector::Zero, startPosition + direction * 2048.0f, cg.clientNum, MASK_SHOT | MASK_OPAQUE );

	Vector laserPosition = Vector( tr.endpos ) - direction * laser.radius * 1.5f;
	laser.origin << laserPosition;

	// Further bounces = smaller intensity
	if ( bounce )
	{
		laser.radius = 3.0f / (1.0f + bounce * 0.6f);
	}
	else
	{
		laser.radius = 3.0f;
	}

	// DO NOT add it on a sky surface
	if ( ~tr.surfaceFlags & SURF_SKY )
	{
		trap_R_AddRefEntityToScene( &laser );
		trap_R_AddAdditiveLightToScene( laserPosition, 6.0f * laser.radius, 1.0f, 0.0f, 0.0f );

		// Reflect on glass & mirrors
		if ( tr.surfaceFlags & SURF_GLASS )
		{
			Vector bouncedDirection = direction.Reflect( tr.plane.normal );
			AddLaserDot( laserPosition, bouncedDirection, bounce + 1 );
		}
	}
}
