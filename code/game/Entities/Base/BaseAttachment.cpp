#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "BaseAttachment.hpp"

using namespace Entities;

DefineAbstractEntityClass( BaseAttachment, BaseEntity );

void BaseAttachment::CalculateFrame( IEntity* ent, int& startFrame, int& endFrame, float& fraction )
{
	BaseEntity* bent = static_cast<BaseEntity*>(ent);
	entityState_t es = *ent->GetState();

	float framerate = es.framerate;
	float animTime = es.animationTime * 0.001f;
	int frame = es.frame;
	int flags = es.animationFlags;
	animHandle anim = es.animation;
	qhandle_t model = es.modelindex;

	if ( bent->GetState()->animation >= bent->anims.size() )
		return;

	const auto& ma = bent->anims.at( bent->GetState()->animation );

	if ( flags & AnimFlag_Manual )
	{
		startFrame = frame;
		endFrame = 0;
		fraction = 0.0f;
		return;
	}

	float calcFrame = (level.time * 0.001f - animTime) * framerate;

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

	endFrame = calcFrame + ma.firstFrame;
	startFrame = endFrame + 1;
	fraction = 1.0f - (calcFrame - (int)calcFrame);
}

void BaseAttachment::Spawn()
{
	BaseEntity::Spawn();

	GetState()->pos.trType = TR_INTERPOLATE;
	GetState()->apos.trType = TR_INTERPOLATE;
}

void BaseAttachment::PostSpawn()
{
	BaseEntity::PostSpawn();

	IEntity* parent = GetTargetEntity();
	if ( nullptr == parent )
	{
		Remove();
	}

	const char* boneTarget = spawnArgs->GetCString( "bone", "default" );
	AttachTo( parent, boneTarget );
}

void BaseAttachment::Think()
{
	UpdateAttachmentTransform();

	BaseEntity::Think();
}

void BaseAttachment::AttachTo( IEntity* parent, const char* targetBone )
{
	strncpy( shared.s.attachBone, targetBone, 16 );
	shared.s.otherEntityNum2 = parent->GetEntityIndex();

	SwitchToBone( targetBone );
}

void BaseAttachment::SwitchToBone( const char* targetBone )
{
	if ( nullptr == targetBone )
		return;

	strncpy( shared.s.attachBone, targetBone, 16 );
}

void BaseAttachment::Detach()
{
	SwitchToBone( "" );
	shared.s.otherEntityNum2 = ENTITYNUM_NONE;
}

const char* BaseAttachment::GetBoneName() const
{
	return shared.s.attachBone;
}

void BaseAttachment::UpdateAttachmentTransform()
{
	orientation_t tag;
	IEntity* parent = gEntities[shared.s.otherEntityNum2];

	int startFrame, endFrame;
	float fraction;

	BaseAttachment::CalculateFrame( parent, startFrame, endFrame, fraction );

	gameImports->LerpTag( &tag, parent->GetState()->modelindex, startFrame, endFrame, fraction, shared.s.attachBone );

	Vector axis( tag.axis[0] );
	axis = axis.ToAngles();

	SetAngles( parent->GetAngles() + axis );
	SetCurrentAngles( parent->GetCurrentAngles() + axis );
	(parent->GetCurrentAngles() + axis).CopyToArray( GetState()->apos.trBase );

	SetOrigin( parent->GetOrigin() + tag.origin );
	SetCurrentOrigin( parent->GetCurrentOrigin() + tag.origin );
	(parent->GetCurrentOrigin() + tag.origin).CopyToArray( GetState()->pos.trBase );
}
