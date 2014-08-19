/**************************************************************************
*** Project: SGF Syntax Checker & Converter
***	File:	 util.c
***
*** Copyright (C) 1996-2004 by Arno Hollosi
*** (see 'main.c' for more copyright information)
***
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "all.h"
#include "protos.h"

/* #define NO_STRERROR */		/* for dumb pseudo-ANSI compilers */

int error_count = 0;
int critical_count = 0;
int warning_count = 0;
int ignored_count = 0;

char error_enabled[MAX_ERROR_NUM];

const char *error_mesg[] =
{
	"unknown command '%s' (-h for help)\n",
	"unknown command line option '%c' (-h for help)\n",
	"could not open source file '%s' - ",
	"could not read source file '%s' - ",
	"could not allocate %s (not enough memory)\n",
/* 5 */
	"possible SGF data found in front of game-tree (before '(;')\n",
	"could not find start mark '(;' - no SGF data found\n",
	"illegal char(s) found: ",
	"variation nesting incomplete (missing ')')\n",
	"unexpected end of file\n",
/* 10 */
	"property identifier too long - more than 100 chars (deleted)\n",
	"empty variation found (ignored)\n",
	"property <%s> may have only ONE value (other values deleted)\n",
	"illegal <%s> value deleted: ",
	"illegal <%s> value corrected; new value: [%s], old value: ",
/* 15 */
	"lowercase char not allowed in property identifier\n",
	"empty <%s> value %s (deleted)\n",
	"illegal root property <%s> found (assuming %s)\n",
	"game stored in tree %d is not Go. Can not check move & position type"
									" -> errors will not get corrected!\n",
	"property <%s> without any values found (ignored)\n",
/* 20 */
	"illegal variation start found (ignored)\n",
	"$00 byte deleted - binary file?\n",
	"property <%s> expects compose type value (value deleted): ",
	"move in root node found (splitted node into two)\n",
	"illegal <%s> value corrected; new value: [%s:%s], old value: ",
/* 25 */
	"could not open destination file '%s' - ",
	"could not write destination file '%s' - ",
	"property <%s> already exists (%s)\n",
	"%sproperty <%s> deleted\n",
	"setup and move properties mixed within a node (%s)\n",
/* 30 */
	"property identifier consists of more than 2 uppercase letters: <%s>\n",
	"root property <%s> outside root node (deleted)\n",
	"gameinfo property <%s> has illegal format %s - value: ",
	"file not saved (because of critical errors)\n",
	"unknown property <%s> %s\n",
/* 35 */
	"missing semicolon at start of game-tree (detection might be wrong [use -b2 in that case])\n",
	"black and white move within a node (split into two nodes)\n",
	"%s <%s> position not unique ([partially] deleted) - value(s): ",
	"AddStone <%s> has no effect ([partially] deleted) - value(s): ",
	"property <%s> is not defined in FF[%d] (%s)\n",
/* 40 */
	"annotation property <%s> contradicts previous property (deleted)\n",
	"combination of <%s> found (converted to <%s>)\n",
	"move annotation <%s> without a move in node (deleted)\n",
	"game info entry <%s> outside game-info node (line:%d col:%d) (deleted)\n",
	"different %s stored in one file (may cause troubles with some applications)\n",
/* 45 */
	"unknown file format (only able to handle files up to FF[4])\n",
	"square board size in rectangular definition (corrected)\n",
	"no source file specified (-h for help)\n",
	"bad command line option parameter '%s' (-h for help)\n",
	"board size too big (corrected to %dx%d)\n",
/* 50 */
	"used feature is not defined in FF[%d] (parsing done anyway)\n",
	"<VW> property: %s (%s)\n",
	"destination file name too long (max. length: 480 chars)\n",
	"values without property id found (deleted)\n",
	"empty node deleted\n",
/* 55 */
	"possible incorrect variation level can not be corrected\n",
	"variation level corrected\n",
	"forbidden move found (played on a point occupied by another stone)\n",
	"obsolete <KI> property found: %s\n",
	"file contains more than one game tree\n",
/* 60 */
	"value of HA property differs from number of setup stones\n",
	"setup stones in main line found (outside root node)\n",
	"two successive moves have the same color\n",
	"can not reorder variations: too many variations\n",
	"FF4 style pass value '[]' in older format found (corrected)\n",
/*65 */
	"node outside variation found. Missing '(' assumed.\n",
	"illegal chars after variation start '(' found. Missing ';' assumed.\n",
	"unknown command line option '%s' (-h for help)\n"
};


/**************************************************************************
*** Function:	SearchPos
***				Calculate the position in rows/columns within a buffer
*** Parameters: c ... current position
***				s ... pointer to start of buffer
***				x ... result: #columns
***				y ... result: #rows
*** Returns:	- (result stored in x,y)
**************************************************************************/

void SearchPos(char *c, char *s, int *x, int *y)
{
	char newline = 0;

	*y = 1;
	*x = 1;

	while(s != c)
	{
		if(*s == '\r' || *s == '\n')		/* linebreak char? */
		{
			if(newline && newline != *s)	/* different from preceeding char? */
				newline = 0;				/* ->yes: no real linebreak */
			else
			{
				(*y)++;						/* lines += 1 */
				*x = 1;						/* reset column */
				newline = *s;
			}
		}
		else
		{
			newline = 0;					/* no linebreak char */
			(*x)++;							/* column++ */
		}

		s++;
	}
}


/**************************************************************************
*** Function:	PrintError
***				Prints an error message to E_OUTPUT (see #define)
***				Performs several calculations if required
*** Parameters: type ... error code
***				additional parameters as needed:
***				[position] [accumulate] [acc_size] 
*** Returns:	TRUE: error printed
***				FALSE: error disabled
***				(may quit program, if error is fatal!)
**************************************************************************/

int PrintError(U_LONG type, ...)
{
	int x, y, print_c = 0;
	char *pos = NULL, *pos2;
	static char *illegal, *last_pos;
	static int ill_count = 0;
	static U_LONG ill_type = E_NO_ERROR, last_type;
	va_list arglist;

	va_start(arglist, type);

	if(type & E_ERROR4)
	{
		if(sgfc->info->FF >= 4)		type |= E_ERROR;
		else						type |= E_WARNING;
	}
	if(type & E_WARNING_STRICT)
	{
		if(option_strict_checking)	type |= E_ERROR;
		else						type |= E_WARNING;
	}

	if((!error_enabled[(type & M_ERROR_NUM)-1] && !(type & E_FATAL_ERROR)
		 && type != E_NO_ERROR) || (!option_warnings && (type & E_WARNING)))
	{								/* error message enabled? */
		ignored_count++;
		return(FALSE);
	}

	if(type & E_SEARCHPOS)			/* get pointer to position if required */
	{
		pos = va_arg(arglist, char *);

		if(pos == last_pos && type == last_type && type & E_DEL_DOUBLE)
		/* eliminate doubel messages generated by compressed point lists */
			return(FALSE);
		else
		{
			last_pos = pos;
			last_type = type;
		}
	}
	else
		last_type = E_NO_ERROR;


	if((type & E_ACCUMULATE))			/* accumulate error messages? */
	{
		if(va_arg(arglist, int))		/* TRUE: accumulate */
		{
			if(ill_count)				/* any errors already accumulated? */
			{
				if((illegal + ill_count != pos) ||				/* succeed? */
				  ((ill_type & M_ERROR_NUM)!=(type & M_ERROR_NUM)))
				{
					PrintError(ill_type, illegal, FALSE);	/* no -> flush */
					illegal = pos;							/* set new */
					ill_type = type;
				}
			}
			else						/* first error */
			{
				ill_type = type;		/* set data */
				illegal = pos;
			}

			if(type & E_MULTIPLE)					/* more than one char? */
				ill_count += va_arg(arglist, int);	/* get size */
			else
				ill_count++;						/* else +1 */

			va_end(arglist);
			return(TRUE);
		}								/* FALSE: print it */
		else
			print_c = 1;
	}
	else								/* not an ACCUMULATE type */
		if(ill_count)					/* any errors waiting? */
			PrintError(ill_type, illegal, FALSE);	/* flush buffer */
													/* and continue ! */

	if(type & E_SEARCHPOS)				/* print position if required */
	{
		SearchPos(pos, sgfc->buffer, &x, &y);
		fprintf(E_OUTPUT, "Line:%d Col:%d - ", y, x);
	}

	switch(type & M_ERROR_TYPE)
	{
		case E_FATAL_ERROR:	fprintf(E_OUTPUT, "Fatal error %d", (int)(type & M_ERROR_NUM));
							break;
		case E_ERROR:		fprintf(E_OUTPUT, "Error %d", (int)(type & M_ERROR_NUM));
							error_count++;
							break;
		case E_WARNING:		fprintf(E_OUTPUT, "Warning %d", (int)(type & M_ERROR_NUM));
							warning_count++;
							break;
	}

	if(type & E_CRITICAL)
	{
		critical_count++;
		fprintf(E_OUTPUT, " (critical): ");
	}
	else
		if(type != E_NO_ERROR)
			fprintf(E_OUTPUT, ": ");

	if(type != E_NO_ERROR)
	{
		/* print error */
		vfprintf(E_OUTPUT, error_mesg[(type & M_ERROR_NUM)-1], arglist);

		if(print_c)					/* print accumulated string? */
		{
			putc('"', E_OUTPUT);	/* string is not terminated with '\0'!! */
			for(; ill_count; ill_count--)
			{
				putc(*illegal, E_OUTPUT);
				illegal++;
			}
			fputs("\"\n", E_OUTPUT);
		}

		if(type & E_ERRNO)			/* print DOS error message? */
		{
#ifdef NO_STRERROR					/* strerror() may not be defined! */
			fprintf(E_OUTPUT, "error code: %d\n", errno);
#else
			char *err;

			err = strerror(errno);
			if(err)
				fprintf(E_OUTPUT, "%s\n", err);
			else
				fprintf(E_OUTPUT, "error code: %d\n", errno);
#endif
			errno = 0;
		}

		if(type & E_VALUE)			/* print a property value ("[value]\n") */
		{
			pos2 = SkipText(pos, NULL, ']', 0);
			while(pos <= pos2)
			{
				putc(*pos, E_OUTPUT);
				pos++;
			}
			putc('\n', E_OUTPUT);
		}
	}

	va_end(arglist);

	if((type & M_ERROR_TYPE) == E_FATAL_ERROR)	/* fatal one? */
	{
		FreeSGFInfo(sgfc);
		exit(20);							/* say dada */
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	EncodePosChar // DecodePosChar
***				Encodes or decodes a position character
*** Parameters: c ... number or character
*** Returns:	FALSE for error, a-zA-Z or 1-52
**************************************************************************/

char EncodePosChar(int c)
{
	if(c <= 0 || c > 52)
		return(FALSE);
	if(c <= 26)
		return((char)('a'+c-1));
	else
		return((char)('A'+c-27));
}

int DecodePosChar(char c)
{
	if(islower(c))
		return(c-'a'+1);
	else
		if(isupper(c))
			return(c-'A'+27);
		else
			return(FALSE);
}


/**************************************************************************
*** Function:	FreeSGFInfo
***				Frees all memory and other resources of a SGFInfo
***				Doesn't free the SGFInfo structure itself!
*** Parameters: sgf ... pointer to SGFInfo structure
*** Returns:	-
**************************************************************************/

void FreeSGFInfo(struct SGFInfo *sgf)
{
#ifndef DIRTY_FREE
	struct Node *n, *m;
	struct Property *p;
	struct TreeInfo *t, *hlp;
#endif

	if(!sgf)							/* check just to be sure */
		return;

#ifndef DIRTY_FREE

	t = sgf->tree;						/* free TreeInfo's */
	while(t)
	{
		hlp = t->next;
		free(t);
		t = hlp;
	}

	n = sgf->first;						/* free Nodes */
	while(n)
	{
		m = n->next;
		p = n->prop;
		while(p)
			p = Del_Property(n, p);		/* and properties */
		free(n);
		n = m;
	}

	if(sgf->buffer)						/* free buffer & file */
		free(sgf->buffer);

#endif

	if(sgf->file)
		fclose(sgf->file);
}


/**************************************************************************
*** Function:	f_AddTail
***				Adds a node at the tail of a double-linked list
*** Parameters: h	... list header (first/last)
***				n	... node to add
*** Returns:	-
**************************************************************************/

void f_AddTail(struct ListHead *h, struct ListNode *n)
{
	n->next = NULL;

	if(h->last)
		h->last->next = n;

	n->prev = h->last;
	h->last = n;

	if(!h->first)
		h->first = n;
}

/**************************************************************************
*** Function:	f_Enqueue
***				Inserts a node into a double-linked list according to its
***				priority (same priorities: at the tail)
*** Parameters: h	... list header (first/last)
***				n	... node to insert
*** Returns:	-
**************************************************************************/

void f_Enqueue(struct ListHead *h, struct ListNode *n)
{
	struct ListNode *i;

	i = h->first;

	while(i)
	{
		if(i->priority < n->priority)
		{
			if(i->prev)
				i->prev->next = n;
			else
				h->first = n;

			n->prev = i->prev;
			n->next = i;
			i->prev = n;
			return;
		}

		i = i->next;
	}

	f_AddTail(h, n);		/* end of list reached -> add node at tail */
}

/**************************************************************************
*** Function:	f_Delete
***				Deletes (unlinks) an entry of a double-linked list
*** Parameters: h	... list header (first/last)
***				n	... node to delete
*** Returns:	-
**************************************************************************/

void f_Delete(struct ListHead *h, struct ListNode *n)
{
	if(n->prev)
		n->prev->next = n->next;
	else
		h->first = n->next;

	if(n->next)
		n->next->prev = n->prev;
	else
		h->last = n->prev;
}


/**************************************************************************
*** Function:	strnccmp
***				String compare, not case sensitive
*** Parameters: a, b ... strings to be compared
***				len  ... number of bytes to compare or 0
*** Returns:	TRUE=strings not equal, FALSE= equal
**************************************************************************/

int strnccmp(char *a, char *b, int len)
{
	if(!len)
		len = strlen(a) + 1;

	while(*a && *b && len)
	{
		if(toupper(*a) != toupper(*b))
			return(TRUE);
		a++;
		b++;
		len--;
	}

	if(len && (*a || *b))
		return(TRUE);

	return(FALSE);
}


/**************************************************************************
*** Function:	Kill_Chars
***				Deletes selected char-set out of a given string
*** Parameters: value ... string
***				kill ... type of chars to be removed
***						 C_ISSPACE, C_ISPUNCT, C_IS8BIT, C_ISALPHA
***				cset ... additional set of chars to be tested
***						 (kill has to be one of C_NOTinSET or C_inSET)
*** Returns:	number of chars removed from string
**************************************************************************/

U_LONG Kill_Chars(char *value, U_SHORT kill, char *cset)
{
	U_LONG faulty = 0, err = 0;
	char *c, *d;

	for(c = d = value; *c; c++)
	{
		if(((kill & C_ISSPACE) && isspace(*c)) ||
		   ((kill & C_ISPUNCT) && ispunct(*c)) ||
		   ((kill & C_IS8BIT) && *c & 0x80) ||
		   ((kill & C_ISALPHA) && isalpha(*c)))
			err = 1;
		else
			if(kill & C_NOTinSET)
			{
				if(!strchr(cset, *c))
					err = 1;
			}
			else
				if(kill & C_inSET)
					if(strchr(cset, *c))
						err = 1;
		if(err)
		{
			faulty++;
			err = 0;
		}
		else
			*d++ = *c;
	}
	*d = 0;			/* end mark */

	return(faulty);
}


/**************************************************************************
*** Function:	Test_Chars
***				Tests if string contains specified char-set
***				Spaces are ignored (can't be tested)
*** Parameters: value ... string
***				test ... test for chars (C_ISPUNCT, C_IS8BIT, C_ISALPHA)
***				cset ... char-set to be searched in value
***						 (test has to be one of C_inSET or C_NOTinSET)
*** Returns:	number of selected chars found in value
**************************************************************************/

U_LONG Test_Chars(char *value, U_SHORT test, char *cset)
{
	U_LONG faulty = 0;
	char *c;

	for(c = value; *c; c++)
	{
		if(isspace(*c))		/* WhiteSpace are ignored !! */
			continue;

		if(((test & C_ISPUNCT) && ispunct(*c)) ||
		   ((test & C_IS8BIT) && *c & 0x80) ||
		   ((test & C_ISALPHA) && isalpha(*c)))
			faulty++;
		else
			if(test & C_NOTinSET)
			{
				if(!strchr(cset, *c))
					faulty++;
			}
			else
				if(test & C_inSET)
					if(strchr(cset, *c))
						faulty++;
	}

	return(faulty);
}


/**************************************************************************
*** Function:	Find_Property
***				Searches property within a node
*** Parameters: n ... node to be searched
***				id ... property id
*** Returns:	pointer to property or NULL
**************************************************************************/

struct Property *Find_Property(struct Node *n, token id)
{
	struct Property *p;

	for(p = n->prop; p; p=p->next)
		if(p->id == id)
			break;

	return(p);
}


/**************************************************************************
*** Function:	Del_PropValue
***				Deletes a value of a property
*** Parameters: p ... property to which value belongs
***				v ... value to be deleted
*** Returns:	v->next
**************************************************************************/

struct PropValue *Del_PropValue(struct Property *p, struct PropValue *v)
{
	struct PropValue *next;

	if(v->value)		free(v->value);
	if(v->value2)		free(v->value2);

	next = v->next;

	Delete(&p->value, v);
	free(v);

	return(next);
}


/**************************************************************************
*** Function:	Del_Property
***				Deletes a property
*** Parameters: n ... node which contains property
***				p ... property to be deleted
*** Returns:	p->next
**************************************************************************/

struct Property *Del_Property(struct Node *n, struct Property *p)
{
	struct Property *next;
	struct PropValue *v;

	v = p->value;		/* del property values */
	while(v)
		v = Del_PropValue(p, v);

	next = p->next;		/* remove property from node */

	if(n)
		Delete(&n->prop, p);

	if(p->id == TKN_UNKNOWN)
		free(p->idstr);

	free(p);

	return(next);
}


/**************************************************************************
*** Function:	Del_Node
***				Deletes empty node if
***				- node has no siblings
***				- has siblings (or is root) but has max. one child
*** Parameters: n		... node that should be deleted
*** Returns:	n->next
**************************************************************************/

struct Node *Del_Node(struct Node *n, U_LONG error_code)
{
	struct Node *p, *h;
	struct Property *i;

	p = n->parent;

	if((p && (n->sibling || (p->child != n))) || !p)
	{
		if(n->child)
			if(n->child->sibling)
				return(n->next);
	}

	if(error_code != E_NO_ERROR)
		PrintError(error_code, n->buffer);

	if(n->prop)						/* delete properties */
	{
		i = n->prop;
		while(i)
			i = Del_Property(n, i);
	}

	if(!p)							/* n is a root node */
	{
		if(n->child)
		{
			n->child->sibling = n->sibling;
			n->child->parent = NULL;
		}

		if(sgfc->root == n)			/* n is first root */
		{
			if(n->child)	sgfc->root = n->child;
			else			sgfc->root = n->sibling;
		}
		else
		{							/* n is subsequent root */
			h = sgfc->root;
			while(h->sibling != n)
				h = h->sibling;

			if(n->child)	h->sibling = n->child;
			else			h->sibling = n->sibling;
		}
	}
	else							/* n is root of subtree */
	{
		if(n->sibling || (p->child != n))	/* empty node has siblings */
		{
			if(n->child)			/* child has no siblings! */
			{
				n->child->parent = p;
				n->child->sibling = n->sibling;
			}

			if(p->child == n)		/* n is first sibling */
			{
				if(n->child)		p->child = n->child;
				else				p->child = n->sibling;
			}
			else
			{						/* n is subsequent sibling */
				h = p->child;
				while(h->sibling != n)
					h = h->sibling;

				if(n->child)		h->sibling = n->child;
				else				h->sibling = n->sibling;
			}
		}
		else						/* empty node has no siblings */
		{							/* but child may have siblings */
			p->child = n->child;
			h = n->child;
			while(h)				/* set new parent */
			{
				h->parent = p;
				h = h->sibling;
			}
		}
	}

	h = n->next;
	Delete(&sgfc->first, n);
	free(n);

	return(h);
}


/**************************************************************************
*** Function:	New_PropValue
***				Adds (or sets) a new property value
***				(adds property to node if necessary)
*** Parameters: n ... node
***				id .. property id
***				value  ... first value
***				value2 ... second value (if compose type) or NULL
***				unique ... TRUE - old values get deleted
*** Returns:	pointer to property
**************************************************************************/

struct Property *New_PropValue(struct Node *n, token id,
							   char *value, char *value2, int unique)
{
	struct Property *p;
	struct PropValue *v;

	p = Find_Property(n, id);
	if(p)
	{
		if(unique)
			for(v = p->value; v; v = Del_PropValue(p, v));
	}
	else
		p = Add_Property(n, id, NULL, NULL);

	Add_PropValue(p, NULL, value, -1, value2, -1);

	return(p);
}
