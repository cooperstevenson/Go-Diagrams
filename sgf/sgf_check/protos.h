/**************************************************************************
*** Project: SGF Syntax Checker & Converter
***	File:	 protos.h
***
*** Copyright (C) 1996-2004 by Arno Hollosi
*** (see 'main.c' for more copyright information)
***
**************************************************************************/


/**** main.c ****/

extern struct SGFInfo *sgfc;

extern char option_warnings;
extern char option_keep_head;
extern char option_keep_unknown_props;
extern char option_keep_obsolete_props;
extern char option_del_empty_nodes;
extern char option_del_move_markup;
extern char option_split_file;
extern char option_write_critical;
extern char option_interactive;
extern char option_linebreaks;
extern char option_softlinebreaks;
extern char option_expandcpl;
extern char option_findstart;
extern char option_pass_tt;
extern char option_fix_variation;
extern char option_game_signature;
extern char option_strict_checking;
extern char option_reorder_variations;
extern char *option_infile;
extern char *option_outfile;


/**** load.c ****/

void CopyValue(char *, char *, U_LONG , int );
struct PropValue *Add_PropValue(struct Property *, char *, char *,
								long, char *, long);
struct Property *Add_Property(struct Node *, token , char *, char *);
struct Node *NewNode(struct Node * , int);

char *SkipText(char * , char * , char , int );
void LoadSGF(struct SGFInfo * );


/**** save.c ****/

void SaveSGF(struct SGFInfo * );


/**** properties.c ****/

extern struct SGFToken sgf_token[];


/**** parse.c ****/

int Parse_Number(char * , U_SHORT );
int Parse_Text(char * , U_SHORT );
int Parse_Move(char * , U_SHORT );
int Parse_Float(char * , U_SHORT );
int Parse_Color(char * , U_SHORT );
int Parse_Triple(char * , U_SHORT );

int Check_Value(struct Property *, struct PropValue *, U_SHORT ,
				int (*)(char *, U_SHORT));
int Check_Text(struct Property *, struct PropValue *);
int Check_Label(struct Property *, struct PropValue *);
int Check_Pos(struct Property *, struct PropValue *);
int Check_AR_LN(struct Property *, struct PropValue *);
int Check_Figure(struct Property *, struct PropValue *);

void Check_Properties(struct Node *, struct BoardStatus *);


/**** parse2.c ****/

int ExpandPointList(struct Property *, struct PropValue *, int );
void CompressPointList(struct Property * );

void Split_Node(struct Node *, U_SHORT, token, int);
void ParseSGF(struct SGFInfo * );


/**** execute.c ****/

int Do_Move(struct Node *, struct Property *, struct BoardStatus *);
int Do_Addstones(struct Node *, struct Property *, struct BoardStatus *);
int Do_Letter(struct Node *, struct Property *, struct BoardStatus *);
int Do_Mark(struct Node *, struct Property *, struct BoardStatus *);
int Do_Markup(struct Node *, struct Property *, struct BoardStatus *);
int Do_Annotate(struct Node *, struct Property *, struct BoardStatus *);
int Do_Root(struct Node *, struct Property *, struct BoardStatus *);
int Do_GInfo(struct Node *, struct Property *, struct BoardStatus *);
int Do_View(struct Node *, struct Property *, struct BoardStatus *);


/**** gameinfo.c ****/

int Check_GameInfo(struct Property *, struct PropValue *);
int PromptGameInfo(struct Property *, struct PropValue *, int (*)(char *, U_SHORT));


/**** util.c ****/

extern int error_count;
extern int critical_count;
extern int warning_count;
extern int ignored_count;
extern char error_enabled[MAX_ERROR_NUM];

void SearchPos(char * , char * , int * , int * );
int PrintError(U_LONG , ... );

int  DecodePosChar(char );
char EncodePosChar(int );

void FreeSGFInfo(struct SGFInfo *);

void f_AddTail(struct ListHead * , struct ListNode * );
void f_Enqueue(struct ListHead * , struct ListNode * );
void f_Delete(struct ListHead * , struct ListNode * );

int strnccmp(char * , char * , int);
U_LONG Kill_Chars(char * , U_SHORT , char * );
U_LONG Test_Chars(char * , U_SHORT , char * );

struct Property *Find_Property(struct Node *, token );

struct PropValue *Del_PropValue(struct Property *, struct PropValue *);
struct Property *Del_Property(struct Node *, struct Property *);
struct Node *Del_Node(struct Node *, U_LONG);

struct Property *New_PropValue(struct Node *, token, char *, char *, int);


/**** strict.c ****/

void Strict_Checking(struct SGFInfo *sgf);


/**** protos.h ****/

#define AddTail(h,n) f_AddTail((struct ListHead *)(h), (struct ListNode *)(n))
#define Enqueue(h,n) f_Enqueue((struct ListHead *)(h), (struct ListNode *)(n))
#define Delete(h,n) f_Delete((struct ListHead *)(h), (struct ListNode *)(n))

#define SaveMalloc(type, v, sz, err)	{ v = (type)malloc((size_t)(sz)); if(!v) PrintError(FE_OUT_OF_MEMORY, err); }
