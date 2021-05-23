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

#include <ctype.h>
#include "client.h"

/* key up events are sent even if in console mode */

#define		MAXCMDLINE	256
char	key_lines[32][MAXCMDLINE];
int		key_linepos;
int	anykeydown;

int		edit_line=0;
int		history_line=0;

char * keybindings[MAX_KEYS];
qboolean consolekeys[MAX_KEYS];	// if true, can't be rebound while in console
qboolean menubound[MAX_KEYS];		// if true, can't be rebound while in menu
// keydown and keyrepeat state, > 1 is autorepeating
unsigned char keydown[MAX_KEYS];

typedef struct keyname_s
{
	char * name;
	int keynum;
} keyname_t;

static keyname_t keynames[] =
{
	{ "TAB", K_TAB },
	{ "ENTER", K_ENTER },
	{ "ESCAPE", K_ESCAPE },
	{ "SPACE", K_SPACE },

	{ "SEMICOLON", ';' },
	{ "DOUBLEQUOTE", '"' },
	{ "DOLLAR", '$' },
	{ "PLUS", '+' },
	{ "MINUS", '-' },

	{ "BACKSPACE", K_BACKSPACE },

	{ "UPARROW", K_UPARROW },
	{ "DOWNARROW", K_DOWNARROW },
	{ "LEFTARROW", K_LEFTARROW },
	{ "RIGHTARROW", K_RIGHTARROW },
	
	{ "LWIN", K_LWIN },
	{ "RWIN", K_RWIN },
	{ "MENU", K_MENU },

	{ "ALT", K_ALT },
	{ "CTRL", K_CTRL },
	{ "SHIFT", K_SHIFT },

	{ "COMMAND", K_COMMAND },

	{ "CAPSLOCK", K_CAPSLOCK },
	{ "SCROLL", K_SCROLL },
	
	{ "F1", K_F1 },
	{ "F2", K_F2 },
	{ "F3", K_F3 },
	{ "F4", K_F4 },
	{ "F5", K_F5 },
	{ "F6", K_F6 },
	{ "F7", K_F7 },
	{ "F8", K_F8 },
	{ "F9", K_F9 },
	{ "F10", K_F10 },
	{ "F11", K_F11 },
	{ "F12", K_F12 },

	{ "INS", K_INS },
	{ "DEL", K_DEL },
	{ "PGDN", K_PGDN },
	{ "PGUP", K_PGUP },
	{ "HOME", K_HOME },
	{ "END", K_END },

	{ "MOUSE1", K_MOUSE1 },
	{ "MOUSE2", K_MOUSE2 },
	{ "MOUSE3", K_MOUSE3 },
	{ "MOUSE4", K_MOUSE4 },
	{ "MOUSE5", K_MOUSE5 },

	{ "MWHEELUP", K_MWHEELUP },
	{ "MWHEELDOWN", K_MWHEELDOWN },

	{ "JOY1", K_JOY1 },
	{ "JOY2", K_JOY2 },
	{ "JOY3", K_JOY3 },
	{ "JOY4", K_JOY4 },
	{ "JOY5", K_JOY5 },
	{ "JOY6", K_JOY6 },
	{ "JOY7", K_JOY7 },
	{ "JOY8", K_JOY8 },
	{ "JOY9", K_JOY9 },
	{ "JOY10", K_JOY10 },
	{ "JOY11", K_JOY11 },
	{ "JOY12", K_JOY12 },
	{ "JOY13", K_JOY13 },
	{ "JOY14", K_JOY14 },
	{ "JOY15", K_JOY15 },
	{ "JOY16", K_JOY16 },
	{ "JOY17", K_JOY17 },
	{ "JOY18", K_JOY18 },
	{ "JOY19", K_JOY19 },
	{ "JOY20", K_JOY20 },
	{ "JOY21", K_JOY21 },
	{ "JOY22", K_JOY22 },
	{ "JOY23", K_JOY23 },
	{ "JOY24", K_JOY24 },
	{ "JOY25", K_JOY25 },
	{ "JOY26", K_JOY26 },
	{ "JOY27", K_JOY27 },
	{ "JOY28", K_JOY28 },
	{ "JOY29", K_JOY29 },
	{ "JOY30", K_JOY30 },
	{ "JOY31", K_JOY31 },
	{ "JOY32", K_JOY32 },

	{ "AUX1", K_AUX1 },
	{ "AUX2", K_AUX2 },
	{ "AUX3", K_AUX3 },
	{ "AUX4", K_AUX4 },
	{ "AUX5", K_AUX5 },
	{ "AUX6", K_AUX6 },
	{ "AUX7", K_AUX7 },
	{ "AUX8", K_AUX8 },
	{ "AUX9", K_AUX9 },
	{ "AUX10", K_AUX10 },
	{ "AUX11", K_AUX11 },
	{ "AUX12", K_AUX12 },
	{ "AUX13", K_AUX13 },
	{ "AUX14", K_AUX14 },
	{ "AUX15", K_AUX15 },
	{ "AUX16", K_AUX16 },

	{ "KP_HOME", K_KP_HOME },
	{ "KP_UPARROW",	K_KP_UPARROW },
	{ "KP_PGUP", K_KP_PGUP },
	{ "KP_LEFTARROW", K_KP_LEFTARROW },
	{ "KP_5", K_KP_5 },
	{ "KP_RIGHTARROW", K_KP_RIGHTARROW },
	{ "KP_END",	K_KP_END },
	{ "KP_DOWNARROW", K_KP_DOWNARROW },
	{ "KP_PGDN", K_KP_PGDN },
	{ "KP_ENTER", K_KP_ENTER },
	{ "KP_INS",	K_KP_INS },
	{ "KP_DEL",	K_KP_DEL },
	{ "KP_SLASH", K_KP_SLASH },
	{ "KP_MINUS", K_KP_MINUS },
	{ "KP_PLUS", K_KP_PLUS },
	{ "KP_NUMLOCK",	K_KP_NUMLOCK },
	{ "KP_STAR", K_KP_STAR },
	{ "KP_EQUALS", K_KP_EQUALS },

	{ "PAUSE", K_PAUSE },

	{ NULL,	0 }
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/

void CompleteCommand (void)
{
	char	*cmd, *s;

	s = key_lines[edit_line]+1;
	if (*s == '\\' || *s == '/')
		s++;

	cmd = Cmd_CompleteCommand (s);
	if (!cmd)
		cmd = Cvar_CompleteVariable (s);
	if (cmd)
	{
		key_lines[edit_line][1] = '/';
		strcpy (key_lines[edit_line]+2, cmd);
		key_linepos = strlen(cmd)+2;
		key_lines[edit_line][key_linepos] = ' ';
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
		return;
	}
}

/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
void Key_Console (int key)
{

	switch ( key )
	{
	case K_KP_SLASH:
		key = '/';
		break;
	case K_KP_MINUS:
		key = '-';
		break;
	case K_KP_PLUS:
		key = '+';
		break;
	case K_KP_HOME:
		key = '7';
		break;
	case K_KP_UPARROW:
		key = '8';
		break;
	case K_KP_PGUP:
		key = '9';
		break;
	case K_KP_LEFTARROW:
		key = '4';
		break;
	case K_KP_5:
		key = '5';
		break;
	case K_KP_RIGHTARROW:
		key = '6';
		break;
	case K_KP_END:
		key = '1';
		break;
	case K_KP_DOWNARROW:
		key = '2';
		break;
	case K_KP_PGDN:
		key = '3';
		break;
	case K_KP_INS:
		key = '0';
		break;
	case K_KP_DEL:
		key = '.';
		break;
	}

	if ( ( tolower( key ) == 'v' && keydown[K_CTRL] ) ||
		( ( key == K_INS || key == K_KP_INS ) && keydown[K_SHIFT] ) ) {
		
		char * cbd;
		
		if ( ( cbd = Sys_GetClipboardData() ) != 0 ) {

			int i;

			i = strlen( cbd );
			
			if ( i + key_linepos >= MAXCMDLINE ) {
				i = MAXCMDLINE - key_linepos;
			}

			if ( i > 0 ) {

				cbd[i] = 0;
				strcat( key_lines[edit_line], cbd );
				key_linepos += i;
			}

			free( cbd );
		}

		return;
	}

	if ( tolower( key ) == 'l' && keydown[K_CTRL] ) {

		Cbuf_AddText( "clear\n" );
		return;
	}

	if ( key == K_ENTER || key == K_KP_ENTER )
	{	// backslash text are commands, else chat
		if (key_lines[edit_line][1] == '\\' || key_lines[edit_line][1] == '/')
			Cbuf_AddText (key_lines[edit_line]+2);	// skip the >
		else
			Cbuf_AddText (key_lines[edit_line]+1);	// valid command

		Cbuf_AddText ("\n");
		Com_Printf ("%s\n",key_lines[edit_line]);
		edit_line = (edit_line + 1) & 31;
		history_line = edit_line;
		key_lines[edit_line][0] = ']';
		key_linepos = 1;
		if (cls.state == ca_disconnected)
			SCR_UpdateScreen ();	// force an update, because the command
									// may take some time
		return;
	}

	if (key == K_TAB)
	{	// command completion
		CompleteCommand ();
		return;
	}
	
	// TODO:: add interactive line editing with K_LEFTARROW, K_RIGHTARROW
	if ( ( key == K_BACKSPACE ) || ( key == K_LEFTARROW ) || ( key == K_KP_LEFTARROW ) || ( ( key == 'h' ) && ( keydown[K_CTRL] ) ) )
	{
		if (key_linepos > 1)
			key_linepos--;
		return;
	}

	if ( ( key == K_MWHEELUP && keydown[K_SHIFT] )
		|| ( key == K_UPARROW ) || ( key == K_KP_UPARROW )
		|| ( ( tolower( key ) == 'p' ) && keydown[K_CTRL] ) ) {
		
		do {
			history_line = ( history_line - 1 ) & 31;
		} while ( history_line != edit_line && !key_lines[history_line][1] );
		
		if ( history_line == edit_line ) {
			history_line = ( edit_line + 1 ) & 31;
		}
		
		strcpy( key_lines[edit_line], key_lines[history_line] );
		key_linepos = strlen( key_lines[edit_line] );
		
		return;
	}

	if ( ( key == K_MWHEELDOWN && keydown[K_SHIFT] )
		|| ( key == K_DOWNARROW ) || ( key == K_KP_DOWNARROW )
		|| ( ( tolower( key ) == 'n' ) && keydown[K_CTRL] ) ) {
		
		if ( history_line == edit_line ) return;
		
		do {
			history_line = ( history_line + 1 ) & 31;
		} while ( history_line != edit_line && !key_lines[history_line][1] );
		
		if ( history_line == edit_line ) {

			key_lines[edit_line][0] = ']';
			key_linepos = 1;

		} else {

			strcpy( key_lines[edit_line], key_lines[history_line] );
			key_linepos = strlen( key_lines[edit_line] );
		}

		return;
	}

	if (key == K_PGUP || key == K_KP_PGUP )
	{
		con.display -= 2;
		return;
	}

	if (key == K_PGDN || key == K_KP_PGDN ) 
	{
		con.display += 2;
		if (con.display > con.current)
			con.display = con.current;
		return;
	}

	if (key == K_HOME || key == K_KP_HOME )
	{
		con.display = con.current - con.totallines + 10;
		return;
	}

	if (key == K_END || key == K_KP_END )
	{
		con.display = con.current;
		return;
	}
	
	if (key < 32 || key > 127)
		return;	// non printable
		
	if (key_linepos < MAXCMDLINE-1)
	{
		key_lines[edit_line][key_linepos] = ( char )key;
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
	}
}

/*
===================
Key_IsDown
===================
*/
qboolean Key_IsDown(int keynum)
{
	if ( keynum == -1 ) {
		return false;
	}

	return keydown[keynum] != 0;
}

/*
===================
Key_AnyKeyDown
===================
*/

int Key_AnyKeyDown(void)
{
	return anykeydown;
}

/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at the given
string. Single ascii characters return themselves, while the K_* names are
matched up.
===================
*/
int Key_StringToKeynum(char * str)
{
	keyname_t * kn;
	
	if ( !str || !str[0] ) {
		return -1;
	}

	if ( !str[1] ) {
		return str[0];
	}

	for (kn = keynames; kn->name; kn++)
	{
		if ( !Q_strcasecmp( str, kn->name ) ) {
			return kn->keynum;
		}
	}

	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the given
keynum.
===================
*/
char * Key_KeynumToString(int keynum)
{
	keyname_t * kn;	
	static char tinystr[2] = { 0 };
	
	if ( keynum == -1 ) {
		return "<KEY NOT FOUND>";
	}


	if ( ( keynum > 32 ) && ( keynum < 127 ) && keynum != ';' && keynum != '"'
		&& keynum != '$' && keynum != '+' && keynum != '-') {
		
		/* printable ascii, except for quotes or general escape sequences ';',
		'$' used to expand cvars to values in macros or commands and '+' or '-'
		that are used in configuration files and on the command line */

		tinystr[0] = ( char )keynum;
		tinystr[1] = 0;

		return tinystr;
	}
	
	for (kn = keynames; kn->name; kn++)
	{
		if ( keynum == kn->keynum ) {
			return kn->name;
		}
	}

	return "<UNKNOWN KEYNUM>";
}

/*
===================
Key_GetKeyBindNum

keynum offset into keybindings to begin search.
===================
*/
int Key_GetKeyBindNum(int keynum, char * binding)
{
	int i;

	if (keynum < 0) {
		keynum = 0;
	}

	for (i = keynum; i < 256; i++) {
		if ( keybindings[i] && !Q_stricmp( keybindings[i], binding ) ) {
			return i;
		}
	}

	return -1;
}

/*
===================
Key_GetKeyBindName
===================
*/
char * Key_GetKeyBindName(char * binding)
{
	return Key_KeynumToString( Key_GetKeyBindNum( 0, binding ) );
}

/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding(int keynum, char * binding)
{
	char * newbind;
	int	l;
			
	if ( keynum == -1 ) {
		return;
	}

	/* free old bindings */
	if ( keybindings[keynum] ) {

		Z_Free( keybindings[keynum] );
		keybindings[keynum] = NULL;
	}

	/* allocate memory for new binding */
	l = strlen( binding );
	
	newbind = Z_Malloc( l + 1 );
	strcpy( newbind, binding );
	newbind[l] = 0;
	
	keybindings[keynum] = newbind;	
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("unbind <key> : remove commands from a key\n");
		return;
	}
	
	b = Key_StringToKeynum( Cmd_Argv( 1 ) );

	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, "");
}

void Key_Unbindall_f (void)
{
	int		i;
	
	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			Key_SetBinding (i, "");
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];
	
	c = Cmd_Argc();

	if (c < 2)
	{
		Com_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}

	b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Com_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b] );
		else
			Com_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}
	
// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
		strcat (cmd, Cmd_Argv(i));
		if (i != (c-1))
			strcat (cmd, " ");
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			fprintf (f, "bind %s \"%s\"\n", Key_KeynumToString( i ), keybindings[i]);
}


/*
============
Key_Bindlist_f

============
*/
void Key_Bindlist_f (void)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			Com_Printf ("%s \"%s\"\n", Key_KeynumToString( i ), keybindings[i]);
}


/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;
	int knum;

	knum = K_NUM;

	if ( knum > MAX_KEYS ) {

		Com_Error( ERR_FATAL, "Key number exceeds maximum." );
		return;
	}

	for (i=0 ; i<32 ; i++)
	{
		key_lines[i][0] = ']';
		key_lines[i][1] = 0;
	}
	key_linepos = 1;
	
//
// init ascii characters in console mode
//
	for (i=32 ; i<128 ; i++)
		consolekeys[i] = true;
	consolekeys[K_ENTER] = true;
	consolekeys[K_KP_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_KP_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_KP_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_KP_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_KP_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_KP_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_KP_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_KP_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_KP_PGDN] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_KP_INS] = true;
	consolekeys[K_KP_DEL] = true;
	consolekeys[K_KP_SLASH] = true;
	consolekeys[K_KP_STAR] = true;
	consolekeys[K_KP_PLUS] = true;
	consolekeys[K_KP_MINUS] = true;
	consolekeys[K_KP_5] = true;
	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;
	consolekeys[K_MOUSE4] = true;
	consolekeys[K_MOUSE5] = true;

	consolekeys['`'] = false;
	consolekeys['~'] = false;

	menubound[K_ESCAPE] = true;
	for (i=0 ; i<12 ; i++)
		menubound[K_F1+i] = true;

//
// register our functions
//
	Cmd_AddCommand ("bind",Key_Bind_f);
	Cmd_AddCommand ("unbind",Key_Unbind_f);
	Cmd_AddCommand ("unbindall",Key_Unbindall_f);
	Cmd_AddCommand ("bindlist",Key_Bindlist_f);
}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
void Key_Event (int key, qboolean down, unsigned time)
{
	cvar_t * fullscreen;
	char	*kb;
	char	cmd[1024];
	
	// update auto-repeat status
	if ( down ) {

		if ( keydown[key] < 255 ) {
			keydown[key]++;
		}

		if ( key != K_BACKSPACE 
			&& key != K_PAUSE 
			&& key != K_PGUP 
			&& key != K_KP_PGUP 
			&& key != K_PGDN
			&& key != K_KP_PGDN
			&& keydown[key] > 1 ) {
			return;	// ignore most autorepeats
		}
			
		if ( key >= 200 && !keybindings[key] ) {
			Com_Printf( "%s is unbound, hit F4 to set.\n",
				Key_KeynumToString( key ) );
		}
	} else {
		keydown[key] = 0;
	}

	if ( keydown[K_ALT] && ( down && key == K_ENTER ) ) {

		fullscreen = Cvar_Get( "vid_fullscreen", "0", CVAR_ARCHIVE );
		Cvar_SetValue( "vid_fullscreen", !fullscreen->value );
		Cbuf_AddText( "vid_restart\n" );

		return;
	}

	// console key is hardcoded, so the user can never unbind it
	if ( key == '`' || key == '~' ) {
		
		if ( !down ) {
			return;
		}

		Cbuf_AddText( "toggleconsole clear\n" );
		
		return;
	}

	if ( !cls.disable_screen ) {

		/* any key during the attract mode will bring up the menu */
		if ( cl.attractloop && cls.key_dest != key_menu &&
			!( key >= K_F1 && key <= K_F12 ) ) {
			key = K_ESCAPE;
		}

		/* menu key is hardcoded, so the user can never unbind it */
		if ( key == K_ESCAPE ) {

			if ( !down ) {
				return;
			}

			if ( cl.frame.playerstate.stats[STAT_LAYOUTS] &&
				cls.key_dest == key_game ) {
				
				/* put away help computer / inventory */
				Cbuf_AddText( "cmd putaway\n" );
				
				return;
			}

			switch ( cls.key_dest )
			{
				case key_message:
				{
					Key_Message( key );
				} break;
				case key_menu:
				{
					M_Keydown( key );
				} break;
				case key_game:
				case key_console:
				{
					M_Menu_Main_f();
				} break;
				default:
				{
					Com_Error( ERR_FATAL, "Bad cls.key_dest" );
				}
			}

			return;
		}
	}

	// track if any key is down for BUTTON_ANY
	keydown[key] = down;
	
	if ( down ) {
		if ( keydown[key] == 1 ) {
			anykeydown++;
		}
	} else {

		anykeydown--;

		if ( anykeydown < 0 ) {
			anykeydown = 0;
		}
	}

//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
	if (!down)
	{
		kb = keybindings[key];
		if (kb && kb[0] == '+')
		{
			Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
			Cbuf_AddText (cmd);
		}

		return;
	}

//
// if not a consolekey, send to the interpreter no matter what mode is
//
	if ( (cls.key_dest == key_menu && menubound[key])
	|| (cls.key_dest == key_console && !consolekeys[key])
	|| (cls.key_dest == key_game && ( cls.state == ca_active || !consolekeys[key] ) ) )
	{
		kb = keybindings[key];
		if (kb)
		{
			if (kb[0] == '+')
			{	// button commands add keynum and time as a parm
				Com_sprintf (cmd, sizeof(cmd), "%s %i %i\n", kb, key, time);
				Cbuf_AddText (cmd);
			}
			else
			{
				Cbuf_AddText (kb);
				Cbuf_AddText ("\n");
			}
		}
		return;
	}

	if (!down)
		return;		// other systems only care about key down events

	switch (cls.key_dest)
	{
	case key_message:
		Key_Message (key);
		break;
	case key_menu:
		M_Keydown (key);
		break;
	case key_game:
	case key_console:
		Key_Console (key);
		break;
	default:
		Com_Error (ERR_FATAL, "Bad cls.key_dest");
	}
}

/*
===================
Key_ClearTyping
===================
*/
void Key_ClearTyping(void)
{
	key_lines[edit_line][1] = 0;			// clear any typing
	key_linepos = 1;
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates(void)
{
	int i;

	anykeydown = false;

	for (i = 0; i < MAX_KEYS; i++) {
		if ( keydown[i] ) {
			Key_Event( i, false, 0 );
		}

		keydown[i] = 0;
	}
}
