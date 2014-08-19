/**************************************************************************
*** Project: SGF Syntax Checker & Converter
***	File:	 main.c
***
*** Copyright (C) 1996-2006 by Arno Hollosi
***
*** Copyright notice:
***
*** This program is open source software; you can redistribute it
*** and/or modify it under the terms of the BSD License (see file COPYING)
***
*** This program is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
***
*** The author can be reached at <ahollosi@xmp.net>
***
***		Arno Hollosi
***		Sonnbichlstrasse 50
***		A-6322 Kirchbichl
***		Austria (Europe)
***
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "all.h"
#include "protos.h"

struct SGFInfo sgf;

struct SGFInfo *sgfc = NULL;
						/* pointer to current (actual) SGFInfo structure
						** used by PrintError/ LoadSGF & ParseSGF
						** the latter two set this pointer on calling */

char option_warnings = TRUE;
char option_keep_head = FALSE;
char option_keep_unknown_props = TRUE;
char option_keep_obsolete_props = TRUE;
char option_del_empty_nodes = FALSE;
char option_del_move_markup = FALSE;
char option_split_file = FALSE;
char option_write_critical = FALSE;
char option_interactive = FALSE;
char option_linebreaks = 1;
char option_softlinebreaks = TRUE;
char option_expandcpl = FALSE;
char option_pass_tt = FALSE;
char option_fix_variation = FALSE;
char option_findstart = 1;
char option_game_signature = FALSE;
char option_strict_checking = FALSE;
char option_reorder_variations = FALSE;
char *option_infile = NULL;
char *option_outfile = NULL;


/**************************************************************************
*** Function:	PrintHelp
***				Prints banner + options
**************************************************************************/

void PrintHelp(int everything)
{
	puts(
		" SGFC V1.16  - Smart Game Format Syntax Checker & Converter\n"
		"               Copyright (C) 1996-2006 by Arno Hollosi\n"
		"               Email: <ahollosi@xmp.net>\n"
		" ----------------------------------------------------------");

	if(everything)
		puts(
		" sgfc [options] infile [outfile]\n"
		" Options:\n"
		"    -h  ... print this help message\n"
		"    -bx ... x = 1,2,3: beginning of SGF data is detected by\n"
		"              1 - sophisticated search algorithm (default)\n"
		"              2 - first occurence of '(;'\n"
		"              3 - first occurence of '('\n"
		"    -c  ... write file even if a critical error occurs\n"
		"    -dn ... n = number : disable message number -n-\n"
		"    -e  ... expand compressed point lists\n"
		"    -g  ... print game signature (Go GM[1] games only)\n"
		"    -i  ... interactive mode (faulty game-info values only)\n"
		"    -k  ... keep header in front of SGF data\n"
		"    -lx ... x = 1,2,3,4: a hard linebreak is\n"
		"              1 - any linebreak encountered (default)\n"
		"              2 - any linebreak not preceeded by a space (MGT)\n"
		"              3 - two linebreaks in a row\n"
		"              4 - paragraph style (ISHI format, MFGO)\n"
		"    -m  ... delete markup on current move\n"
		"    -n  ... delete empty nodes\n"
		"    -o  ... delete obsolete properties\n"
		"    -p  ... write pass moves as '[tt]' if possible\n"
		"    -r  ... restrictive checking\n"
		"    -s  ... split game collection into single files\n"
		"    -t  ... do not insert any soft linebreaks into text values\n"
		"    -u  ... delete unknown properties\n"
		"    -v  ... correct variation level and root moves\n"
		"    -w  ... disable warning messages\n"
		"    -yP ... delete property P (P = property id)\n"
		"    -z  ... reverse ordering of variations"
		);
	else
		puts(" 'sgfc -h' for help on options");
}


/**************************************************************************
*** Function:	ParseArgs
***				Parses commandline options
***				Options are represented by one char and are preceeded with
***				a minus. It's valid to list more than one option per argv[]
*** Parameters: argc ... argument count (like main())
***				argv ... arguments (like main())
*** Returns:	TRUE for ok / FALSE for exit program (help printed)
**************************************************************************/

int ParseArgs(int argc, char *argv[])
{
	int i, n, m;
	char *c, *hlp;

	if(argc <= 1)		/* called without arguments */
	{
		PrintHelp(FALSE);
		return(FALSE);
	}

	for(i = 1; i < argc; i++)
	{
		switch(argv[i][0])
		{
			case '-':
				for(c = &argv[i][1]; *c; c++)
				{
					switch(*c)
					{
						case 'w':	option_warnings = FALSE;			break;
						case 'u':	option_keep_unknown_props = FALSE;	break;
						case 'o':	option_keep_obsolete_props = FALSE;	break;
						case 'c':	option_write_critical = TRUE;		break;
						case 'e':	option_expandcpl = TRUE;			break;
						case 'k':	option_keep_head = TRUE;			break;
						case 't':	option_softlinebreaks = FALSE;		break;
						case 'p':	option_pass_tt = TRUE;				break;
						case 's':	option_split_file = TRUE;			break;
						case 'n':	option_del_empty_nodes = TRUE;		break;
						case 'm':	option_del_move_markup = TRUE;		break;
						case 'v':	option_fix_variation = TRUE;		break;
						case 'i':	option_interactive = TRUE;			break;
						case 'g':	option_game_signature = TRUE;		break;
						case 'r':	option_strict_checking = TRUE;		break;
						case 'z':	option_reorder_variations = TRUE;	break;
						case 'd':	c++; hlp = c;
									n = (int)strtol(c, &c, 10);
									if(n < 1 || n > MAX_ERROR_NUM)
										PrintError(FE_BAD_PARAMETER, hlp);
									error_enabled[n-1] = FALSE;
									c--;
									break;
						case 'l':	c++;
									n = *c - '0';
									if(n < 1 || n > 4)
										PrintError(FE_BAD_PARAMETER, c);
									option_linebreaks = n;
									break;
						case 'b':	c++;
									n = *c - '0';
									if(n < 1 || n > 3)
										PrintError(FE_BAD_PARAMETER, c);
									option_findstart = n;
									break;
						case 'y':	c++;
									for(n = 0; isupper(*c); c++, n++);
									m = 0;
									c -= n;
									if(n)
									{
										for(m = 1; sgf_token[m].id; m++)
											if(!strnccmp(c, sgf_token[m].id, n))
												break;
									}
									if(!n || !sgf_token[m].id)
										PrintError(FE_BAD_PARAMETER, c);
									else
									{
										c += n-1;
										sgf_token[m].flags |= DELETE_PROP;
									}
									break;
						case 'h':	PrintHelp(TRUE);
									return(FALSE);
						case '-':	/* long options */
									c++;
									if(!strncmp(c, "help", 4))
									{
										PrintHelp(TRUE);
										return(FALSE);
									}
									if(!strncmp(c, "version", 7))
									{
										PrintHelp(FALSE);
										return(FALSE);
									}
									PrintError(FE_UNKNOWN_LONG_OPTION, c);
									break;
						default:	PrintError(FE_UNKNOWN_OPTION, *c);
									break;
					}
				}
				break;

			default:			/* argument isn't preceeded by '-' */
				if(!option_infile)
					option_infile = argv[i];
				else
				if(!option_outfile)
					option_outfile = argv[i];
				else
					PrintError(FE_TOO_MANY_FILES, argv[i]);
				break;
		}
	}

	if(!option_infile)
		PrintError(FE_MISSING_SOURCE_FILE);

	return(TRUE);
}


/**************************************************************************
*** Function:	main
*** Parameters: as usual
*** Returns:	 0 on success
***				 5 if there were warnings
***				10 if there were errors
***				20 if a fatal error occured
**************************************************************************/

#ifndef VERSION_NO_MAIN

int main(int argc, char *argv[])
{
	int ret = 0;

	memset(error_enabled, TRUE, sizeof(error_enabled));

	if(!ParseArgs(argc, argv))
		return(0);

	memset(&sgf, 0, sizeof(struct SGFInfo));	/* init SGFInfo structure */
	sgf.name = option_infile;

	LoadSGF(&sgf);
	ParseSGF(&sgf);

	if(option_outfile)
	{
		if(option_write_critical || !critical_count)
		{
			sgf.name = option_outfile;
			SaveSGF(&sgf);
		}
		else
			PrintError(E_CRITICAL_NOT_SAVED);
	}

	fprintf(E_OUTPUT, "%s: ", option_infile);	/* print status line */

	if(error_count || warning_count)			/* errors & warnings */
	{
		ret = 5;
		if(error_count)
		{
			fprintf(E_OUTPUT, "%d error(s)  ", error_count);
			ret = 10;
		}

		if(warning_count)
			fprintf(E_OUTPUT, "%d warning(s)  ", warning_count);

		if(critical_count)						/* critical ones */
			fprintf(E_OUTPUT, "(critical:%d)  ", critical_count);
	}
	else										/* file ok */
		fprintf(E_OUTPUT, "OK  ");

	if(ignored_count)
		fprintf(E_OUTPUT, "(%d message(s) ignored)", ignored_count);

	fprintf(E_OUTPUT, "\n");

	FreeSGFInfo(&sgf);
	return(ret);
}
#endif
