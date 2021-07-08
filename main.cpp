#include <cstdlib>

#include "def.h"

char infilename[80];

int main()
{
    long i;

    globalinit();

    //printf("please input source program file name: ");
    //scanf("%s",infilename);
    //printf("\n");

    //if((infile=fopen(infilename,"r"))==NULL)
    if((infile=fopen("C:\\Users\\19146\\Desktop\\test1.pl0","r"))==NULL)
    {
        printf("File %s can't be opened.\n", infilename);
        exit(1);
    }

    getsym();
    block(declbegsys|statbegsys|period);

    if(sym!=period)
    {
        error(9);
    }

    fclose(infile);

    if(err==0)
    {
        listcode(0);               //如果编译没有错误，就先生成中间代码再解释执行
        interpret();
    }
    else
    {
        printf("%3ld errors in PL/0 program\n",err);        //如果编译有错误就输出错误个数
    }

    return (0);
}
