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
// g_utils.c -- misc utility functions for game module

#include "Game/g_local.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

using namespace Entities;

typedef struct {
  char oldShader[MAX_QPATH];
  char newShader[MAX_QPATH];
  float timeOffset;
} shaderRemap_t;

#define MAX_SHADER_REMAPS 128

int remapCount = 0;
shaderRemap_t remappedShaders[MAX_SHADER_REMAPS];

void AddRemap(const char *oldShader, const char *newShader, float timeOffset) {
	int i;

	for (i = 0; i < remapCount; i++) {
		if (Q_stricmp(oldShader, remappedShaders[i].oldShader) == 0) {
			// found it, just update this one
			strcpy(remappedShaders[i].newShader,newShader);
			remappedShaders[i].timeOffset = timeOffset;
			return;
		}
	}
	if (remapCount < MAX_SHADER_REMAPS) {
		strcpy(remappedShaders[remapCount].newShader,newShader);
		strcpy(remappedShaders[remapCount].oldShader,oldShader);
		remappedShaders[remapCount].timeOffset = timeOffset;
		remapCount++;
	}
}

const char *BuildShaderStateConfig(void) {
	static char	buff[MAX_STRING_CHARS*4];
	char out[(MAX_QPATH * 2) + 5];
	int i;
  
	memset(buff, 0, MAX_STRING_CHARS);
	for (i = 0; i < remapCount; i++) {
		Com_sprintf(out, (MAX_QPATH * 2) + 5, "%s=%s:%5.2f@", remappedShaders[i].oldShader, remappedShaders[i].newShader, remappedShaders[i].timeOffset);
		Q_strcat( buff, sizeof( buff ), out);
	}
	return buff;
}

/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
================
G_FindConfigstringIndex

================
*/
int G_FindConfigstringIndex( char *name, int start, int max, qboolean create ) {
	int		i;
	char	s[MAX_STRING_CHARS];

	if ( !name || !name[0] ) {
		return 0;
	}

	for ( i=1 ; i<max ; i++ ) {
		gameImports->GetConfigString( start + i, s, sizeof( s ) );
		if ( !s[0] ) {
			break;
		}
		if ( !strcmp( s, name ) ) {
			return i;
		}
	}

	if ( !create ) {
		return 0;
	}

	if ( i == max ) {
		G_Error( "G_FindConfigstringIndex: overflow" );
	}

	gameImports->SetConfigString( start + i, name );

	return i;
}


int G_ModelIndex( char* name ) 
{
	return G_FindConfigstringIndex( name, CS_MODELS, MAX_MODELS, qtrue );
}

int G_SoundIndex( char* name ) 
{
	return G_FindConfigstringIndex( name, CS_SOUNDS, MAX_SOUNDS, qtrue );
}

/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float	*tv( float x, float y, float z ) {
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos( const vec3_t v ) {
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}


/*
===============
G_SetMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
===============
*/
void G_SetMovedir( vec3_t angles, vec3_t movedir ) {
	static vec3_t VEC_UP		= {0, -1, 0};
	static vec3_t MOVEDIR_UP	= {0, 0, 1};
	static vec3_t VEC_DOWN		= {0, -2, 0};
	static vec3_t MOVEDIR_DOWN	= {0, 0, -1};

	if ( VectorCompare (angles, VEC_UP) ) {
		VectorCopy (MOVEDIR_UP, movedir);
	} else if ( VectorCompare (angles, VEC_DOWN) ) {
		VectorCopy (MOVEDIR_DOWN, movedir);
	} else {
		AngleVectors (angles, movedir, NULL, NULL);
	}
	VectorClear( angles );
}


float vectoyaw( const vec3_t vec ) {
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0) {
		yaw = 0;
	} else {
		if (vec[PITCH]) {
			yaw = ( atan2( vec[YAW], vec[PITCH]) * 180 / M_PI );
		} else if (vec[YAW] > 0) {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if (yaw < 0) {
			yaw += 360;
		}
	}

	return yaw;
}

/*
================
G_CreateRotationMatrix
================
*/
void G_CreateRotationMatrix( vec3_t angles, vec3_t matrix[3] )
{
	AngleVectors( angles, matrix[0], matrix[1], matrix[2] );
	VectorInverse( matrix[1] );
}

/*
================
G_TransposeMatrix
================
*/
void G_TransposeMatrix( vec3_t matrix[3], vec3_t transpose[3] )
{
	int i, j;
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
================
G_RotatePoint
================
*/
void G_RotatePoint( vec3_t point, vec3_t matrix[3] )
{
	vec3_t tvec;

	VectorCopy( point, tvec );
	point[0] = DotProduct( matrix[0], tvec );
	point[1] = DotProduct( matrix[1], tvec );
	point[2] = DotProduct( matrix[2], tvec );
}

/*
================
DebugLine

  debug polygons only work when running a local game
  with r_debugSurface set to 2
================
*/
int DebugLine(vec3_t start, vec3_t end, int color) {
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy(start, points[0]);
	VectorCopy(start, points[1]);
	//points[1][2] -= 2;
	VectorCopy(end, points[2]);
	//points[2][2] -= 2;
	VectorCopy(end, points[3]);


	VectorSubtract(end, start, dir);
	VectorNormalize(dir);
	dot = DotProduct(dir, up);
	if (dot > 0.99 || dot < -0.99) VectorSet(cross, 1, 0, 0);
	else CrossProduct(dir, up, cross);

	VectorNormalize(cross);

	VectorMA(points[0], 2, cross, points[0]);
	VectorMA(points[1], -2, cross, points[1]);
	VectorMA(points[2], -2, cross, points[2]);
	VectorMA(points[3], 2, cross, points[3]);

	return 0;
	//return trap_DebugPolygonCreate(color, 4, points);
}
