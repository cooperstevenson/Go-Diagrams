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
#include <string.h>
#include <ctype.h>

#include "all.h"
#include "protos.h"


/**************************************************************************
*** Function:	ExpandPointList
***				Expanding compressed pointlists
*** Parameters: p ... property
***				v ... compose value to be expanded
***				error ... if TRUE print errors
*** Returns:	TRUE if success / FALSE on error (exits on low memory)
**************************************************************************/

int ExpandPointList(struct Property *p, struct PropValue *v, int error)
{
	int x1, y1, x2, y2, h = 0;
	char val[2];
	
	x1 = DecodePosChar(v->value[0]);
	y1 = DecodePosChar(v->value[1]);
	x2 = DecodePosChar(v->value2[0]);
	y2 = DecodePosChar(v->value2[1]);

	if(x1 == x2 && y1 == y2)	/* illegal definition */
	{
		free(v->value2);
		v->value2 = NULL;
		if(error)
			PrintError(E_BAD_VALUE_CORRECTED, v->buffer, p->idstr, v->value);
		return(FALSE);
	}

	if(x1 > x2)					/* encoded as [ul:lr] ? */
	{
		h = x1; x1 = x2; x2 = h;
		v->value[0] = EncodePosChar(x1);
		v->value2[0] = EncodePosChar(x2);
	}

	if(y1 > y2)
	{
		h = y1; y1 = y2; y2 = h;
		v->value[1] = EncodePosChar(y1);
		v->value2[1] = EncodePosChar(y2);
	}

	if(h && error)
		PrintError(E_BAD_COMPOSE_CORRECTED, v->buffer, p->idstr, v->value, v->value2);

	for(; x1 <= x2; x1++)		/* expand point list */
		for(h = y1; h <= y2; h++)
		{
			val[0] = EncodePosChar(x1);
			val[1] = EncodePosChar(h);
			Add_PropValue(p, v->buffer, val, 2, NULL, 0);
		}

	return(TRUE);
}


/**************************************************************************
*** Function:	CompressPointList
***				A simple greedy algorithm to compress pointlists
*** Parameters: p ... property which list should get compressed
*** Returns:	- (exits on low memory)
**************************************************************************/

void CompressPointList(struct Property *p)
{
	char board[MAX_BOARDSIZE+2][MAX_BOARDSIZE+2];
	int x, y, yy, i, j, m, mx, my, expx, expy;
	struct PropValue *v;
	char val1[12], val2[2];

	memset(board, 0, (MAX_BOARDSIZE+2)*(MAX_BOARDSIZE+2));

	x = yy = MAX_BOARDSIZE+10;
	mx = my = 0;

	v = p->value;
	while(v)		/* generate board position & delete old values */
	{
		if(strlen(v->value))
		{
			i = DecodePosChar(v->value[0]);
			j = DecodePosChar(v->value[1]);
			board[i][j] = 1;
			if(x > i)	x = i;						/* get minimum */
			if(yy > j)	yy = j;
			if(mx < i)	mx = i;						/* get maximum */
			if(my < j)	my = j;

			v = Del_PropValue(p, v);
		}
		else
			v = v->next;
	}

	for(; x <= mx; x++)							/* search whole board */
		for(y = yy; y <= my; y++)
			if(board[x][y])						/* starting point found */
			{									/* --> ul corner */
				i = x;	j = y;
				expx = TRUE;	expy = TRUE;
				while(expx || expy)				/* still enlarging area? */
				{
					if(board[i+1][y] && expx)
					{
						for(m = y; m <= j; m++)
							if(!board[i+1][m])
								break;
						if(m > j)				/* new column ok? */
							i++;
						else
							expx = FALSE;		/* x limit reached */
					}
					else
						expx = FALSE;

					if(board[x][j+1] && expy)
					{
						for(m = x; m <= i; m++)
							if(!board[m][j+1])
								break;
						if(m > i)				/* new row ok? */
							j++;
						else
							expy = FALSE;		/* y limit reached */
					}
					else
						expy = FALSE;
				}

				val1[0] = EncodePosChar(x);
				val1[1] = EncodePosChar(y);
				val2[0] = EncodePosChar(i);
				val2[1] = EncodePosChar(j);

				if(x != i || y != j)			/* Add new values to property */
					Add_PropValue(p, NULL, val1, 2, val2, 2);
				else
					Add_PropValue(p, NULL, val1, 2, NULL, 0);

				for(; i >= x; i--)				/* remove points from board */
					for(m = j; m >= y; m--)
						board[i][m] = 0;
			}
}


/**************************************************************************
*** Function:	Correct_Variation
***				Checks if variation criteria fit and corrects level
*** Parameters: n ... first node of siblings (child of parent)
*** Returns:	-
**************************************************************************/

void Correct_Variation(struct Node *n)
{
	struct Node *p, *i, *j, *k;
	struct Property *pmv, *w, *b, *ae;
	int fault = 0, success = 0;

	p = n->parent;
	if(!p)					/* don't mess with root nodes */
		return;

	if(p->parent)			/* Does parent have siblings? */
		if(p->sibling || (p->parent->child != p))
			return;

	pmv = Find_Property(p, TKN_B);
	if(!pmv)
	{
		pmv = Find_Property(p, TKN_W);
		if(!pmv)
			return;			/* no move in parent found */
	}

	i = n->sibling;
	while(i)					/* check variations */
	{
		fault++;
		j = i;
		i = i->sibling;

		ae = Find_Property(j, TKN_AE);
		if(ae)
		{
			if(ae->value->next)			/* AE has more than one value */
				continue;
			if(strcmp(ae->value->value, pmv->value->value))
				continue;				/* AE doesn't undo parent move */

			w = Find_Property(j, TKN_AW);
			b = Find_Property(j, TKN_AB);

			if(w || b)					/* AB or AW in same node as AE */
			{
				if(w && b)				/* AW and AB in same node */
					continue;

				if(w)	b = w;
				if(b->value->next)		/* AB/AW has more than one value */
					continue;

				if(b->id == TKN_AW)		b->id = TKN_W;
				else					b->id = TKN_B;
				b->flags = sgf_token[b->id].flags;	/* update local copy */

				Split_Node(j, TYPE_SETUP|TYPE_ROOT|TYPE_GINFO, TKN_N, FALSE);
			}

			if(j->child)				/* variation must have a child */
			{
				if(j->child->sibling)	/* but child must not have siblings */
					continue;

				if(Find_Property(j->child, TKN_W) || Find_Property(j->child, TKN_B))
				{
					w = j->prop;		/* check for properties which occur */
					while(w)			/* in node AND child */
					{
						if(Find_Property(j->child, w->id))
							break;
						w = w->next;
					}

					if(!w)				/* no double properties */
					{					/* -> nodes may be merged and moved */
						success++;
						fault--;
					}
				}
			}
		}
	}

	if(fault && success)		/* critical case */
	{
		PrintError(W_VARLEVEL_UNCERTAIN, n->buffer);
		return;
	}

	if(success)					/* found variations which can be corrected */
	{
		PrintError(W_VARLEVEL_CORRECTED, n->buffer);

		i = n->sibling;
		while(i)
		{
			b = i->prop;
			while(b)			/* move !SETUP properties to second node */
			{
				w = b->next;

				if(!(b->flags & TYPE_SETUP))
				{
					Delete(&i->prop, b);
					Enqueue(&i->child->prop, b);
				}

				b = w;
			}

			j = i->child;
			i->child = NULL;
			i = i->sibling;

			Del_Node(j->parent, E_NO_ERROR);	/* delete SETUP node */

			j->parent = p->parent;				/* move tree to new level */
			k = p;
			while(k->sibling)
				k = k->sibling;
			k->sibling = j;
		}
	}
}

/**************************************************************************
*** Function:	Correct_Variations
***				Checks for wrong variation levels and corrects them
*** Parameters: r ... start node
***				ti ... pointer to TreeInfo (for check of GM)
*** Returns:	-
**************************************************************************/

void Correct_Variations(struct Node *r, struct TreeInfo *ti)
{
	struct Node *n;

	if(!r)
		return;

	if(!r->parent)		/* root node? */
	{
		if(Find_Property(r, TKN_B) || Find_Property(r, TKN_W))
		{
			Split_Node(r, TYPE_ROOT|TYPE_GINFO, TKN_NONE, FALSE);
			PrintError(WS_MOVE_IN_ROOT, r->buffer);
		}
	}

	if(ti->GM != 1)		/* variation level correction only for Go games */
		return;

	while(r)
	{
		if(r->sibling)
		{
			n = r;
			while(n)
			{
				if(!r->parent)	Correct_Variations(n->child, ti->next);
				else			Correct_Variations(n->child, ti);

				n = n->sibling;
			}

			Correct_Variation(r);
			break;
		}

		r = r->child;
	}

	return;
}


/**************************************************************************
*** Function:	Reorder_Variations
***				Reorders variations (including main branch) from A,B,C to C,B,A
*** Parameters: r ... start node
*** Returns:	-
**************************************************************************/

void Reorder_Variations(struct Node *r)
{
	struct Node *n, *s[MAX_REORDER_VARIATIONS];
	int i;

	if(!r)
		return;

	if (!r->parent && r->sibling)
		Reorder_Variations(r->sibling);

	while(r)
	{
		if(r->child && r->child->sibling)
		{
			i = 0;
			n = r->child;
			while(n)
			{
				if(i >= MAX_REORDER_VARIATIONS)
				{
					PrintError(E_TOO_MANY_VARIATIONS, n->buffer);
					break;
				}
				s[i++] = n;
				Reorder_Variations(n);
				n = n->sibling;
			}
			if(i < MAX_REORDER_VARIATIONS)
			{
				i--;
				s[0]->sibling = NULL;
				s[0]->parent->child = s[i];
				for(; i > 0; i--)
					s[i]->sibling = s[i-1];
			}
			break;
		}
		r = r->child;
	}
	return;
}


/**************************************************************************
*** Function:	Del_EmptyNodes (recursive)
***				Deletes empty nodes
*** Parameters: n ... start node
*** Returns:	-
**************************************************************************/

void Del_EmptyNodes(struct Node *n)
{
	if(n->child)
		Del_EmptyNodes(n->child);

	if(n->sibling)
		Del_EmptyNodes(n->sibling);

	if(!n->prop)
		Del_Node(n, W_EMPTY_NODE_DELETED);
}


/**************************************************************************
*** Function:	Calc_GameSig
***				Calculates and prints game signature as proposed
***				by Dave Dyer
*** Parameters: r ... first root node
***				ti .. first tree info structure
*** Returns:	-
**************************************************************************/

void Calc_GameSig(struct Node *r, struct TreeInfo *ti)
{
	char id[14];
	int i;
	struct Property *b, *w;
	struct Node *n;

	while(r)
	{
		if(ti->GM == 1)
		{
			strcpy(id, "------ ------");
			n = r;
			i = 0;
			while(n && i < 71)
			{
				b = Find_Property(n, TKN_B);
				w = Find_Property(n, TKN_W);
				if(b || w)
				{
					i++;
					if(w)
						b = w;

					switch(i)
					{
						case 20:
						case 40:
						case 60:	id[i/10-2] = b->value->value[0];
									id[i/10-1] = b->value->value[1];
									break;
						case 31:
						case 51:
						case 71:	id[i/10+4] = b->value->value[0];
									id[i/10+5] = b->value->value[1];
									break;
						default:	break;
					}
				}
				n = n->child;
			}

			printf("Game signature - tree %d: '%s'\n", ti->num, id);
		}
		else
			printf("Game signature - tree %d: contains GM[%d] "
				   "- can't calculate signature\n", ti->num, ti->GM);

		r = r->sibling;
		ti = ti->next;
	}
}


/**************************************************************************
*** Function:	Split_Node
***				Splits one node into two and moves selected properties
***				into the second node
*** Parameters: n		... node that should be split
***				flags	... prop->flags to select properties
***				id		... id of an extra property
***				move	... TRUE:  move selected to second node
***							FALSE: selected props stay in first node
*** Returns:	-
**************************************************************************/

void Split_Node(struct Node *n, U_SHORT flags, token id, int move)
{
	struct Property *p, *hlp;
	struct Node *newnode;

	newnode = NewNode(n, TRUE);		/* create new child node */
	newnode->buffer = n->buffer;

	p = n->prop;
	while(p)
	{
		hlp = p->next;
		if((move && ((p->flags & flags) || p->id == id)) ||
		  (!move && (!(p->flags & flags) && p->id != id)))
		{
			Delete(&n->prop, p);
			AddTail(&newnode->prop, p);
		}
		p = hlp;
	}
}


/**************************************************************************
*** Function:	Split_MoveSetup
***				Tests if node contains move & setup properties
***				if yes -> splits node into two: setup & N[] --- other props
***				Deletes PL[] property if it's the only setup property
***				(frequent error of an application (which one?))
*** Parameters: n ... pointer to Node
*** Returns:	TRUE if node is split / FALSE otherwise
**************************************************************************/

int Split_MoveSetup(struct Node *n)
{
	struct Property *p, *s = NULL;
	U_SHORT f, sc = 0;

	p = n->prop;
	f = 0;

	while(p)				/* OR all flags */
	{
		if(p->flags & TYPE_SETUP)
		{
			s = p;			/* remember property */
			sc++;			/* setup count++ */
		}

		f |= p->flags;
		p = p->next;
	}

	if((f & TYPE_SETUP) && (f & TYPE_MOVE))		/* mixed them? */
	{
		if(sc == 1 && s->id == TKN_PL)			/* single PL[]? */
		{
			PrintError(E4_MOVE_SETUP_MIXED, s->buffer, "deleted PL property");
			Del_Property(n, s);
		}
		else
		{
			PrintError(E4_MOVE_SETUP_MIXED, s->buffer, "split into two nodes");
			Split_Node(n, TYPE_SETUP|TYPE_GINFO|TYPE_ROOT, TKN_N, FALSE);
			return(TRUE);
		}
	}
	return(FALSE);
}


/**************************************************************************
*** Function:	Check_DoubleProp
***				Checks uniqueness of properties within a node
***				Tries to merge or link values, otherwise deletes property
***				- can't merge !PVT_LIST && PVT_COMPOSE
*** Parameters: n ... pointer to Node
*** Returns:	-
**************************************************************************/

void Check_DoubleProp(struct Node *n)
{
	struct Property *p, *q;
	struct PropValue *v, *w;
	char *c;
	size_t l;

	p = n->prop;
	while(p)
	{
		q = p->next;
		while(q)
		{
			if(!strcmp(p->idstr, q->idstr))		/* ID's identical? */
			{
				if(p->flags & DOUBLE_MERGE)
				{
					PrintError(E_DOUBLE_PROP, q->buffer, q->idstr, "values merged");
					if(p->flags & PVT_LIST)
					{
						v = p->value;
						while(v->next)	v = v->next;
						v->next = q->value;
						q->value->prev = v;
						q->value = NULL;	/* values are not deleted */
					}
					else	/* single values are merged to one value */
					{
						v = p->value;	l = strlen(v->value);
						w = q->value;

						SaveMalloc(char *, c, l + strlen(w->value) + 4, "new property value");

						strcpy(c, v->value);
						strcpy(c+l+2, w->value);
						c[l]   = '\n';
						c[l+1] = '\n';

						free(v->value);		/* free old buffer */
						v->value = c;
					}
				}
				else
					PrintError(E_DOUBLE_PROP, q->buffer, q->idstr, "deleted");

				q = Del_Property(n, q);	/* delete double property */
			}
			else
				q = q->next;
		}
		p = p->next;
	}
}


/**************************************************************************
*** Function:	GetNumber
***				Parses a positive int value for correctness
***				deletes property otherwise
*** Parameters: n ... pointer to node that contains property
***				p ... pointer to property
***				value ... 2: parse value2 else parse value1
***				d ... pointer to int variable
***				def ... default return value
*** Returns:	TRUE ... success / FALSE ... errornous property deleted
**************************************************************************/

int GetNumber(struct Node *n, struct Property *p, int value, int *d,
			  int def, char *err_action)
{
	char *v;

	if(!p)				/* no property? -> set default value */
	{
		*d = def;
		return(TRUE);
	}

	if(value == 2)	v = p->value->value2;
	else			v = p->value->value;

	switch(Parse_Number(v, 0))
	{
		case 0: PrintError(E_BAD_ROOT_PROP, p->value->buffer, p->idstr, err_action);
				*d = def;
				Del_Property(n, p);
				return(FALSE);

		case -1: PrintError(E_BAD_VALUE_CORRECTED, p->value->buffer, p->idstr, v);
		case 1:	*d = atoi(v);
				if(*d < 1)
				{
					PrintError(E_BAD_ROOT_PROP, p->value->buffer, p->idstr, err_action);
					*d = def;
					Del_Property(n, p);
					return(FALSE);
				}
				break;
	}

	return(TRUE);
}


/**************************************************************************
*** Function:	Init_TreeInfo
***				Creates new TreeInfo structure, inits it
***				and set sgfc->info to the new structure
*** Parameters: r ... pointer to root node
*** Returns:	-
**************************************************************************/

void Init_TreeInfo(struct Node *r)
{
	static int FF_diff = 0, GM_diff = 0;
	struct TreeInfo *ti;
	struct Property *ff, *gm, *sz;

	SaveMalloc(struct TreeInfo *, ti, sizeof(struct TreeInfo), "tree info structure");

	ti->FF = 0;						/* Init structure */
	ti->GM = 0;
	ti->bwidth = ti->bheight = 0;
	ti->root = r;
	if(sgfc->last)
		ti->num = sgfc->last->num + 1;
	else
		ti->num = 1;

	AddTail(&sgfc->tree, ti);		/* add to SGFInfo */

	sgfc->info = ti;				/* set as current tree info */

	ff = Find_Property(r, TKN_FF);
	if(!GetNumber(r, ff, 1, &ti->FF, 1, "FF[1]"))
		ff = NULL;

	if(ti->FF > 4)
		PrintError(FE_UNKNOWN_FILE_FORMAT, ff->value->buffer);

	gm = Find_Property(r, TKN_GM);
	if(!GetNumber(r, gm, 1, &ti->GM, 1, "GM[1]"))
		gm = NULL;

	if(ti->GM == 1)		/* board size only of interest if Game == Go */
	{
		sz = Find_Property(r, TKN_SZ);
		if(sz)
		{
			if(GetNumber(r, sz, 1, &ti->bwidth, 19, "19x19"))
			{
				if(ti->FF < 4 && (ti->bwidth > 19 || sz->value->value2))
					PrintError(E_VERSION_CONFLICT, sz->buffer, ti->FF);

				if(sz->value->value2)	/* rectangular board? */
				{
					if(!GetNumber(r, sz, 2, &ti->bheight, 19, "19x19"))
						ti->bwidth = 19;
					else
					{
						if(ti->bwidth == ti->bheight)
						{
							PrintError(E_SQUARE_AS_RECTANGULAR, sz->buffer);
							free(sz->value->value2);
							sz->value->value2 = NULL;
						}
					}
				}
				else			/* square board */
					ti->bheight = ti->bwidth;

				if(ti->bheight > 52 || ti->bwidth > 52)	/* board too big? */
				{
					if(ti->bwidth > 52)
					{
						ti->bwidth = 52;
						strcpy(sz->value->value, "52");
					}

					if(ti->bheight > 52)
					{
						ti->bheight = 52;
						if(sz->value->value2)
							strcpy(sz->value->value2, "52");
					}

					if(ti->bwidth == ti->bheight && sz->value->value2)
					{
						free(sz->value->value2);
						sz->value->value2 = NULL;
					}

					PrintError(E_BOARD_TOO_BIG, sz->buffer, ti->bwidth, ti->bheight);
				}
			}
			else			/* faulty SZ deleted */
				ti->bheight = 19;
		}
		else
			ti->bwidth = ti->bheight = 19;	/* default size */
	}
	else
		PrintError(WCS_GAME_NOT_GO, gm->buffer, ti->num);

	if(ti->prev)
	{
		char *buffer;

		if(ti->prev->FF != ti->FF && !FF_diff)
		{
			if(ff)	buffer = ff->buffer;
			else	buffer = r->buffer;

			PrintError(WS_ROOT_PROP_DIFFERS, buffer, "file formats");
			FF_diff = 1;
		}

		if(ti->prev->GM != ti->GM && !GM_diff)
		{
			if(gm)	buffer = gm->buffer;
			else	buffer = r->buffer;

			PrintError(WS_ROOT_PROP_DIFFERS, buffer, "game types");
			GM_diff = 1;
		}
	}
}


/**************************************************************************
*** Function:	Check_SGFTree
***				Steps recursive through the SGF tree and
***				calls Check_Properties for each node
*** Parameters: r   ... pointer to root node of current tree
***				old ... board status before parsing root node
*** Returns:	-
**************************************************************************/

void Check_SGFTree(struct Node *r, struct BoardStatus *old)
{
	struct Node *n;
	struct BoardStatus *st;

	SaveMalloc(struct BoardStatus *, st, sizeof(struct BoardStatus), "board status buffer");

	while(r)
	{
		st->board = NULL;
		st->markup = NULL;
		if(old)
		{
			memcpy(st, old, sizeof(struct BoardStatus));

			if(st->bsize)
			{
				SaveMalloc(char *, st->board, st->bsize, "goban buffer");
				memcpy(st->board, old->board, st->bsize);
			}

			if(st->msize)
				SaveMalloc(U_SHORT *, st->markup, st->msize, "markup buffer");

			st->mrkp_chngd = TRUE;
		}
		else
		{
			Init_TreeInfo(r);		/* set TreeInfo */
			memset(st, 0, sizeof(struct BoardStatus));
			st->bwidth = sgfc->info->bwidth;

			st->bsize = st->bwidth * sgfc->info->bheight * sizeof(char);
			if(st->bsize)
			{
				SaveMalloc(char *, st->board, st->bsize, "goban buffer");
				memset(st->board, 0, st->bsize);
			}

			st->msize = st->bwidth * sgfc->info->bheight * sizeof(U_SHORT);
			if(st->msize)
				SaveMalloc(U_SHORT *, st->markup, st->msize, "markup buffer");
			st->mrkp_chngd = TRUE;
		}

		n = r;
		while(n)
		{
			st->annotate = 0;
			if(st->mrkp_chngd && st->msize)
				memset(st->markup, 0, st->msize);
			st->mrkp_chngd = FALSE;

			if(n->sibling && n != r)	/* for n=r loop is done outside */
			{
				Check_SGFTree(n, st);
				break;					/* did complete subtree -> break */
			}

			Check_DoubleProp(n);		/* remove/merge double properties */

			Check_Properties(n, st);	/* perform checks, update board status */

			if(Split_MoveSetup(n))
				n = n->child;			/* new child node already parsed */

			n = n->child;
		}

		r = r->sibling;
		if(st->board)	free(st->board);
		if(st->markup)	free(st->markup);
	}

	free(st);
}


/**************************************************************************
*** Function:	ParseSGF
***				Calls the check routines one after another
*** Parameters: sgf ... pointer to SGFInfo structure
*** Returns:	-
**************************************************************************/

void ParseSGF(struct SGFInfo *sgf)
{
	sgfc = sgf;		/* set current SGFInfo context */
					/* sgfc->info isn't initialized yet! */

	Check_SGFTree(sgf->root, NULL);

	if(option_fix_variation)
		Correct_Variations(sgfc->root, sgfc->tree);

	if(option_del_empty_nodes)
		Del_EmptyNodes(sgfc->root);

	if(option_reorder_variations)
		Reorder_Variations(sgfc->root);

	if(option_game_signature)
		Calc_GameSig(sgfc->root, sgfc->tree);

	if(option_strict_checking)
		Strict_Checking(sgfc);
}
