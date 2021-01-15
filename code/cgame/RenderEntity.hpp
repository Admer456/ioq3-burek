#pragma once

class RenderEntity
{
public:
	RenderEntity() = default;
	RenderEntity( const char* modelName );
	RenderEntity( qhandle_t modelHandle );

	static void CalculateAnimation( refEntity_t& ref, entityState_t& es );

	// Updates the render entity, incrementing frame counters etc.
	void		Update( float deltaTime );

	// Changes the current animation
	void		StartAnimation( animHandle anim, bool fromStart = true );
	
	// Resets the animation to start from frame 0
	void		Reset();

	// Sets the animation mode. If true, then
	// currentFrame won't get updated per frame, but
	// rather manually via SetCurrentFrame()
	void		SetAnimationMode( bool manual );

	void		SetCurrentFrame( float frame );

	// Retrieves the currently playing animation
	animHandle	GetCurrentAnimation();

	// Retrieves an animHandle by the given name
	// If it fails to find one, it returns 0xff
	animHandle	GetAnimByName( const char* animName );

	Assets::ModelAnimation GetAnimData( animHandle handle );

	refEntity_t& GetRefEntity();

	Vector		origin{ Vector::Zero };
	Vector		angles{ Vector::Zero };

protected:
	// Initialises the animation data etc.
	void		LoadModelConfig( const char* modelConfigPath );

	// The raw render entity that gets passed to the renderer
	refEntity_t ref;

	// If true, then currentFrame won't get updated per 
	// frame, but rather manually via SetCurrentFrame()
	bool		isManual{ false };

	// Usually it'd be an integer, however, a float allows me to 
	// very easily calculate ref.frame, ref.oldframe AND ref.backlerp
	float		currentFrame{ 0 };

	animHandle	currentAnim{ 0 };

	std::vector<Assets::ModelAnimation> anims;
};
