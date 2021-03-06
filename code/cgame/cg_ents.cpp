/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.hpp"


/*
======================
CG_PositionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}


/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	vec3_t			tempAxis[3];

//AxisClear( entity->axis );
	// lerp the tag
	trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( entity->axis, lerped.axis, tempAxis );
	MatrixMultiply( tempAxis, ((refEntity_t *)parent)->axis, entity->axis );
}



/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent ) {
	if ( cent->currentState.solid == SOLID_BMODEL ) {
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );
		trap_S_UpdateEntityPosition( cent->currentState.number, origin );
	} else {
		trap_S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {

	// update sound origins
	CG_SetEntitySoundPosition( cent );

	// add loop sound
	if ( cent->currentState.loopSound ) {
		if (cent->currentState.eType != ET_SPEAKER) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ cent->currentState.loopSound ] );
		} else {
			trap_S_AddRealLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ cent->currentState.loopSound ] );
		}
	}

	// constant light glow
	if(cent->currentState.constantLight)
	{
		int		cl;
		float		i, r, g, b;

		cl = cent->currentState.constantLight;
		r = (float) (cl & 0xFF) / 255.0;
		g = (float) ((cl >> 8) & 0xFF) / 255.0;
		b = (float) ((cl >> 16) & 0xFF) / 255.0;
		i = (float) ((cl >> 24) & 0xFF) * 4.0;
		trap_R_AddLightToScene(cent->lerpOrigin, i, r, g, b);
	}
}

static void CG_AxialOrientation( refEntity_t& re, const TightOrientation& orientation )
{
	if ( orientation.forward[0] == 0 && orientation.forward[1] == 0 && orientation.forward[2] == 0 )
		return;

	Vector forward, right, up;
	forward = orientation.GetForward();
	right = orientation.GetRight();
	up = orientation.GetUp();

	re.axis[0] << forward;
	re.axis[1] << right;
	re.axis[2] << up;
	//forward.CopyToArray( re.axis[0] );
	//right.CopyToArray( re.axis[1] );
	//up.CopyToArray( re.axis[2] );
}

/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent, bool attach = false ) 
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if (!s1->modelindex) 
	{
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) 
	{
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	}
	else 
	{
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// player model
	if (s1->number == cg.snap->ps.clientNum) 
	{
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	if ( s1->solid != SOLID_BMODEL && s1->framerate )
		RenderEntity::CalculateAnimation( ent, cent->currentState );

	CG_AxialOrientation( ent, cent->currentState.apos.axialOrientation );

	// Attaching entities are special
	// Attachments on attachments aren't yet supported
	if ( attach )
	{
		int parentIndex = cent->currentState.otherEntityNum2;
		centity_t* parentEnt = &cg_entities[parentIndex];

		if ( parentEnt->currentValid )
		{
			qhandle_t modelIndex = parentEnt->currentState.modelindex;
			refEntity_t parent;
			memset( &parent, 0, sizeof( parent ) );

			VectorCopy( parentEnt->lerpOrigin, parent.origin );
			AnglesToAxis( parentEnt->lerpAngles, parent.axis );
			parent.hModel = modelIndex;

			CG_PositionRotatedEntityOnTag( &ent, &parent, modelIndex, cent->currentState.attachBone );
		}
	}

	// add to refresh list
	trap_R_AddRefEntityToScene (&ent);

	// Entity effects
	if ( cent->currentState.effectFlags & EffectFlags::GlowShell )
	{
		ent.customShader = cgs.media.glowShellMaterial;

		trap_R_AddRefEntityToScene( &ent );
	}

	// add the secondary model
	if ( s1->modelindex2 ) 
	{
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		ent.customShader = 0;
		trap_R_AddRefEntityToScene( &ent );
	}
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker( centity_t *cent ) {
	if ( ! cent->currentState.clientNum ) {	// FIXME: use something other than clientNum...
		return;		// not auto triggering
	}

	if ( cg.time < cent->miscTime ) {
		return;
	}

	trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm] );

	//	ent->s.frame = ent->wait * 10;
	//	ent->s.clientNum = ent->random * 10;
	cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
}

/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) {
	refEntity_t		ent;
	entityState_t	*es;
	gitem_t			*item;
	int				msec;
	float			frac;
	float			scale;

	es = &cent->currentState;
	if ( es->modelindex >= bg_numItems ) {
		CG_Error( "Bad item index %i on entity", es->modelindex );
	}

	// if set to invisible, skip
	if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) ) {
		return;
	}

	item = &bg_itemlist[ es->modelindex ];
	if ( cg_simpleItems.integer && item->giType != IT_TEAM ) {
		memset( &ent, 0, sizeof( ent ) );
		ent.reType = RT_SPRITE;
		VectorCopy( cent->lerpOrigin, ent.origin );
		ent.radius = 14;
		ent.customShader = cg_items[es->modelindex].icon;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		trap_R_AddRefEntityToScene(&ent);
		return;
	}

	// items bob up and down continuously
	scale = 0.005 + cent->currentState.number * 0.00001;
	cent->lerpOrigin[2] += 4 + cos( ( cg.time + 1000 ) *  scale ) * 4;

	memset (&ent, 0, sizeof(ent));

	// autorotate at one of two speeds
	if ( item->giType == IT_HEALTH ) {
		VectorCopy( cg.autoAnglesFast, cent->lerpAngles );
		AxisCopy( cg.autoAxisFast, ent.axis );
	} else {
		VectorCopy( cg.autoAngles, cent->lerpAngles );
		AxisCopy( cg.autoAxis, ent.axis );
	}
	
	if( item->giType == IT_WEAPON && item->giTag == WP_RAILGUN ) {
		clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
		Byte4Copy( ci->c1RGBA, ent.shaderRGBA );
	}

	ent.hModel = cg_items[es->modelindex].models[0];

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = qfalse;

	// if just respawned, slowly scale up
	msec = cg.time - cent->miscTime;
	if ( msec >= 0 && msec < ITEM_SCALEUP_TIME ) {
		frac = (float)msec / ITEM_SCALEUP_TIME;
		VectorScale( ent.axis[0], frac, ent.axis[0] );
		VectorScale( ent.axis[1], frac, ent.axis[1] );
		VectorScale( ent.axis[2], frac, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	} else {
		frac = 1.0;
	}

	// items without glow textures need to keep a minimum light value
	// so they are always visible
	if ( ( item->giType == IT_WEAPON ) ||
		 ( item->giType == IT_ARMOR ) ) {
		ent.renderfx |= RF_MINLIGHT;
	}

	// increase the size of the weapons when they are presented as items
	if ( item->giType == IT_WEAPON ) {
		VectorScale( ent.axis[0], 1.5, ent.axis[0] );
		VectorScale( ent.axis[1], 1.5, ent.axis[1] );
		VectorScale( ent.axis[2], 1.5, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	}

	CG_AxialOrientation( ent, cent->currentState.apos.axialOrientation );

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	// accompanying rings / spheres for powerups
	if ( !cg_simpleItems.integer ) 
	{
		vec3_t spinAngles;

		VectorClear( spinAngles );

		if ( item->giType == IT_HEALTH || item->giType == IT_POWERUP )
		{
			if ( ( ent.hModel = cg_items[es->modelindex].models[1] ) != 0 )
			{
				if ( item->giType == IT_POWERUP )
				{
					ent.origin[2] += 12;
					spinAngles[1] = ( cg.time & 1023 ) * 360 / -1024.0f;
				}
				AnglesToAxis( spinAngles, ent.axis );
				
				// scale up if respawning
				if ( frac != 1.0 ) {
					VectorScale( ent.axis[0], frac, ent.axis[0] );
					VectorScale( ent.axis[1], frac, ent.axis[1] );
					VectorScale( ent.axis[2], frac, ent.axis[2] );
					ent.nonNormalizedAxes = qtrue;
				}
				trap_R_AddRefEntityToScene( &ent );
			}
		}
	}
}

//============================================================================

/*
===============
CG_Missile
===============
*/
static void CG_Missile( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	//const weaponInfo_t		*weapon;
//	int	col;

	return; // TODO: implement the weapon system from the clientside

	s1 = &cent->currentState;
	if ( s1->weapon >= WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	//weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	
	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}

	// spin as it moves
	if ( s1->pos.trType != TR_STATIONARY ) {
		RotateAroundDirection( ent.axis, cg.time / 4 );
	} else {
		{
			RotateAroundDirection( ent.axis, s1->time );
		}
	}

	// add to refresh list, possibly with quad glow
	//CG_AddRefEntityWithPowerups( &ent, s1, TEAM_FREE );
	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) 
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) 
	{
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} 
	else
	{
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	CG_AxialOrientation( ent, cent->currentState.apos.axialOrientation );

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	// add the secondary model
	if ( s1->modelindex2 ) 
	{
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		trap_R_AddRefEntityToScene(&ent);
	}
}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	ByteToDir( s1->eventParm, ent.axis[0] );
	PerpendicularVector( ent.axis[1], ent.axis[0] );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
	ent.oldframe = s1->powerups;
	ent.frame = s1->frame;		// rotation speed
	ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
================
CG_CreateRotationMatrix
================
*/
void CG_CreateRotationMatrix(vec3_t angles, vec3_t matrix[3]) {
	AngleVectors(angles, matrix[0], matrix[1], matrix[2]);
	VectorInverse(matrix[1]);
}

/*
================
CG_TransposeMatrix
================
*/
void CG_TransposeMatrix(vec3_t matrix[3], vec3_t transpose[3]) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
================
CG_RotatePoint
================
*/
void CG_RotatePoint(vec3_t point, vec3_t matrix[3]) {
	vec3_t tvec;

	VectorCopy(point, tvec);
	point[0] = DotProduct(matrix[0], tvec);
	point[1] = DotProduct(matrix[1], tvec);
	point[2] = DotProduct(matrix[2], tvec);
}

/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out, vec3_t angles_in, vec3_t angles_out) {
	centity_t	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles, deltaAngles;
	vec3_t	matrix[3], transpose[3];
	vec3_t	org, org2, move2;

	if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
		VectorCopy( in, out );
		VectorCopy(angles_in, angles_out);
		return;
	}

	cent = &cg_entities[ moverNum ];
	if ( cent->currentState.eType != ET_MOVER ) {
		VectorCopy( in, out );
		VectorCopy(angles_in, angles_out);
		return;
	}

	BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	// origin change when on a rotating object
	CG_CreateRotationMatrix( deltaAngles, transpose );
	CG_TransposeMatrix( transpose, matrix );
	VectorSubtract( in, oldOrigin, org );
	VectorCopy( org, org2 );
	CG_RotatePoint( org2, matrix );
	VectorSubtract( org2, org, move2 );
	VectorAdd( deltaOrigin, move2, deltaOrigin );

	VectorAdd( in, deltaOrigin, out );
	VectorAdd( angles_in, deltaAngles, angles_out );
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
	vec3_t		current, next;
	float		f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if ( cg.nextSnap == NULL ) {
		CG_Error( "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
	}

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

	Vector currentAxis[3];
	Vector nextAxis[3];

	currentAxis[0] = cent->currentState.apos.axialOrientation.GetForward();
	currentAxis[1] = cent->currentState.apos.axialOrientation.GetRight();
	currentAxis[2] = cent->currentState.apos.axialOrientation.GetUp();

	nextAxis[0] = cent->nextState.apos.axialOrientation.GetForward();
	nextAxis[1] = cent->nextState.apos.axialOrientation.GetRight();
	nextAxis[2] = cent->nextState.apos.axialOrientation.GetUp();

	for ( int i = 0; i < 3; i++ )
	{
		cent->lerpAxis[i] = currentAxis[i] + f * (nextAxis[i] - currentAxis[i]);
	}
}

/*
===============
CG_CalcEntityLerpPositions

===============
*/
static void CG_CalcEntityLerpPositions( centity_t *cent ) {

	// if this player does not want to see extrapolated players
	if ( !cg_smoothClients.integer ) {
		// make sure the clients use TR_INTERPOLATE
		if ( cent->currentState.number < MAX_CLIENTS ) {
			cent->currentState.pos.trType = TR_INTERPOLATE;
			cent->nextState.pos.trType = TR_INTERPOLATE;
		}
	}

	if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if ( cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP &&
											cent->currentState.number < MAX_CLIENTS) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// just use the current frame and evaluate as best we can
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if ( cent != &cg.predictedPlayerEntity ) {
		CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
		cg.snap->serverTime, cg.time, cent->lerpOrigin, cent->lerpAngles, cent->lerpAngles);
	}
}

/*
===============
CG_Sprite
===============
*/
static void CG_Sprite( centity_t* cent )
{
	refEntity_t spr;
	float alpha = cent->currentState.origin2[1];

	memset( &spr, 0, sizeof( spr ) );

	spr.reType = RT_SPRITE;
	
	VectorCopy( cent->lerpOrigin, spr.lightingOrigin );
	VectorCopy( cent->lerpOrigin, spr.origin );

	spr.customShader = cgs.gameMaterials[cent->currentState.modelindex];
	spr.radius = cent->currentState.origin2[0];
	spr.shaderRGBA[0] = 255.0f * cent->currentState.angles2[0] * alpha;
	spr.shaderRGBA[1] = 255.0f * cent->currentState.angles2[1] * alpha;
	spr.shaderRGBA[2] = 255.0f * cent->currentState.angles2[2] * alpha;
	spr.shaderRGBA[3] = 255U;

	trap_R_AddRefEntityToScene( &spr );
}

#include "../shared/Weapons/WeaponIDs.hpp"

/*
===============
CG_Character
===============
*/
static void CG_Character( centity_t* cent )
{
	using Flags = ActorClientBits;

	refEntity_t	ent;
	entityState_t* s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if ( !s1->modelindex )
	{
		return;
	}

	int headNumber{ 0 }, headState{ 0 };
	int encodedStuff = s1->time2;
	Vector lookDirection = s1->angles2;

	if ( encodedStuff & Flags::Head1 ) headNumber = 0;
	if ( encodedStuff & Flags::Head2 ) headNumber = 1;
	if ( encodedStuff & Flags::Head3 ) headNumber = 2;

	if ( encodedStuff & Flags::Dead ) headState = 1;

	memset( &ent, 0, sizeof( ent ) );

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL )
	{
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	}
	else
	{
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	if ( s1->solid != SOLID_BMODEL && s1->framerate )
		RenderEntity::CalculateAnimation( ent, cent->currentState );

	CG_AxialOrientation( ent, cent->currentState.apos.axialOrientation );

	// add to refresh list
	trap_R_AddRefEntityToScene( &ent );

	// Entity effects
	if ( cent->currentState.effectFlags & EffectFlags::GlowShell )
	{
		ent.customShader = cgs.media.glowShellMaterial;
		trap_R_AddRefEntityToScene( &ent );
	}

	// add the head mode
	{
		refEntity_t head;
		memcpy( &head, &ent, sizeof( head ) );
		AxisClear( head.axis );

		// So that the head doesn't end up getting rotated wrongly
		// Man, I should've just made a monolithic model lol
		Vector correctedRotation = Vector( 0, 0, -90 );
		
		head.hModel = cgs.media.headModels[headNumber * 2 + headState];

		AnglesToAxis( correctedRotation, head.axis );
		CG_PositionRotatedEntityOnTag( &head, &ent, ent.hModel, "Head" );
		
		trap_R_AddRefEntityToScene( &head );

		if ( encodedStuff & Flags::Hat )
		{
			head.hModel = cgs.media.accessoryModels[Accessory_Hat];
			trap_R_AddRefEntityToScene( &head );
		}

		if ( encodedStuff & Flags::Hair )
		{
			head.hModel = cgs.media.accessoryModels[Accessory_Hair];
			trap_R_AddRefEntityToScene( &head );
		}

		if ( encodedStuff & Flags::Cap )
		{
			head.hModel = cgs.media.accessoryModels[Accessory_Cap];
			trap_R_AddRefEntityToScene( &head );
		}

		if ( encodedStuff & Flags::Sunglasses )
		{
			head.hModel = cgs.media.accessoryModels[Accessory_Sunglasses];
			trap_R_AddRefEntityToScene( &head );
		}
	}

	// add the weapon model
	// fists are 0, any other weapon is 1+
	if ( encodedStuff & (Flags::ShowPistol | Flags::ShowPistolHolstered) )
	{
		refEntity_t weapon;
		memcpy( &weapon, &ent, sizeof( weapon ) );
		AxisClear( weapon.axis );

		Vector correctedRotation;

		if ( encodedStuff & Flags::ShowPistol )
			correctedRotation = Vector( 0, 90, -90 );
		else
			correctedRotation = Vector( 0, 90, 0 );

		AnglesToAxis( correctedRotation, weapon.axis );

		weapon.hModel = gWeapons[WeaponID_Pistol]->GetWeaponInfo().worldModelHandle;
		char* boneName = (encodedStuff & Flags::ShowPistol) ? "RWrist" : "Holster";
		CG_PositionRotatedEntityOnTag( &weapon, &ent, ent.hModel, boneName );

		float time = GetClient()->Time();

		Vector origin = weapon.origin;

		if ( encodedStuff & Flags::ShowPistol )
		{
			origin += Vector( weapon.axis[0] ) * 5.6f;
			origin += Vector( weapon.axis[1] ) * -2.4f;
			origin += Vector( weapon.axis[2] ) * 0.4f;
		}
		else
		{
			origin += Vector( weapon.axis[2] ) * -0.5f;
		}

		weapon.origin << origin;

		trap_R_AddRefEntityToScene( &weapon );
	}
}

/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) {
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	// add automatic effects
	CG_EntityEffects( cent );

	switch ( cent->currentState.eType ) {
	default:
		CG_Error( "Bad entity type: %i", cent->currentState.eType );
		break;
	case ET_INVISIBLE:	break;
	case ET_GENERAL:	CG_General( cent ); break;
	case ET_PLAYER:		CG_Player( cent ); break;
	case ET_ITEM:		CG_Item( cent ); break;
	case ET_MISSILE:	CG_Missile( cent ); break;
	case ET_MOVER:		CG_Mover( cent ); break;
	case ET_BEAM:		CG_Beam( cent ); break;
	case ET_PORTAL:		CG_Portal( cent ); break;
	case ET_SPEAKER:	CG_Speaker( cent ); break;
	case ET_SPRITE:		CG_Sprite( cent ); break;
	case ET_SKY:		break;
	case ET_ATTACHMENT: CG_General( cent, true ); break;
	case ET_CHARACTER:	CG_Character( cent ); break;
	}
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent = nullptr;
	playerState_t		*ps;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because 
									// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = (cg.time & 1023) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
	CG_AddCEntity( &cg.predictedPlayerEntity );

	// lerp the non-predicted value for lightning gun origins
	CG_CalcEntityLerpPositions( &cg_entities[cg.snap->ps.clientNum] );

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[cg.snap->entities[num].number];
		CG_AddCEntity( cent );
	}
}

