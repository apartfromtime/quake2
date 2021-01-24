/*
* Copyright (C) 1997-2001 Id Software, Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or (at
* your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
* 02111-1307, USA.
*/

/*
===============================================================================

SV_GAME - interface between the server and game module.

===============================================================================
*/

#include "server.h"

#define MAX_PRINT_CHARS 			1024
game_t * game;


/*
===============
SV_Unicast

Sends the contents of the mutlicast buffer to a single client
===============
*/
void SV_Unicast(edict_t * ent, qboolean reliable)
{
	int 		p;
	client_t * 	client;

	if ( !ent ) {
		return;
	}

	p = NUM_FOR_EDICT( ent );
	
	if ( p < 1 || p > maxclients->value ) {
		return;
	}

	client = svs.clients + ( p - 1 );

	if ( reliable ) {
		SZ_Write( &client->netchan.message, sv.multicast.data,
			sv.multicast.cursize );
	} else {
		SZ_Write( &client->datagram, sv.multicast.data,
			sv.multicast.cursize );
	}

	SZ_Clear( &sv.multicast );
}


/*
===============
SV_DebugPrintf

Debug print to server console
===============
*/
void SV_DebugPrintf(const char * fmt, ...)
{
	char 		msg[MAX_PRINT_CHARS];
	va_list 	argptr;
	
	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );

	Com_Printf( "%s", msg );
}


/*
===============
SV_GameClientPrintf

Print to a single client
===============
*/
void SV_GameClientPrintf(edict_t * ent, int level, const char * fmt, ...)
{
	char		msg[MAX_PRINT_CHARS];
	va_list		argptr;
	int n = 0;

	if ( ent ) {

		n = NUM_FOR_EDICT( ent );

		if ( n < 1 || n > maxclients->value ) {
			Com_Error( ERR_DROP, "cprintf to a non-client" );
		}
	}

	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );

	if ( ent ) {
		SV_ClientPrintf( svs.clients + ( n - 1 ), level, "%s", msg );
	} else {
		Com_Printf( "%s", msg );
	}
}


/*
===============
SV_ClientCenterPrintf

centerprint to a single client
===============
*/
void SV_GameClientCenterPrintf(edict_t * ent, const char * fmt, ...)
{
	char		msg[MAX_PRINT_CHARS];
	va_list		argptr;
	int			n;

	n = NUM_FOR_EDICT( ent );

	if ( n < 1 || n > maxclients->value ) {
		return;
	}

	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );

	MSG_WriteByte( &sv.multicast, svc_centerprint );
	MSG_WriteString( &sv.multicast, msg );
	SV_Unicast( ent, true );
}


/*
===============
SV_Error

Abort the server with a game error
===============
*/
void SV_Error(const char * fmt, ...)
{
	char		msg[MAX_PRINT_CHARS];
	va_list		argptr;

	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );

	Com_Error( ERR_DROP, "Game Error: %s", msg );
}


/*
=================
SV_SetModel

Also sets mins and maxs for inline bmodels
=================
*/
void SV_SetModel(edict_t * ent, const char * name)
{
	int 		i;
	cmodel_t * 	mod;

	if ( !name ) {
		Com_Error( ERR_DROP, "SV_SetModel: NULL" );
	}

	i = SV_ModelIndex( name );

	// ent->model = name;
	ent->s.modelindex = i;

	// if it is an inline model, get the size information for it
	if ( name[0] == '*' ) {

		mod = CM_InlineModel( name );
		VectorCopy( mod->mins, ent->mins );
		VectorCopy( mod->maxs, ent->maxs );
		SV_LinkEdict( ent );
	}

}

/*
===============
SV_ConfigString
===============
*/
void SV_ConfigString(int index, const char * val)
{
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "configstring: bad index %i\n", index );
	}

	if ( !val ) {
		val = "";
	}

	// change the string in sv
	strcpy( sv.configstrings[index], val );

	
	if (sv.state != ss_loading) {

		// send the update to everyone
		SZ_Clear( &sv.multicast );
		MSG_WriteChar( &sv.multicast, svc_configstring );
		MSG_WriteShort( &sv.multicast, index );
		MSG_WriteString( &sv.multicast, val );

		SV_Multicast(vec3_origin, MULTICAST_ALL_R);
	}
}

void SV_WriteChar(int c)
{
	MSG_WriteChar( &sv.multicast, c );
}

void SV_WriteByte(int c)
{
	MSG_WriteByte( &sv.multicast, c );
}

void SV_WriteShort(int c)
{
	MSG_WriteShort( &sv.multicast, c );
}

void SV_WriteLong(int c)
{
	MSG_WriteLong( &sv.multicast, c );
}

void SV_WriteFloat(float f)
{
	MSG_WriteFloat( &sv.multicast, f );
}

void SV_WriteString(char *s)
{
	MSG_WriteString( &sv.multicast, s );
}

void SV_WritePos(vec3_t pos)
{
	MSG_WritePos( &sv.multicast, pos );
}

void SV_WriteDir(vec3_t dir)
{
	MSG_WriteDir( &sv.multicast, dir );
}

void SV_WriteAngle(float f)
{
	MSG_WriteAngle( &sv.multicast, f );
}


/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS(vec3_t p1, vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte * 	mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	area1 	= CM_LeafArea( leafnum );
	mask 	= CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
	area2 	= CM_LeafArea( leafnum );
	
	if ( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) ) {
		return false;
	}

	if ( !CM_AreasConnected (area1, area2 ) ) {
		return false; 			// a door blocks sight
	}

	return true;
}


/*
=================
SV_inPHS

Also checks portalareas so that doors block sound
=================
*/
qboolean SV_inPHS(vec3_t p1, vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte * 	mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	area1 	= CM_LeafArea( leafnum );
	mask 	= CM_ClusterPHS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
	area2 	= CM_LeafArea( leafnum );
	
	if ( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7) ) ) ) ) {
		return false;		// more than one bounce away
	}

	if ( !CM_AreasConnected( area1, area2 ) ) {
		return false;		// a door blocks hearing
	}

	return true;
}

/*
=================
SV_GameStartSound
=================
*/
void SV_GameStartSound(edict_t * entity, int channel, int sound_num,
	float volume, float attenuation, float timeofs)
{
	if ( !entity ) {
		return;
	}

	SV_StartSound(NULL, entity, channel, sound_num, volume, attenuation,
		timeofs);
}

//==============================================

/*
===============
SV_ShutdownGameProgs

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
void SV_ShutdownGameProgs(void)
{
	Common_UnloadGameDLL();
}

/*
===============
SV_InitGameProgs

Init the game subsystem for a new map
===============
*/
void SV_InitGameProgs (void)
{
	// unload anything we have now
	if ( game ) {
		SV_ShutdownGameProgs();
	}

	Common_LoadGameDLL();
}

