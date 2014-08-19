/**************************************************************************
*** Project: SGF Syntax Checker & Converter
***	File:	 parse.c
***
*** Copyright (C) 1996-2004 by Arno Hollosi
*** (see 'main.c' for more copyright information)
***
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "all.h"
#include "protos.h"


/**************************************************************************
*** Function:	Parse_Number
***				Checks for illegal chars and for LONG INT range
*** Parameters: value ... pointer to value string
*** Returns:	-1/0/1 for corrected error / error / OK
**************************************************************************/

int Parse_Number(char *value, U_SHORT dummy)
{
	long i;
	int ret = 1;
	char *d;

	if(Kill_Chars(value, C_NOTinSET, "+-0123456789"))
		ret = -1;

	if(strlen(value))					/* empty? */
	{
		errno = 0;
		i = strtol(value, &d, 10);

		if(*d)							/* if *d: d >= value + 1 */
		{
			*d = 0;
			if(strlen(value))	ret = -1;
			else				ret = 0;
		}

		if(errno == ERANGE)				/* ot of range? */
		{
			sprintf(value, "%ld", i);	/* set to max range value */
			ret = -1;
		}
	}
	else
		ret = 0;

	return(ret);
}


/**************************************************************************
*** Function:	Parse_Text
***				Transforms any kind of linebreaks to '\n' (or ' ')
***				and all WS to space. Cuts off trailing WS.
*** Parameters: value	... pointer to property value
***				flags	... PVT_SIMPLE (SimpleText type)
***							PVT_COMPOSE (compose type)
*** Returns:	length of converted string (0 for empty string)
**************************************************************************/

int Parse_Text(char *value, U_SHORT flags)
{
	char *s, *d, old = 0, nl;

	do			/* loop, because trailing spaces may be protected by '\' */
	{
		d = value + strlen(value) - 1;		/* remove trailing spaces */
		while(d >= value && isspace(*d))
			*d-- = 0;

		if(!strlen(value))
			return(0);

		if(*d == '\\' && *(d-1) != '\\')	/* remove trailing '\' */
			*d = 0;
	}while(!*d);

	if(flags & PVT_SIMPLE)	nl = ' ';
	else					nl = '\n';

	s = d = value;

	while(*s)							/* transform linebreaks to '\n' */
	{									/*			and all WS to space */
		if(*s == '\r' || *s == '\n')	/* linebreak char? */
		{
			if(old && old != *s)		/* different from preceeding char? */
				old = 0;				/* -> no real linebreak */
			else
			{
				old = *s;
				*d++ = nl;				/* insert lienbreak */
			}
		}
		else							/* other chars than \r,\n */
		{
			old = 0;
			if(isspace(*s))	*d++ = ' ';	/* transform all WS to space */
			else			*d++ = *s;
		}
		s++;
	}
	*d = 0;


	s = d = value;						/* remove unnecessary '\' */
										/* apply linebreak style */
	while(*s)
	{
		if(*s == '\\')
		{
			switch(*(s+1))
			{
				case '\\':
				case ']':	*d++ = *s++;
							break;
				case ':':	if(flags & (PVT_COMPOSE|PVT_WEAKCOMPOSE))
									*d++ = *s++;
							else	s++;
							break;
				case '\n':	s += 2;		/* '\' + '\n' is removed */
							break;
				default:	*d++ = *(s+1);
							s += 2;
							break;
			}
			continue;
		}

		if(*s == '\n')
			switch(option_linebreaks)
			{
				case 1:	/* every line break encountered */
						*d++ = *s++;
						break;
				case 2:	/* MGT style */
						if((s != value) && (*(s-1) == ' '))
						{
							*d++ = ' ';
							s++;
						}
						else
							*d++ = *s++;
						break;
				case 3: /* two linebreaks in a row */
						if(*(s+1) == '\n')
						{
							*d++ = *s;
							s += 2;
						}
						else
						{
							*d++ = ' ';
							s++;
						}
						break;
				case 4: /* paragraph style (ISHI format, MFGO) */
						if(*(s+1) == '\n')
						{
							*d++ = *s++;
							*d++ = *s++;
						}
						else
						{
							*d++ = ' ';
							s++;
						}
						break;
						
			}
		else
			*d++ = *s++;
	}
	*d = 0;

	return((int)strlen(value));
}


/**************************************************************************
*** Function:	Parse_Move
***				Kills illegal chars, checks position (board size)
***				transforms FF[3] PASS 'tt' into FF[4] PASS ''
*** Parameters: value ... pointer to value string
***				pos   ... TRUE: check as position (handling of 'tt')
*** Returns:	-1/0/1	for corrected error / error / OK
**************************************************************************/

int Parse_Move(char *value, U_SHORT pos)
{
	int ret = 1, c;

	if(sgfc->info->GM != 1)			/* game != GO ? */
		return(1);

	if(Kill_Chars(value, C_ISSPACE|C_ISPUNCT|C_IS8BIT, NULL))
		ret = -1;

	if(!strlen(value))				/* empty value? */
    {
		if(!pos)
		{
			if(sgfc->info->FF >= 4)
				return(ret);
			else					/* new pass '[]' in old FF[1-3] */
				return(-101);		/* possible cause: missing FF   */
		}
		else
			return(0);
    }

	if(strlen(value) != 2)			/* value too long? */
	{
		*(value+2) = 0;
		ret = -1;
	}

	if(!pos && !strcmp(value, "tt"))
	{
		if(sgfc->info->bwidth <= 19 && sgfc->info->bheight <= 19)
		{
			*value = 0;					/* new pass */
			return(ret);
		}
	}

	c = DecodePosChar(*value);
	if(!c)								/* check range */
		return(0);
	if(c > sgfc->info->bwidth)
		return(0);

	c = DecodePosChar(*(value+1));
	if(!c)
		return(0);
	if(c > sgfc->info->bheight)
		return(0);

	return(ret);
}


/**************************************************************************
*** Function:	Parse_Float
***				Checks for correct float format / tries to correct
*** Parameters: value ... pointer to value string
***				flags ... TYPE_GINFO => disallow '-' and '+' characters
*** Returns:	-1/0/1/2 for corrected error / error / OK / corrected
**************************************************************************/

int Parse_Float(char *value, U_SHORT flags)
{
	int ret = 1, where = 0;
	/* where (bits): 0-minus / 1-int / 2-fraction / 3-'.' / 4-plus */
	U_LONG i;
	char *s, *d;
	char *allowed = (flags & TYPE_GINFO) ? "0123456789.," : "0123456789+-.,";

	if(Kill_Chars(value, C_NOTinSET, allowed))
		ret = -1;

	s = d = value;
	while(*s)
	{
		switch(*s)
		{
			case '+':	if(where)	ret = -1;		/* '+' gets swallowed */
						else	{
									where = 16;
									ret = 2;
								}
						break;
			case '-':	if(where)	ret = -1;
						else
						{
							*d++ = *s;
							where = 1;
						}
						break;
			case ',':	ret = -1;
						*s = '.';
			case '.':	if(where & 8)	ret = -1;
						else
						{
							*d++ = *s;
							where |= 8;
						}
						break;
			default:	if(where & 8)	where |= 4;
						else			where |= 2;
						*d++ = *s;
						break;
		}
		s++;
	}

	*d = 0;

	if(!strlen(value) || !(where & 6))	/* empty || no digits? */
		ret = 0;
	else
	{
		if((where & 8) && !(where & 2))		/* missing '0' in front of '.' */
		{
			ret = -1;
			i = strlen(value);
			d = value + i;
			s = d - 1;

			*(d+1) = 0;
			for(; i; i--)
				*d-- = *s--;

			if(where & 1)	*(value+1) = '0';	/* minus? */
			else			*value = '0';
		}

		if((where & 8) && (where & 4))	/* check for unnecssary '0' */
		{
			int mod = 0;	/* if correction occured */
			d = value + strlen(value) - 1;

			while(*d == '0')
			{
				*d-- = 0;
				mod = 1;
			}

			if(*d == '.')
			{
				*d = 0;
				mod = 1;
			}

			if(ret == 1 && mod == 1)
				ret = 2;
		}

		if((where & 8) && !(where & 4))		/* '.' without digits following */
		{
			ret = -1;
			*(value + strlen(value) - 1) = 0;
		}
	}

	return(ret);
}


/**************************************************************************
*** Function:	Parse_Color
***				Checks & corrects color value
*** Parameters: value	... pointer to value string
*** Returns:	-1/0/1	for corrected error / error / OK
**************************************************************************/

int Parse_Color(char *value, U_SHORT dummy)
{
	int ret = 1;

	if(Kill_Chars(value, C_NOTinSET, "BbWw"))
		ret = -1;

	switch(*value)
	{
		case 'B':
		case 'W':	break;
		case 'b':	*value = 'B';	/* uppercase required */
					ret = -1;
					break;
		case 'w':	*value = 'W';
					ret = -1;
					break;
		default:	return(0);		/* unknown char -> error */
	}

	if(strlen(value) != 1)			/* string too long? */
	{
		*(value+1) = 0;
		ret = -1;
	}

	return(ret);
}


/**************************************************************************
*** Function:	Parse_Triple
***				Checks & corrects triple value
*** Parameters: value	... pointer to value string
*** Returns:	-1/0/1	for corrected error / error / OK
**************************************************************************/

int Parse_Triple(char *value, U_SHORT dummy)
{
	int ret = 1;

	if(Kill_Chars(value, C_NOTinSET, "12"))
		ret = -1;

	if(!strlen(value))
	{
		*value = '1';
		*(value+1) = 0;
		ret = -1;
	}

	if(*value != '1' && *value != '2')
		return(0);

	if(strlen(value) != 1)		/* string too long? */
	{
		*(value+1) = 0;
		ret = -1;
	}

	return(ret);
}


/**************************************************************************
*** Function:	Check_Value
***				Checks value type & prints error messages
*** Parameters: p		... pointer to property containing the value
***				v		... pointer to property value
***				flags	... flags to be passed on to parse function
***				Parse_Value ... function used for parsing
***				err_corr	... error code if value got corrected
***				err_del		... error code if value has to be deleted
*** Returns:	TRUE for success / FALSE if value has to be deleted
**************************************************************************/

int Check_Value(struct Property *p, struct PropValue *v, U_SHORT flags,
				int (*Parse_Value)(char *, U_SHORT))
{
	switch((*Parse_Value)(v->value, flags))
	{
		case -101:	/* special case for Parse_Move */
					PrintError(E_FF4_PASS_IN_OLD_FF, v->buffer);
					break;
		case -1:	PrintError(E_BAD_VALUE_CORRECTED, v->buffer, p->idstr, v->value);
					break;
		case 0:		PrintError(E_BAD_VALUE_DELETED, v->buffer, p->idstr);
					return(FALSE);
		case 1:
		case 2:		break;
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Check_Text
***				Checks type value & prints error messages
*** Parameters: p ... pointer to property containing the value
***				v ... pointer to property value
*** Returns:	TRUE for success / FALSE if value has to be deleted
**************************************************************************/

int Check_Text(struct Property *p, struct PropValue *v)
{
	if(!Parse_Text(v->value, p->flags))
	{
		if(p->flags & PVT_DEL_EMPTY)
		{
			PrintError(W_EMPTY_VALUE_DELETED, v->buffer, p->idstr, "found");
			return(FALSE);
		}
	}
	return(TRUE);
}


/**************************************************************************
*** Function:	Check_Pos
***				Checks position type & expand compressed point lists
*** Parameters: p ... pointer to property containing the value
***				v ... pointer to property value
*** Returns:	TRUE for success / FALSE if value has to be deleted
**************************************************************************/

int Check_Pos(struct Property *p, struct PropValue *v)
{
	if(!Check_Value(p, v, TRUE, Parse_Move))
		return(FALSE);

	if(v->value2)	/* compressed point list */
	{
		if(sgfc->info->FF < 4)
			PrintError(E_VERSION_CONFLICT, v->buffer, sgfc->info->FF);

		switch((Parse_Move)(v->value2, TRUE))
		{
			case -1:	PrintError(E_BAD_VALUE_CORRECTED, v->buffer, p->idstr, v->value2);
						break;
			case 0:		PrintError(E_BAD_VALUE_DELETED, v->buffer, p->idstr);
						return(FALSE);
			case 1:		break;
		}

		if(sgfc->info->GM == 1)
			return((!ExpandPointList(p, v, TRUE)));
		else
			return(TRUE);
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Check_Label
***				Checks label type value & prints error messages
*** Parameters: p ... pointer to property containing the value
***				v ... pointer to property value
*** Returns:	TRUE for success / FALSE if value has to be deleted
**************************************************************************/

int Check_Label(struct Property *p, struct PropValue *v)
{
	int error = 0;

	switch(Parse_Move(v->value, TRUE))
	{
		case 0:		PrintError(E_BAD_VALUE_DELETED, v->buffer, p->idstr);
					return(FALSE);
		case -1:	error = 1;
		case 1:		switch(Parse_Text(v->value2, p->flags))
					{
						case 0:	PrintError(E_BAD_VALUE_DELETED, v->buffer, p->idstr);
								return(FALSE);
						case 1:	if(strlen(v->value2) > 4 && sgfc->info->FF < 4)
								{
									error = 1;
									*(v->value2+4) = 0;
								}
								break;
					}
					if(error)
						PrintError(E_BAD_COMPOSE_CORRECTED, v->buffer, p->idstr, v->value, v->value2);
					break;
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Check_AR_LN
***				Checks arrow/line type values & prints error messages
*** Parameters: p ... pointer to property containing the value
***				v ... pointer to property value
*** Returns:	TRUE for success / FALSE if value has to be deleted
**************************************************************************/

int Check_AR_LN(struct Property *p, struct PropValue *v)
{
	int error = 0;

	switch(Parse_Move(v->value, TRUE))
	{
		case 0:		PrintError(E_BAD_VALUE_DELETED, v->buffer, p->idstr);
					return(FALSE);
		case -1:	error = 1;
		case 1:		switch(Parse_Move(v->value2, TRUE))
					{
						case 0:	PrintError(E_BAD_VALUE_DELETED, v->buffer, p->idstr);
								return(FALSE);
						case -1:
								error = 1;
						case 1:	if(!strcmp(v->value, v->value2))
								{
									PrintError(E_BAD_VALUE_DELETED, v->buffer, p->idstr);
									return(FALSE);
								}
								break;
					}
					if(error)
						PrintError(E_BAD_COMPOSE_CORRECTED, v->buffer, p->idstr, v->value, v->value2);
					break;
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Check_Figure
***				Check FG property values
*** Parameters: p ... pointer to property containing the value
***				v ... pointer to property value
*** Returns:	TRUE for success / FALSE if value has to be deleted
**************************************************************************/

int Check_Figure(struct Property *p, struct PropValue *v)
{
	if(!v->value2)	/* no compose type */
	{
		if(strlen(v->value))
		{
			if(!Parse_Text(v->value, PVT_SIMPLE|PVT_COMPOSE))
				PrintError(E_BAD_VALUE_CORRECTED, v->buffer, "FG", "");
			else
			{
				v->value2 = v->value;
				SaveMalloc(char *, v->value, 4, "new FG number value");
				strcpy(v->value, "0");
				PrintError(E_BAD_COMPOSE_CORRECTED, v->buffer, "FG", v->value, v->value2);
			}
		}
	}
	else
	{
		Parse_Text(v->value2, PVT_SIMPLE|PVT_COMPOSE);
		switch(Parse_Number(v->value, 0))
		{
			case 0:	strcpy(v->value, "0");
			case -1:
					PrintError(E_BAD_COMPOSE_CORRECTED, v->buffer, "FG", v->value, v->value2);
			case 1:	break;
		}
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Check_PropValues
***				Checks values for syntax errors (calls Check_XXX functions)
*** Parameters: p ... pointer to property
*** Returns:	-
**************************************************************************/

void Check_PropValues(struct Property *p)
{
	struct PropValue *v;

	v = p->value;
	while(v)					/* value loop */
	{
		if(!strlen(v->value) && !(p->flags & PVT_CHECK_EMPTY))
		{													/* empty value? */
			if(sgf_token[p->id].flags & PVT_DEL_EMPTY)
			{
				PrintError(W_EMPTY_VALUE_DELETED, v->buffer, p->idstr, "found");
				v = Del_PropValue(p, v);
			}
			else if(!(p->flags & PVT_EMPTY))
			{
				PrintError(E_EMPTY_VALUE_DELETED, v->buffer, p->idstr, "not allowed");
				v = Del_PropValue(p, v);
			}
			else
				v = v->next;
		}
		else
			if(sgf_token[p->id].CheckValue)
			{
				if((*sgf_token[p->id].CheckValue)(p, v))
					v = v->next;
				else
					v = Del_PropValue(p, v);
			}
			else
				v = v->next;
	}
}


/**************************************************************************
*** Function:	CheckID_Lowercase
***				Checks if the property ID contains lowercase letters (FF[4])
*** Parameters: p ... pointer to property ID (in source buffer)
*** Returns:	-
**************************************************************************/

void CheckID_Lowercase(char *p)
{
	while(isalpha(*p))
	{
		if(islower(*p))
		{
			PrintError(E_LC_IN_PROPID, p);
			break;		/* print error only once */
		}
		p++;
	}
}


/**************************************************************************
*** Function:	Check_Properties
***				Performs various checks on properties ID's
***				and calls Check_PropValues
*** Parameters: p   ... pointer to node containing the properties
***				st	... pointer to board status
*** Returns:	-
**************************************************************************/

void Check_Properties(struct Node *n, struct BoardStatus *st)
{
	struct Property *p, *hlp;

	p = n->prop;
	while(p)						/* property loop */
	{
		if((!(sgf_token[p->id].ff & (1 << (sgfc->info->FF - 1)))) &&
			 (p->id != TKN_KI))
		{
			if(sgf_token[p->id].data & ST_OBSOLETE)
				PrintError(WS_PROPERTY_NOT_IN_FF, p->buffer, p->idstr, sgfc->info->FF, "converted");
			else
				PrintError(WS_PROPERTY_NOT_IN_FF, p->buffer, p->idstr, sgfc->info->FF, "parsing done anyway");
		}

		if(!option_keep_obsolete_props && !(sgf_token[p->id].ff & FF4) &&
		   !(sgf_token[p->id].data & ST_OBSOLETE))
		{
			PrintError(W_PROPERTY_DELETED, p->buffer, "obsolete ", p->idstr);
			p = Del_Property(n, p);
			continue;
		}


		if(sgfc->info->FF >= 4)
			CheckID_Lowercase(p->buffer);

		Check_PropValues(p);

		if(!p->value)				/* all values of property deleted? */
			p = Del_Property(n, p);	/* -> del property */
		else
		{
			hlp = p->next;

			if(sgf_token[p->id].Execute_Prop)
			{
				if(!(*sgf_token[p->id].Execute_Prop)(n, p, st) || !p->value)
					Del_Property(n, p);
			}

			p = hlp;
		}
	}
}
