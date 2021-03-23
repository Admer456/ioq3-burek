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

	GetState()->eType = ET_ATTACHMENT;

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
	if ( simple && nextThink < level.time * 0.001f )
		return;

	UpdateAttachmentTransform();

	BaseEntity::Think();

	nextThink = level.time*0.001f + 2.0f;
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

	if ( nullptr == parent )
	{
		return;
	}

	if ( simple )
	{
		SetCurrentOrigin( parent->GetCurrentOrigin().Snapped() );

		return;
	}

	int startFrame, endFrame;
	float fraction;

	BaseAttachment::CalculateFrame( parent, startFrame, endFrame, fraction );

	gameImports->LerpTag( &tag, parent->GetState()->modelindex, startFrame, endFrame, fraction, shared.s.attachBone );

	/*
	tag.axis[0] -> bone's local down
	tag.axis[1] -> bone's local forward
	tag.axis[2] -> bone's local right
	*/

	TightOrientation& to = GetState()->apos.axialOrientation;
	Vector forward = tag.axis[1];
	Vector up = Vector( tag.axis[2] ) * -1.0f;

	to.SetForward( forward );
	to.SetUp( up );

	SetAngles( Vector::Zero );
	SetCurrentAngles( Vector::Zero );
}
