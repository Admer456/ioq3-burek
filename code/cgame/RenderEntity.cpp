#include "cg_local.hpp"
#include "RenderEntity.hpp"
#include <string>

RenderEntity::RenderEntity( const char* modelName )
{
	memset( &ref, 0, sizeof( ref ) );

	ref.hModel = trap_R_RegisterModel( modelName );

	std::string modelPath = modelName;
	modelPath = modelPath.substr( 0, modelPath.size() - 4 ); // strip the extension
	modelPath += ".mcfg"; // MCFG = Model ConFiGuration file

	LoadModelConfig( modelPath.c_str() );
}

void RenderEntity::CalculateAnimation( refEntity_t& ref, entityState_t& es )
{
	float framerate = es.framerate;
	float animTime = es.animationTime * 0.001f;
	int frame = es.frame;
	int flags = es.animationFlags;
	animHandle anim = es.animation;
	qhandle_t model = es.modelindex;

	if ( GetClient()->GetAnimationsForModel( model ).empty() )
	{
		return;
	}

	Assets::ModelAnimation ma = GetClient()->GetAnimationsForModel( model ).at( anim );

	if ( flags & AnimFlag_Manual )
	{
		ref.frame = frame;
		ref.oldframe = 0;
		ref.backlerp = 0.0f;
		return;
	}

	float calcFrame = (cg.time * 0.001f - animTime) * framerate;

	if ( flags & AnimFlag_Loop )
	{
		if ( calcFrame >= ma.numFrames )
		{
			calcFrame -= (((int)calcFrame) / ma.numFrames) * ma.numFrames;
		}
	}
	else if ( calcFrame > ma.numFrames )
	{
		calcFrame = ma.numFrames;
	}

	ref.oldframe = calcFrame + ma.firstFrame;
	ref.frame = ref.oldframe + 1;
	ref.backlerp = 1.0f - (calcFrame - (int)calcFrame);
}

void RenderEntity::Update( float deltaTime )
{
	if ( currentAnim != AnimHandleNotFound )
	{
		// manual = frames are set manually elsewhere
		if ( !isManual )
		{
			float framerate = 1000.0f / MAX( 1, anims[currentAnim].frameLerp );
			currentFrame += deltaTime * framerate;
		}

		ref.oldframe = currentFrame + anims[currentAnim].firstFrame;
		ref.frame = ref.oldframe + 1;
		ref.backlerp = 1.0f - (currentFrame - (int)currentFrame);
	}

	origin.CopyToArray( ref.origin );
	AnglesToAxis( angles, ref.axis );

	if ( currentFrame > anims[currentAnim].numFrames )
		currentFrame -= anims[currentAnim].numFrames;
}

void RenderEntity::StartAnimation( animHandle anim, bool fromStart )
{
	currentAnim = anim;

	if ( fromStart )
		Reset();
}

void RenderEntity::Reset()
{
	currentFrame = 0.0f;
	Update( 0.0f );
}

void RenderEntity::SetAnimationMode( bool manual )
{
	isManual = manual;
}

void RenderEntity::SetCurrentFrame( float frame )
{
	currentFrame = frame;
	Update( 0.0f );
}

animHandle RenderEntity::GetCurrentAnimation()
{
	return currentAnim;
}

animHandle RenderEntity::GetAnimByName( const char* animName )
{
	for ( Assets::ModelAnimation& anim : anims )
	{
		if ( !anim.name[0] )
			continue;

		if ( !strcmp( animName, anim.name ) )
			return &anim - anims.data();
	}

	return AnimHandleNotFound;
}

Assets::ModelAnimation RenderEntity::GetAnimData( animHandle handle )
{
	return anims[handle];
}

refEntity_t& RenderEntity::GetRefEntity()
{
	return ref;
}

void RenderEntity::LoadModelConfig( const char* modelConfigPath )
{
	anims = Assets::ModelConfigData::GetAnimations( modelConfigPath );
}
