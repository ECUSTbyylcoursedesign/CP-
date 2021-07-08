#include <cstring>

#include "def.h"

//---------------------------------
//-------------Error
//---------------------------------

char* err_msg[emsgsize] = // ������Ϣ��
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "",
/* 26 */    "",
/* 27 */    "",
/* 28 */    "",
/* 29 */    "",
/* 30 */    "",
/* 31 */    "The number is too great.",
/* 32 */    "There are too many levels.",
/* 33 */    "';' expected, but 'ELSE' found.",
/* 34 */    "'EXIT' Found, but outside the 'WHILE'.",
/* 35 */    "There must be an '(' to follow the 'WRITE'/'READ'/'ODD'.",
/* 36 */    "Variable identifier expected.",
/* 37 */    "'BEGIN' expected.",
/* 38 */    "Undeclared Variable Type.",
/* 39 */    "':' expected."
};
long err;                 // �������

//---------------------------------
//-------------Lexer
//---------------------------------

long cc;                  // �м���
long ll;                  // �г���
FILE* infile;             // �����ļ�
long cx;                  // �������ָ��
char ch;                  // ���������һ���ַ�
char line[81];            // �л�����
char a[al+1];             // �ʷ������������ڴ�����ڱ��������Ĵ�
long kk;                  // ��ǰtoken�ĳ���
char id[al+1];            // ���������һ����ʶ��
char word[norw][al+1];    // �����ֱ�
unsigned _int64 wsym[norw]; // ÿһ�������ֶ�Ӧ��symbol���ͱ�
unsigned _int64 ssym[256];  // ÿһ�����Ŷ�Ӧ��symbol���ͱ�
unsigned _int64 sym;        // ���������һ��token������
long num;                 // ���������һ������

//---------------------------------
//-------------Environment
//---------------------------------

long tx;                  // ���ű�ָ��
tabletype table[txmax+1]; // ���ű�
long dx;                  // ����ջָ��
long lev;                 // ��ǰ�㼶
long enterlist[etlsize];  // ���������е�enterlist��
long etlx;                // enterlistָ��

//---------------------------------
//-------------Generate
//---------------------------------

instruction code[cxmax+1];// �м����
char mnemonic[8][3+1];    // �м�������Ƿ���

//---------------------------------
//-------------Parse
//---------------------------------

unsigned _int64 declbegsys; // decl��ʼ���ż���
unsigned _int64 statbegsys; // stmt��ʼ���ż���
unsigned _int64 simpexpbegsys;// simpexp��ʼ���ż���
unsigned _int64 termbegsys; // term��ʼ���ż���
unsigned _int64 facbegsys;  // factor��ʼ���ż���
long exitlist[elsize];    // while�е�exit��ַ��
long elx;                 // exitlistָ��

//---------------------------------
//-------------Interpretation
//---------------------------------

long s[stacksize];        // ����ջ

//---------------------------------
//-------------Initialization
//---------------------------------

void globalinit()
{
	for(int i=0; i<256; i++)
    {
        ssym[i]=nul;
    }

    strcpy(word[0],  "and       ");
    strcpy(word[1],  "begin     ");
    strcpy(word[2],  "boolean   ");
    strcpy(word[3],  "call      ");
    strcpy(word[4],  "const     ");
    strcpy(word[5],  "div       ");
    strcpy(word[6],  "do        ");
    strcpy(word[7],  "else      ");
    strcpy(word[8],  "end       ");
    strcpy(word[9],  "exit      ");
    strcpy(word[10],  "false     ");
    strcpy(word[11], "if        ");
    strcpy(word[12], "integer   ");
    strcpy(word[13], "mod       ");
    strcpy(word[14], "not       ");
    strcpy(word[15], "odd       ");
    strcpy(word[16], "or        ");
    strcpy(word[17], "procedure ");
    strcpy(word[18], "read      ");
    strcpy(word[19], "real      ");
    strcpy(word[20], "then      ");
    strcpy(word[21], "true      ");
    strcpy(word[22], "var       ");
    strcpy(word[23], "while     ");
    strcpy(word[24], "write     ");

    wsym[0]=andsym;
    wsym[1]=beginsym;
    wsym[2]=boolsym;
    wsym[3]=callsym;
    wsym[4]=constsym;
    wsym[5]=divsym;
    wsym[6]=dosym;
    wsym[7]=elsesym;
    wsym[8]=endsym;
    wsym[9]=exitsym;
    wsym[10]=falsesym;
    wsym[11]=ifsym;
    wsym[12]=intsym;
    wsym[13]=modsym;
    wsym[14]=notsym;
    wsym[15]=oddsym;
    wsym[16]=orsym;
    wsym[17]=procsym;
    wsym[18]=readsym;
    wsym[19]=realsym;
    wsym[20]=thensym;
    wsym[21]=truesym;
    wsym[22]=varsym;
    wsym[23]=whilesym;
    wsym[24]=writesym;

    ssym['+']=plus;
    ssym['-']=minus;
    ssym['*']=times;
    ssym['/']=slash;
    ssym['(']=lparen;
    ssym[')']=rparen;
    ssym['=']=eql;
    ssym[',']=comma;
    ssym['.']=period;
    ssym[';']=semicolon;
    ssym[':']=colon;

    strcpy(mnemonic[lit],"LIT");
    strcpy(mnemonic[opr],"OPR");
    strcpy(mnemonic[lod],"LOD");
    strcpy(mnemonic[sto],"STO");
    strcpy(mnemonic[cal],"CAL");
    strcpy(mnemonic[Int],"INT");
    strcpy(mnemonic[jmp],"JMP");
    strcpy(mnemonic[jpc],"JPC");

    declbegsys=constsym|varsym|procsym;
    statbegsys=beginsym|callsym|ifsym|whilesym|readsym|writesym;
    simpexpbegsys=plus|minus|orsym;
    termbegsys=times|slash|andsym|divsym|modsym;
    facbegsys=ident|number|lparen|oddsym|truesym|falsesym|notsym;

    err=0;                       // ���������
    cc=0;
    cx=0;                        // �м����ָ������
    ll=0;
    ch=' ';
    kk=al;
    lev=0;                       // �����㼶����
    tx=0;                        // ���ű�ָ������
    elx=0;                       // exitlist����
    etlx=0;                      // enterlist����
}