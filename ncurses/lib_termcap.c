
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of ncurses in any   *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/

#include <curses.priv.h>

#include <string.h>
#include <termcap.h>
#include <tic.h>

#define __INTERNAL_CAPS_VISIBLE
#include <term.h>

MODULE_ID("$Id: lib_termcap.c,v 1.11 1996/08/17 22:31:33 tom Exp $")

/*
   some of the code in here was contributed by:
   Magnus Bengtsson, d6mbeng@dtek.chalmers.se
*/

char PC;
char *UP;
char *BC;
short ospeed;

/***************************************************************************
 *
 * tgetent(bufp, term)
 *
 * In termcap, this function reads in the entry for terminal `term' into the
 * buffer pointed to by bufp. It must be called before any of the functions
 * below are called.
 * In this terminfo emulation, tgetent() simply calls setupterm() (which
 * does a bit more than tgetent() in termcap does), and returns its return
 * value (1 if successful, 0 if no terminal with the given name could be
 * found, or -1 if no terminal descriptions have been installed on the
 * system).  The bufp argument is ignored.
 *
 ***************************************************************************/

int tgetent(char *bufp GCC_UNUSED, const char *name)
{
int errcode;

	T(("calling tgetent"));
	setupterm((char *)name, STDOUT_FILENO, &errcode);

	if (errcode != 1)
		return(errcode);

        if (cursor_left)
	    if ((backspaces_with_bs = !strcmp(cursor_left, "\b")) == 0)
		backspace_if_not_bs = cursor_left;

	/* we're required to export these */
	if (pad_char != NULL)
		PC = pad_char[0];
	if (cursor_up != NULL)
		UP = cursor_up;
	if (backspace_if_not_bs != NULL)
		BC = backspace_if_not_bs;
#if defined(TERMIOS)
	/*
	 * Back-convert to the funny speed encoding used by the old BSD
	 * curses library.  Method suggested by Andrey Chernov
	 * <ache@astral.msk.su>
	 */
	if ((ospeed = cfgetospeed(&cur_term->Nttyb)) < 0)
	    ospeed = 1;		/* assume lowest non-hangup speed */
	else
	{
		const short *sp;
		static const short speeds[] = {
#ifdef B115200
			B115200,
#endif
#ifdef B57600
			B57600,
#endif
#ifdef B38400
			B38400,
#else
#ifdef EXTB
			EXTB,
#endif
#endif /* B38400 */
#ifdef B19200
			B19200,
#else
#ifdef EXTA
			EXTA,
#endif
#endif /* B19200 */
			B9600,
			B4800,
			B2400,
			B1800,
			B1200,
			B600,
			B300,
			B200,
			B150,
			B134,
			B110,
			B75,
			B50,
			B0,
		};
#define MAXSPEED	sizeof(speeds)/sizeof(speeds[0])

		for (sp = speeds; sp < speeds + MAXSPEED; sp++) {
			if (sp[0] <= ospeed) {
				break;
			}
		}
		ospeed = MAXSPEED - (sp - speeds);
	}
#else
	ospeed = cur_term->Nttyb.sg_ospeed;
#endif

/* LINT_PREPRO
#if 0*/
#include <capdefaults.c>
/* LINT_PREPRO
#endif*/

	return errcode;
}

/***************************************************************************
 *
 * tgetflag(str)
 *
 * Look up boolean termcap capability str and return its value (TRUE=1 if
 * present, FALSE=0 if not).
 *
 ***************************************************************************/

int tgetflag(const char *id)
{
int i;

	T(("tgetflag: %s", id));
	for (i = 0; i < BOOLCOUNT; i++)
		if (!strcmp(id, boolcodes[i]))
			return cur_term->type.Booleans[i];
	return ERR;
}

/***************************************************************************
 *
 * tgetnum(str)
 *
 * Look up numeric termcap capability str and return its value, or -1 if
 * not given.
 *
 ***************************************************************************/

int tgetnum(const char *id)
{
int i;

	T(("tgetnum: %s", id));
	for (i = 0; i < NUMCOUNT; i++)
		if (!strcmp(id, numcodes[i]))
			return cur_term->type.Numbers[i];
	return ERR;
}

/***************************************************************************
 *
 * tgetstr(str, area)
 *
 * Look up string termcap capability str and return a pointer to its value,
 * or NULL if not given.
 *
 ***************************************************************************/

char *tgetstr(const char *id, char **area GCC_UNUSED)
{
int i;

	T(("tgetstr: %s", id));
	for (i = 0; i < STRCOUNT; i++) {
		T(("trying %s", strcodes[i]));
		if (!strcmp(id, strcodes[i])) {
			T(("found match : %s", cur_term->type.Strings[i]));
			return cur_term->type.Strings[i];
		}
	}
	return NULL;
}

/*
 *	char *
 *	tgoto(string, x, y)
 *
 *	Retained solely for upward compatibility.  Note the intentional
 *	reversing of the last two arguments.
 *
 */

char *tgoto(const char *string, int x, int y)
{
	return(tparm((char *)string, y, x));
}
