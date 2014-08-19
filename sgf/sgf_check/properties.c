/**************************************************************************
*** Project: SGF Syntax Checker & Converter
***	File:	 properties.c
***
*** Copyright (C) 1996-2004 by Arno Hollosi
*** (see 'main.c' for more copyright information)
***
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "all.h"
#include "protos.h"


int Check_Move(struct Property *p, struct PropValue *v)
{
	return(Check_Value(p, v, FALSE, Parse_Move));
}

int Check_Number(struct Property *p, struct PropValue *v)
{
	return(Check_Value(p, v, p->flags, Parse_Number));
}

int Check_Float(struct Property *p, struct PropValue *v)
{
	return(Check_Value(p, v, p->flags, Parse_Float));
}

int Check_Color(struct Property *p, struct PropValue *v)
{
	return(Check_Value(p, v, p->flags, Parse_Color));
}

int Check_Triple(struct Property *p, struct PropValue *v)
{
	return(Check_Value(p, v, p->flags, Parse_Triple));
}

int Check_Empty(struct Property *p, struct PropValue *v)
{
	v->value[0] = 0;
	PrintError(E_BAD_VALUE_CORRECTED, v->buffer, p->idstr, v->value);
	return(TRUE);
}

#define CP_LIST (PVT_LIST|PVT_CPLIST|PVT_WEAKCOMPOSE)

#define FF1234	(FF12|FF3|FF4)
#define FF123	(FF12|FF3)
#define FF34	(FF3|FF4)


/*******************************************/
/* CHECK ALL.H BEFORE MODIFYING THIS ARRAY */
/*******************************************/

struct SGFToken sgf_token[] =
{
	/* TKN_UNKNOWN */
	{ "\0",	0,	FF1234,	Check_Text,	NULL,	PVT_LIST|PVT_EMPTY, 0 },

	{ "B",	60, FF1234,	Check_Move,	Do_Move,	TYPE_MOVE|PVT_CHECK_EMPTY, BLACK },
	{ "W",	60, FF1234,	Check_Move,	Do_Move,	TYPE_MOVE|PVT_CHECK_EMPTY, WHITE },

	{ "AB",	62, FF1234,	Check_Pos,	Do_Addstones,	TYPE_SETUP|CP_LIST|DOUBLE_MERGE, BLACK },
	{ "AW",	62, FF1234,	Check_Pos,	Do_Addstones,	TYPE_SETUP|CP_LIST|DOUBLE_MERGE, WHITE },
	{ "AE",	61, FF1234,	Check_Pos,	Do_Addstones,	TYPE_SETUP|CP_LIST|DOUBLE_MERGE, EMPTY },

	{ "N",	40, FF1234,	Check_Text,	NULL,	PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "C",	40, FF1234,	Check_Text,	NULL,	PVT_DEL_EMPTY|DOUBLE_MERGE|SPLIT_SAVE, 0 },

	{ "BL",	31, FF1234,	Check_Float,	NULL,	TYPE_MOVE, 0 },
	{ "WL",	31, FF1234,	Check_Float,	NULL,	TYPE_MOVE, 0 },
	{ "OB",	30, FF34,	Check_Number,	NULL,	TYPE_MOVE, 0 },
	{ "OW",	30, FF34,	Check_Number,	NULL,	TYPE_MOVE, 0 },

	{ "FF",	96, FF1234,	Check_Number,	Do_Root,	TYPE_ROOT, 0 },
	{ "GM",	95, FF1234,	Check_Number,	Do_Root,	TYPE_ROOT, 0 },
	{ "SZ",	94, FF1234,	Check_Number,	Do_Root,	TYPE_ROOT|PVT_WEAKCOMPOSE, 0 },
	{ "ST",	93, FF4,	Check_Number,	Do_Root,	TYPE_ROOT, 0 },
	{ "CA",	92, FF4,	Check_Text,		Do_Root,	TYPE_ROOT|PVT_SIMPLE, 0 },
	{ "AP",	91, FF4,	Check_Text,		Do_Root,	TYPE_ROOT|PVT_COMPOSE|PVT_SIMPLE, 0 },

	{ "GN",	89, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "GC",	67, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|DOUBLE_MERGE|SPLIT_SAVE, 0 },
	{ "PB",	86, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_SIMPLE|PVT_DEL_EMPTY, 0 },
	{ "PW",	82, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_SIMPLE|PVT_DEL_EMPTY, 0 },
	{ "BR",	85, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_SIMPLE|PVT_DEL_EMPTY, 0 },
	{ "WR",	81, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_SIMPLE|PVT_DEL_EMPTY, 0 },
	{ "PC",	76, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_SIMPLE|PVT_DEL_EMPTY, 0 },
	{ "DT",	77, FF1234,	Check_GameInfo,	Do_GInfo,	TYPE_GINFO|PVT_SIMPLE|PVT_DEL_EMPTY, 0 },
	{ "RE",	75, FF1234,	Check_GameInfo,	Do_GInfo,	TYPE_GINFO|PVT_SIMPLE|PVT_DEL_EMPTY, 0 },
	{ "KM",	78, FF1234,	Check_GameInfo,	Do_GInfo,	TYPE_GINFO, 0 },
	{ "KI",	78, FF12,	Check_Number,	Do_GInfo,	TYPE_GINFO, ST_OBSOLETE },
	{ "HA",	83, FF1234,	Check_Number,	Do_GInfo,	TYPE_GINFO, 0 },
	{ "TM",	74, FF1234,	Check_GameInfo,	Do_GInfo,	TYPE_GINFO, 0 },
	{ "EV",	88, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "RO",	87, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "SO",	70, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "US",	69, FF1234,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "BT",	84, FF34,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "WT",	80, FF34,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "RU",	72, FF34,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "AN",	68, FF34,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "OT",	73, FF4,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "ON",	66, FF34,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|DOUBLE_MERGE|SPLIT_SAVE, 0 },
	{ "CP",	65, FF34,	Check_Text,		Do_GInfo,	TYPE_GINFO|PVT_DEL_EMPTY|DOUBLE_MERGE|SPLIT_SAVE, 0 },

	{ "L",	49,	FF12,	Check_Pos,	Do_Letter,	PVT_LIST|DOUBLE_MERGE, ST_LABEL|ST_OBSOLETE },
	{ "LB",	50, FF34,	Check_Label,Do_Markup,	PVT_LIST|PVT_COMPOSE|DOUBLE_MERGE, ST_LABEL },
	{ "AR",	50, FF4,	Check_AR_LN,NULL,		PVT_LIST|PVT_COMPOSE|DOUBLE_MERGE, 0 },
	{ "LN",	50, FF4,	Check_AR_LN,NULL,		PVT_LIST|PVT_COMPOSE|DOUBLE_MERGE, 0 },
	{ "M",	49, FF12,	Check_Pos,	Do_Mark,	PVT_LIST|DOUBLE_MERGE, ST_MARKUP|ST_OBSOLETE },
	{ "MA",	50, FF34,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_MARKUP },
	{ "TR",	50, FF34,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_MARKUP },
	{ "CR",	50, FF34,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_MARKUP },
	{ "TB",	50, FF1234,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_TERRITORY },
	{ "TW",	50, FF1234,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_TERRITORY },
	{ "DD",	50, FF4,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_DIMMED },
	{ "SQ",	50, FF4,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_MARKUP },
	{ "SL",	50, FF1234,	Check_Pos,	Do_Markup,	CP_LIST|DOUBLE_MERGE, ST_MARKUP },

	{ "PL",	35, FF1234,	Check_Color,	NULL,	TYPE_SETUP, 0 },
	{ "V",	35, FF1234,	Check_Float,	NULL,	0, 0 },
	{ "GB",	35, FF1234,	Check_Triple,	Do_Annotate,	PVT_CHECK_EMPTY, ST_ANN_GEN },
	{ "GW",	35, FF1234,	Check_Triple,	Do_Annotate,	PVT_CHECK_EMPTY, ST_ANN_GEN },
	{ "UC",	35, FF34,	Check_Triple,	Do_Annotate,	PVT_CHECK_EMPTY, ST_ANN_GEN },
	{ "DM",	35, FF34,	Check_Triple,	Do_Annotate,	PVT_CHECK_EMPTY, ST_ANN_GEN },
	{ "TE",	35, FF1234,	Check_Triple,	Do_Annotate,	TYPE_MOVE|PVT_CHECK_EMPTY, ST_ANN_MOVE|ST_ANN_TE },
	{ "BM",	35, FF1234,	Check_Triple,	Do_Annotate,	TYPE_MOVE|PVT_CHECK_EMPTY, ST_ANN_MOVE|ST_ANN_BM },
	{ "DO",	35, FF34,	Check_Empty,	Do_Annotate,	TYPE_MOVE|PVT_EMPTY, ST_ANN_MOVE },
	{ "IT",	35, FF34,	Check_Empty,	Do_Annotate,	TYPE_MOVE|PVT_EMPTY, ST_ANN_MOVE },
	{ "HO",	35,	FF34,	Check_Triple,	Do_Annotate,	PVT_CHECK_EMPTY, 0 },

	{ "KO",	20, FF34,	Check_Empty,	Do_Annotate,	TYPE_MOVE|PVT_EMPTY, ST_KO },
	{ "FG",	20, FF1234,	Check_Figure,	NULL,			PVT_EMPTY|PVT_WEAKCOMPOSE, 0 },
	{ "MN",	20, FF34,	Check_Number,	NULL,			TYPE_MOVE, 0 },
	{ "VW",	90,	FF1234,	Check_Pos,		Do_View,		PVT_EMPTY|CP_LIST, 0 },
	{ "PM",	20, FF4,	Check_Number,	NULL,			0, 0 },

	/* properties not part of FF4 in addition to L, M */
	{ "CH",	5,	FF123,	Check_Triple,	NULL,		PVT_CHECK_EMPTY, 0 },
	{ "SI",	5,	FF3,	Check_Triple,	NULL,		PVT_CHECK_EMPTY, 0 },
	{ "BS",	5,	FF123,	Check_Number,	Do_GInfo,	TYPE_GINFO, 0 },
	{ "WS",	5,	FF123,	Check_Number,	Do_GInfo,	TYPE_GINFO, 0 },
	{ "ID",	5,	FF3,	Check_Text,		Do_GInfo, 	TYPE_GINFO|PVT_DEL_EMPTY|PVT_SIMPLE, 0 },
	{ "TC",	5,	FF3,	Check_Number,	NULL,	0, 0 },
	{ "OM",	5,	FF3,	Check_Number,	NULL,	0, 0 },
	{ "OP",	5,	FF3,	Check_Float,	NULL,	0, 0 },
	{ "OV",	5,	FF3,	Check_Float,	NULL,	0, 0 },
	{ "LT",	5,	FF3,	Check_Empty,	NULL,	PVT_EMPTY, 0 },
	{ "RG",	5,	FF123,	Check_Pos,		NULL,	PVT_LIST|DOUBLE_MERGE, 0 },
	{ "SC",	5,	FF123,	Check_Pos,		NULL,	PVT_LIST|DOUBLE_MERGE, 0 },
	{ "SE",	5,	FF3,	Check_Pos,		NULL,	PVT_LIST|DOUBLE_MERGE, 0 },
	{ "EL",	5,	FF12,	Check_Number,	NULL,	0, 0 },
	{ "EX",	5,	FF12,	Check_Move,		NULL,	0, 0 },

	{ NULL,	0,	0,		NULL,			NULL,	0, 0 }
};
