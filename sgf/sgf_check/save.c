/**************************************************************************
*** Project: SGF Syntax Checker & Converter
***	File:	 save.c
***
*** Copyright (C) 1996-2004 by Arno Hollosi
*** (see 'main.c' for more copyright information)
***
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "all.h"
#include "protos.h"


int save_linelen;

#define MAX_LINELEN		58
#define MAXTEXT_LINELEN 70

#define saveputc(f,c) { if(!WriteChar((f), (c), FALSE))	return(FALSE);	}

#define CheckLineLen(s) { if(save_linelen > MAX_LINELEN) \
						{ saveputc(s, '\n')	} }


/**************************************************************************
*** Function:	WriteChar
***				Writes char to file, modifies save_linelen
***				transforms EndOfLine-Char if necessary
*** Parameters: sfile ... file handle
***				c     ... char to be written
***				spc   ... convert spaces to EOL if line is too long
***                       (used only on PVT_SIMPLE property values)
*** Returns:	TRUE or FALSE
**************************************************************************/

int WriteChar(FILE *sfile, char c, int spc)
{

	if(spc && isspace(c) && (save_linelen >= MAXTEXT_LINELEN))
		c = '\n';

	if(c != '\n')
	{
		save_linelen++;

		if(fputc(c, sfile) == EOF)
			return(FALSE);
	}
	else
	{
		save_linelen = 0;

#if EOLCHAR
		if(fputc(EOLCHAR, sfile) == EOF)
			return(FALSE);
#else
		if(fputc('\r', sfile) == EOF)		/* MSDOS EndOfLine */
			return(FALSE);
		if(fputc('\n', sfile) == EOF)
			return(FALSE);
#endif
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	WritePropValue
***				Value into the given file
***				does necessary conversions
*** Parameters: v ... pointer to value
***				second ... TRUE: write value2, FALSE:write value1
***				flags ... property flags
***				sfile .. file handle
*** Returns:	TRUE or FALSE
**************************************************************************/

int WritePropValue(char *v, int second, U_SHORT flags, FILE *sfile)
{
	U_SHORT fl;

	if(!v)	return(TRUE);
	
	if(second)
		saveputc(sfile, ':');

	fl = option_softlinebreaks && (flags & SPLIT_SAVE);

	while(*v)
	{
		if(!WriteChar(sfile, *v, flags & PVT_SIMPLE))
			return(FALSE);

		if(fl && (save_linelen > MAXTEXT_LINELEN))		/* soft linebreak */
		{
			if (*v == '\\')				/* if we have just written a '\' then  */
				v--;					/* treat it as soft linebreak and set  */
			else						/* v back so that it is written again. */
				saveputc(sfile, '\\');	/* else insert soft linebreak */
			saveputc(sfile, '\n');
		}

		v++;
	}

	return(TRUE);
}

/**************************************************************************
*** Function:	WriteProperty
***				writes ID & value into the given file
***				does necessary conversions
*** Parameters: info ... game-tree info
***				prop ... pointer to property
***				sfile .. file handle
*** Returns:	TRUE or FALSE
**************************************************************************/

int WriteProperty(struct TreeInfo *info, struct Property *prop, FILE *sfile)
{
	static int gi_written = FALSE;

	struct PropValue *v;
	char *p;
	int do_tt;

	if(prop->flags & TYPE_GINFO)
	{
		if(!gi_written)
		{
			saveputc(sfile, '\n');
			saveputc(sfile, '\n');
		}
		gi_written = TRUE;
	}
	else
		gi_written = FALSE;


	p = prop->idstr;			/* write property ID */
	while(*p)
	{
		saveputc(sfile, *p);
		p++;
	}

	do_tt = (info->GM == 1 && option_pass_tt &&
			(info->bwidth <= 19) && (info->bheight <= 19) &&
			(prop->id == TKN_B || prop->id == TKN_W));

	v = prop->value;			/* write property value(s) */

	while(v)
	{
		saveputc(sfile, '[');

		if(do_tt && !strlen(v->value))
			WritePropValue("tt", FALSE, prop->flags, sfile);
		else
		{
			WritePropValue(v->value, FALSE, prop->flags, sfile);
			WritePropValue(v->value2, TRUE, prop->flags, sfile);
		}
		saveputc(sfile, ']');

		CheckLineLen(sfile);
		v = v->next;
	}

	if(prop->flags & TYPE_GINFO)
	{
		saveputc(sfile, '\n');
		if(prop->next)
		{
			if(!(prop->next->flags & TYPE_GINFO))
				saveputc(sfile, '\n');
		}
		else
			saveputc(sfile, '\n');
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	WriteNode
***				writes the node char ';' calls WriteProperty for all props
*** Parameters: n ... node to write
***				sfile ... file handle
*** Returns:	TRUE or FALSE
**************************************************************************/

int WriteNode(struct TreeInfo *info, struct Node *n, FILE *sfile)
{
	struct Property *p;
	saveputc(sfile, ';')

	p = n->prop;
	while(p)
	{
		if((sgf_token[p->id].flags & PVT_CPLIST) && !option_expandcpl &&
		   (info->GM == 1))
			CompressPointList(p);

		if(!WriteProperty(info, p, sfile))
			return(FALSE);

		p = p->next;
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	SetRootProps
***				Sets new root properties for the game tree
*** Parameters: info 	 ... TreeInfo
***				r		 ... root node of tree
*** Returns:	-
**************************************************************************/

void SetRootProps(struct TreeInfo *info, struct Node *r)
{
	if(r->parent)	/* isn't REAL root node */
		return;

	New_PropValue(r, TKN_FF, "4", NULL, TRUE);
	New_PropValue(r, TKN_AP, "SGFC", "1.16", TRUE);

	if(info->GM == 1)			/* may be default value without property */
		New_PropValue(r, TKN_GM, "1", NULL, TRUE);

								/* may be default value without property */
	if(info->bwidth == 19 && info->bheight == 19)
		New_PropValue(r, TKN_SZ, "19", NULL, TRUE);
}


/**************************************************************************
*** Function:	WriteTree
***				recursive function which writes a complete SGF tree
*** Parameters: info 	 ... TreeInfo
***				n		 ... root node of tree
***				sfile	 ... file handle
***				newlines ... number of nl to print
*** Returns:	TRUE: success / FALSE error
**************************************************************************/

int WriteTree(struct TreeInfo *info, struct Node *n, FILE *sfile, int newlines)
{
	if(newlines)
		saveputc(sfile, '\n')

	SetRootProps(info, n);

	saveputc(sfile, '(')
	if(!WriteNode(info, n, sfile))
		return(FALSE);

	n = n->child;

	while(n)
	{
		if(n->sibling)
		{
			while(n)					/* write child + variations */
			{
				if(!WriteTree(info, n, sfile, 1))
					return(FALSE);
				n = n->sibling;
			}
		}
		else
		{
			if(!WriteNode(info, n, sfile))	/* write child */
				return(FALSE);
			n = n->child;
		}
	}

	saveputc(sfile, ')')

	if(newlines != 1)
		saveputc(sfile, '\n')

	return(TRUE);
}


/**************************************************************************
*** Function:	SaveSGF
***				writes the complete SGF tree to a file
*** Parameters: sgf ... pointer to sgfinfo structure
*** Returns:	-
**************************************************************************/

void SaveSGF(struct SGFInfo *sgf)
{
	FILE *sfile;
	struct Node *n;
	struct TreeInfo *info;
	char *c, name[500];
	int nl = 0, i = 1;

	sgfc = sgf;					/* set curent SGFInfo context */

	if(strlen(sgf->name) > 480)
		PrintError(FE_DEST_NAME_TOO_LONG);

	if(option_split_file)
		sprintf(name, "%s_%03d.sgf", sgf->name, i);
	else
		strcpy(name, sgf->name);

	sfile = fopen(name, "wb");
	if(!sfile)
		PrintError(FE_DEST_FILE_OPEN, name);

	if(option_keep_head)
	{
		*sgf->start = '\n';

		for(c = sgf->buffer; c <= sgf->start; c++)
			if(fputc((*c), sfile) == EOF)
			{
				fclose(sfile);
				PrintError(FE_DEST_FILE_WRITE, name);
			}
	}

	save_linelen = 0;

	n = sgf->root;
	info = sgf->tree;

	while(n)
	{
		if(!WriteTree(info, n, sfile, nl))
		{
			fclose(sfile);
			PrintError(FE_DEST_FILE_WRITE, name);
		}

		nl = 2;
		n = n->sibling;
		info = info->next;

		if(option_split_file && n)
		{
			fclose(sfile);
			i++;
			sprintf(name, "%s_%03d.sgf", sgf->name, i);

			if(!(sfile = fopen(name, "wb")))
				PrintError(FE_DEST_FILE_OPEN, name);
		}
	}

	fclose(sfile);
}
