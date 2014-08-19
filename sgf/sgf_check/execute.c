/**************************************************************************
*** Project: SGF Syntax Checker & Converter
***	File:	 execute.c
***
*** Copyright (C) 1996-2004 by Arno Hollosi
*** (see 'main.c' for more copyright information)
***
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "all.h"
#include "protos.h"


U_LONG path_board[MAX_BOARDSIZE*MAX_BOARDSIZE];
U_LONG path_num = 0;


/**************************************************************************
*** Function:	Make_Capture
***				Captures stones on marked by Recursive_Capture
*** Parameters: x ... x position
***				y ... y position
*** Returns:	-
**************************************************************************/

void Make_Capture(int x, int y, struct BoardStatus *st)
{
	if(path_board[MXY(x,y)] != path_num)
		return;

	st->board[MXY(x,y)] = EMPTY;
	path_board[MXY(x,y)] = 0;

	/* recursive calls */
	if(x > 0)	Make_Capture(x-1, y, st);
	if(y > 0)	Make_Capture(x, y-1, st);
	if(x < sgfc->info->bwidth-1)	Make_Capture(x+1, y, st);
	if(y < sgfc->info->bheight-1)	Make_Capture(x, y+1, st);
}


/**************************************************************************
*** Function:	Recursive_Capture
***				Checks for capturing stones
***				marks its path on path_board with path_num
*** Parameters: color ... enemy color
***				x	  ... x position
***				y	  ... y position
*** Returns:	TRUE if capture / FALSE if liberty found
**************************************************************************/

int Recursive_Capture(int color, int x, int y, struct BoardStatus *st)
{
	if(!st->board[MXY(x,y)])
		return(FALSE);		/* liberty found */

	if(st->board[MXY(x,y)] == color)
		return(TRUE);		/* enemy found */

	if(path_board[MXY(x,y)] == path_num)
		return(TRUE);

	path_board[MXY(x,y)] = path_num;

	/* recursive calls */
	if(x > 0)	if(!Recursive_Capture(color, x-1, y, st))	return(FALSE);
	if(y > 0)	if(!Recursive_Capture(color, x, y-1, st))	return(FALSE);
	if(x < sgfc->info->bwidth-1)
				if(!Recursive_Capture(color, x+1, y, st))	return(FALSE);
	if(y < sgfc->info->bheight-1)
				if(!Recursive_Capture(color, x, y+1, st))	return(FALSE);

	return(TRUE);
}


/**************************************************************************
*** Function:	Capture_Stones
***				Calls Recursive_Capture and does capture if necessary
*** Parameters: st	  ... board status
***				color ... color of enemy stones
***				x	  ... column
***				y	  ... row
*** Returns:	-
**************************************************************************/

void Capture_Stones(struct BoardStatus *st, int color, int x, int y)
{
	if(x < 0 || y < 0 || x >= sgfc->info->bwidth || y >= sgfc->info->bheight)
		return;		/* not on board */

	if(!st->board[MXY(x,y)] || st->board[MXY(x,y)] == color)
		return;		/* liberty or friend found */

	if(!path_num)
		memset(path_board, 0, sizeof(path_board));

	path_num++;

	if(Recursive_Capture(color, x, y, st))	/* made prisoners? */
		Make_Capture(x, y, st);				/* ->update board position */
}



/**************************************************************************
*** Function:	Do_Move
***				Executes a move and check for B/W in one node
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_Move(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	int x, y;
	char color;

	if(sgfc->info->GM != 1)		/* game != Go? */
		return(TRUE);

	if(st->annotate & ST_MOVE)	/* there's a move already? */
	{
		PrintError(E_TWO_MOVES_IN_NODE, p->buffer);
		Split_Node(n, 0, p->id, TRUE);
		return(TRUE);
	}

	st->annotate |= ST_MOVE;

	if(!strlen(p->value->value))	/* pass move */
		return(TRUE);

	x = DecodePosChar(p->value->value[0]) - 1;
	y = DecodePosChar(p->value->value[1]) - 1;
	color = (char)sgf_token[p->id].data;

	if(st->board[MXY(x,y)])
		PrintError(WS_ILLEGAL_MOVE, p->buffer);

	st->board[MXY(x,y)] = color;
	Capture_Stones(st, color, x-1, y);		/* check for prisoners */
	Capture_Stones(st, color, x+1, y);
	Capture_Stones(st, color, x, y-1);
	Capture_Stones(st, color, x, y+1);
	Capture_Stones(st, ~color, x, y);		/* check for suicide */

	if(option_del_move_markup)			/* if del move markup, then */
	{						/* mark move position as markup */
		st->markup[MXY(x,y)] |= ST_MARKUP;	/* -> other markup at this */
		st->mrkp_chngd = TRUE;			/* position will be deleted */
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Do_AddStones
***				Executes property checks for unique positions
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_Addstones(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	int x, y;
	char color;
	struct PropValue *v, *w;
	struct Property h;

	if(sgfc->info->GM != 1)		/* game != Go? */
		return(TRUE);

	h.value = NULL;
	h.valend = NULL;

	color = (char)sgf_token[p->id].data;

	v = p->value;
	while(v)
	{
		x = DecodePosChar(v->value[0]) - 1;
		y = DecodePosChar(v->value[1]) - 1;
	
		if(st->markup[MXY(x,y)] & ST_ADDSTONE)
		{
			PrintError(E_POSITION_NOT_UNIQUE, v->buffer, "AddStone", p->idstr);
			v = Del_PropValue(p, v);
			continue;
		}

		st->markup[MXY(x,y)] |= ST_ADDSTONE;
		st->mrkp_chngd = TRUE;

		if(st->board[MXY(x,y)] == color)		/* Add property is redundant */
		{
			w = v->next;
			Delete(&p->value, v);
			AddTail(&h.value, v);
			v = w;
			continue;
		}

		st->board[MXY(x,y)] = color;
		v = v->next;
	}

	if(h.value)
	{
		x = PrintError(WS_ADDSTONE_REDUNDANT, p->buffer, p->idstr);

		v = h.value;
		while(v)
		{
			if(x)	fprintf(E_OUTPUT, "[%s]", v->value);
			v = Del_PropValue(&h, v);
		}

		if(x)
			fprintf(E_OUTPUT, "\n");
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Do_Letter
***				Converts L to LB values / checks unique position
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_Letter(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	int x, y;
	struct PropValue *v;
	char letter[2] = "a";

	if(sgfc->info->GM != 1)		/* game != Go? */
		return(TRUE);

	v = p->value;
	while(v)
	{
		x = DecodePosChar(v->value[0]) - 1;
		y = DecodePosChar(v->value[1]) - 1;
	
		if(st->markup[MXY(x,y)] & ST_LABEL)
		{
			PrintError(E_POSITION_NOT_UNIQUE, v->buffer, "Label", p->idstr);
		}
		else
		{
			st->markup[MXY(x,y)] |= ST_LABEL;
			st->mrkp_chngd = TRUE;
			New_PropValue(n, TKN_LB, v->value, letter, FALSE);
			letter[0]++;
		}

		v = v->next;
	}

	return(FALSE);
}


/**************************************************************************
*** Function:	Do_Mark
***				Converts M to MA/TR depending on board / checks uniqueness
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_Mark(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	int x, y;
	struct PropValue *v;

	if(sgfc->info->GM != 1)		/* game != Go? */
		return(TRUE);

	v = p->value;
	while(v)
	{
		x = DecodePosChar(v->value[0]) - 1;
		y = DecodePosChar(v->value[1]) - 1;
	
		if(st->markup[MXY(x,y)] & ST_MARKUP)
		{
			PrintError(E_POSITION_NOT_UNIQUE, v->buffer, "Markup", p->idstr);
		}
		else
		{
			st->markup[MXY(x,y)] |= ST_MARKUP;
			st->mrkp_chngd = TRUE;

			if(st->board[MXY(x,y)])
				New_PropValue(n, TKN_TR, v->value, NULL, FALSE);
			else
				New_PropValue(n, TKN_MA, v->value, NULL, FALSE);
		}
		v = v->next;
	}

	return(FALSE);
}


/**************************************************************************
*** Function:	Do_Markup
***				Checks unique positions for markup properties
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_Markup(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	int x, y;
	struct PropValue *v;
	U_SHORT flag;

	if(sgfc->info->GM != 1)		/* game != Go? */
		return(TRUE);

	v = p->value;
	flag = sgf_token[p->id].data;

	while(v)
	{
		x = DecodePosChar(v->value[0]) - 1;
		y = DecodePosChar(v->value[1]) - 1;
	
		if(st->markup[MXY(x,y)] & flag)
		{
			PrintError(E_POSITION_NOT_UNIQUE, v->buffer, "Markup", p->idstr);
			v = Del_PropValue(p, v);
			continue;
		}

		st->markup[MXY(x,y)] |= flag;
		st->mrkp_chngd = TRUE;
		v = v->next;
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Do_Annotate
***				Checks annotation properties / converts BM_TE / TE_BM
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_Annotate(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	struct Property *hlp;
	U_SHORT flag;

	flag = sgf_token[p->id].data;

	if((st->annotate & ST_ANN_BM) && p->id == TKN_TE) /* DO (doubtful) */
	{
		PrintError(E4_BM_TE_IN_NODE, p->buffer, "BM-TE", "DO");
		hlp = Find_Property(n, TKN_BM);
		hlp->id = TKN_DO;
		hlp->value->value[0] = 0;
		return(FALSE);
	}

	if(st->annotate & ST_ANN_TE && p->id == TKN_BM)	/* IT (interesting) */
	{
		PrintError(E4_BM_TE_IN_NODE, p->buffer, "TE-BM", "IT");
		hlp = Find_Property(n, TKN_TE);
		hlp->id = TKN_IT;
		hlp->value->value[0] = 0;
		return(FALSE);
	}

	if(st->annotate & flag)
	{
		PrintError(E_ANNOTATE_NOT_UNIQUE, p->buffer, p->idstr);
		return(FALSE);
	}

	if((flag & (ST_ANN_MOVE|ST_KO)) && !(st->annotate & ST_MOVE))
	{
		PrintError(E_ANNOTATE_WITHOUT_MOVE, p->buffer, p->idstr);
		return(FALSE);
	}

	st->annotate |= flag;
	return(TRUE);
}


/**************************************************************************
*** Function:	Do_Root
***				Checks if root properties are stored in root
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_Root(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	if(n->parent)
	{
		PrintError(E_ROOTP_NOT_IN_ROOTN, p->buffer, p->idstr);
		return(FALSE);
	}
	else
		return(TRUE);
}


/**************************************************************************
*** Function:	Do_GInfo
***				checks for uniqueness of properties within the tree
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_GInfo(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	int x, y;

	if(st->ginfo && (st->ginfo != n->buffer))
	{
		SearchPos(st->ginfo, sgfc->buffer, &x, &y);
		PrintError(E4_GINFO_ALREADY_SET, p->buffer, p->idstr, y, x);
		return(FALSE);
	}
	else
	{
		st->ginfo = n->buffer;

		if(p->id == TKN_KI)
		{
			if(Find_Property(n, TKN_KM))
				PrintError(W_INT_KOMI_FOUND, p->buffer, "deleted (<KM> property found)");
			else
			{
				PrintError(W_INT_KOMI_FOUND, p->buffer, "converted to <KM>");

				x = atoi(p->value->value);
				p = New_PropValue(n, TKN_KM, "1234567890", NULL, FALSE);

				if(x & 1)	sprintf(p->value->value, "%d.5", x/2);
				else		sprintf(p->value->value, "%d", x/2);

			}
			return(FALSE);
		}
		return(TRUE);
	}
}


/**************************************************************************
*** Function:	Do_View
***				checks and converts VW property
*** Parameters: n	... Node that contains the property
***				p	... property
***				st	... current board status
*** Returns:	TRUE: ok / FALSE: delete property
**************************************************************************/

int Do_View(struct Node *n, struct Property *p, struct BoardStatus *st)
{
	struct PropValue *v;
	int i = 0;

	v = p->value;

	if(!strlen(v->value))	/* VW[] */
	{
		if(v->next)
		{
			PrintError(E_BAD_VW_VALUES, p->buffer, "values after '[]' value found", "deleted");
			v = v->next;
			while(v)
				v = Del_PropValue(p, v);
		}

		return(TRUE);
	}

	while(v)
	{
		i++;
		if(!strlen(v->value))	/* '[]' within other values */
		{
			PrintError(E_BAD_VW_VALUES, v->buffer, "empty value found in list", "deleted");
			v = Del_PropValue(p, v);
		}
		else
			v = v->next;
	}

	if(sgfc->info->GM != 1)		/* game not Go */
		return(TRUE);

	if(sgfc->info->FF < 4)		/* old definition of VW */
	{
		if(i == 2)				/* transform FF3 values */
		{
			v = p->value;
			v->value2 = v->next->value;
			v->next->value = NULL;
			Del_PropValue(p, v->next);
		
			if(!ExpandPointList(p, v, FALSE))
			{
				PrintError(E_BAD_VW_VALUES, v->buffer, "illegal FF[3] definition", "deleted");
				return(FALSE);
			}

			Del_PropValue(p, v);
		}
		else		/* looks like FF4 defintion (wrong FF set?) */
			PrintError(E_BAD_VW_VALUES, p->buffer, "FF[4] definition in older FF found", "parsing done anyway");
	}

	return(TRUE);
}
