#include <cstdio>

#include "def.h"

void block(unsigned _int64 fsys)
{
    long tx0;       // initial table index
    long cx0;       // initial code index
    long tx1;       // save current table index before processing nested procedures
    long dx1;       // save data allocation index

    dx=3;
    // 地址寄存器给出每层局部量当前已分配到的相对位置
    // 置初始值为 3 的原因是：每一层最开始的位置有三个空间用于存放
    // 静态链 SL、动态链 DL 和 返回地址 RA
    tx0=tx;                      // 记录本层开始时符号表的位置
    table[tx].addr=cx;           // 符号表记下当前层代码的开始地址
    gen(jmp,0,0);                // block开始时首先写下一句跳转指令
                                 // 地址到后面再补

    if(lev>levmax)
    {
        error(32);
    }

    do
    {
        if(sym==constsym)        // 常数定义
        {
            getsym();

            do
            {
                constdeclaration();
                while(sym==comma)
                {
                    getsym(); constdeclaration();
                }

                if(sym==semicolon)
                {
                    getsym();
                }
                else
                {
                    error(5);
                }
            } while(sym==ident);
        }

        if(sym==varsym)
        {
            getsym();
            do
            {
                vardeclaration();
            } while(sym!=beginsym&&sym!=procsym);

        }

        while(sym==procsym)
        {
            getsym();
            if(sym==ident)
            {
                enter(proc); getsym();
            }
            else
            {
                error(4);
            }

            if(sym==semicolon)
            {
                getsym();
            }
            else
            {
                error(5);
            }

            lev=lev+1; tx1=tx; dx1=dx;
            block(fsys|semicolon);
            lev=lev-1; tx=tx1; dx=dx1;

            if(sym==semicolon)
            {
                getsym();
                test(statbegsys|ident|procsym,fsys,6);
            }
            else
            {
                error(5);
            }
        }

        test(statbegsys|ident,declbegsys,7);
    } while(sym&declbegsys);

    if (sym==beginsym)
    {
        code[table[tx0].addr].a=cx;// 把block开头写下的跳转指令的地址补上
        table[tx0].addr=cx;        // tx0的符号表存的是当前block的参数
        cx0=cx;
        gen(Int,0,dx);

        getsym();
        statement(fsys|semicolon|endsym);
        while(sym==semicolon||(sym&statbegsys))
        {
            if(sym==semicolon) getsym();
            else error(10);

            statement(fsys|semicolon|endsym);
        }

        if(sym==endsym) getsym();
        else error(17);

        gen(opr,0,0);            // block结束，加上一句返回指令
        test(fsys,0,8);
        //listcode(cx0);
    } else error(37);
}

void constdeclaration()
{
    if(sym == ident)
    {
        getsym();

        if(sym == eql || sym == becomes)
        {
            if(sym == becomes)
            {
                error(1);
            }

            getsym();

            if(sym == number)
            {
                enter(constant);
                getsym();
            }
            else
            {
                error(2);
            }
        }
        else
        {
            error(3);
        }
    }
    else
    {
        error(4);
    }
}

void vardeclaration()
{
    if(sym == ident)
    {
        enter(variable);
        enterlist[etlx]=tx;
        etlx++;
        getsym();
    } else error(4);

    while(sym==comma)
    {
        getsym();
        if(sym == ident)
        {
            enter(variable);
            enterlist[etlx]=tx;
            etlx++;
            getsym();
        } else error(4);
    }

    if (sym==colon)
    {
        getsym();
        enum object temp;
        switch(sym)
        {
            case intsym:
                temp=integer;
                break;
            case realsym:
                temp=real;
                break;
            case boolsym:
                temp=boolean;
                break;
            default:
                error(38);
        }
        do
        {
            etlx--;
            table[enterlist[etlx]].kind = temp;
        } while(etlx>0);
        getsym();
    } else error(39);

    if (sym==semicolon) getsym();
    else error(10);
}

void statement(unsigned _int64 fsys)
{
    long i,cx1,cx2;

    switch(sym)
    {
        case ident:              // 以标识符开始，则为赋值语句
        {
            i=position(id);
            if(i==0) error(11);
            else if(  table[i].kind!=integer&&table[i].kind!=real&&
            table[i].kind!=boolean)// 非变量
            {
                error(12);
                i=0;
            }

            getsym();

            if(sym==becomes) getsym();
            else error(13);

            simpexpression(fsys);

            if(i!=0)
            {
                if (table[i].kind==boolean) gen(opr,0,21);
                                 // 对boolean变量进行数值整理
                gen(sto,lev-table[i].level,table[i].addr);
            }
        } break;
        case callsym:            // 调用语句
        {
            getsym();
            if(sym!=ident)
            {
                error(14);
            }
            else
            {
                i=position(id);
                if(i==0)
                {
                    error(11);
                }
                else if(table[i].kind==proc)
                {
                    gen(cal,lev-table[i].level,table[i].addr);
                }
                else
                {
                    error(15);
                }

                getsym();
            }
        } break;
        case ifsym:              // if语句
        {
            getsym();
            expression(fsys|thensym|dosym);

            if(sym==thensym)
            {
                getsym();
            }
            else
            {
                error(16);
            }
            cx1=cx;              // 记录下跳转代码的位置，此时跳转地址是0
            gen(jpc,0,0);
            statement(fsys|elsesym);// 紧接着可能是else
            code[cx1].a=cx;      // 把跳转地址加上

            if (sym==elsesym)    // else子句
            {
                cx1=cx;          // 记录下跳转代码的位置
                gen(jmp,0,0);    // 如果有else，那么then后面部分执行完毕后要跳过这段
                getsym();
                statement(fsys);
                code[cx1].a=cx;
            }
        } break;
        case beginsym:           // begin语句
        {
            getsym();
            statement(fsys|semicolon|endsym);
            while(sym==semicolon||(sym&statbegsys))
            {
                if(sym==semicolon)
                {
                    getsym();
                }
                else
                {
                    error(10);
                }
                statement(fsys|semicolon|endsym);
            }
            if(sym==endsym) getsym();
            else error(17);
        } break;
        case whilesym:           // while语句
        {
            cx1=cx; getsym();
            expression(fsys|dosym);

            cx2=elx;                 // 记录下当前层开始的elx
            exitlist[elx]=cx;        // 将jpc的位置记录进exitlist
            elx++;

            gen(jpc,0,0);
            if(sym==dosym)
            {
                getsym();
            }
            else
            {
                error(18);
            }

            statement(fsys|exitsym);
            gen(jmp,0,cx1);

            while(elx>cx2)           // 将exitlist中记录下的跳转语句的地址补充完整
            {
                elx--;
                code[exitlist[elx]].a=cx;
            }
        } break;
        case elsesym:            // else语句
        {
            test(fsys,0,33);     // 具体的处理已经在if中完成了，这里只要判错即可
        } break;
        case exitsym:            // exit语句
        {
            test(fsys,0,34);     // exit语句报错处理

            exitlist[elx]=cx;    // 将jpc的位置记录进exitlist
            elx++;
            gen(jmp,0,0);

            getsym();
        } break;
        case readsym:            // read语句
        {
            getsym();
            if (sym==lparen) getsym();
            else error(35);

            readata();

            while(sym==comma)
            {
                getsym();
                readata();
            }

            if (sym==rparen) getsym();
            else error(22);
        } break;
        case writesym:           // write语句
        {
            getsym();
            if (sym==lparen) getsym();
            else error(35);

            expression(fsys|comma|rparen);
            gen(opr,0,14);

            while(sym==comma)
            {
                getsym();
                expression(fsys|comma|rparen);
                gen(opr,0,14);
            }

            if (sym==rparen) getsym();
            else error(22);
        } break;
    }

    test(fsys,0,19);
}

void expression(unsigned _int64 fsys)
{
    unsigned long relop;

    simpexpression(fsys|eql|neq|lss|gtr|leq|geq);

    if(sym&(eql|neq|lss|gtr|leq|geq))
    {
        relop=sym; getsym();

        simpexpression(fsys);

        switch(relop)
        {
            case eql:
                gen(opr, 0, 8);
                break;

            case neq:
                gen(opr, 0, 9);
                break;

            case lss:
                gen(opr, 0, 10);
                break;

            case geq:
                gen(opr, 0, 11);
                break;

            case gtr:
                gen(opr, 0, 12);
                break;

            case leq:
                gen(opr, 0, 13);
                break;
        }
    }
}

void simpexpression(unsigned _int64 fsys)
{
    unsigned _int64 addop;
    long elx1;

    if(sym==plus || sym==minus)  // 处理开头的正负号
    {
        addop=sym; getsym();

        term(fsys|simpexpbegsys);

        if(addop==minus)
        {
            gen(opr,0,1);
        }
    }
    else term(fsys|simpexpbegsys);

    elx1=elx;                    // 记录下当前层开始的elx

    while(sym & simpexpbegsys)
    {
        addop=sym;

        if(sym==orsym)           // 短路计算，or前为1则不计算右边的term
        {
            gen(opr,0,16);       // 取反，如果是1则会变为0
            exitlist[elx]=cx;    // 将jpc的位置记录进exitlist
            elx++;               // 可能会有多个or
            gen(jpc,0,0);        // 为0则跳转，地址先空着
            gen(opr,0,16);       // 若没跳则要把前面取反的改回来，以免影响正常运算
                                 // 其实这已经确定了原本计算结果是0了
                                 // 也可以用lit(0,0);
        }

        getsym();
        term(fsys|simpexpbegsys);

        switch(addop)
        {
            case plus:
                gen(opr,0,2);
                break;
            case minus:
                gen(opr,0,3);
                break;
            case orsym:
                gen(opr,0,18);
                break;
        }
    }

    if (elx>elx1)                // 语句中存在or短路跳转
    {
        gen(jmp,0,cx+2);         // 若是正常执行则跳过下一句指令
        while(elx>elx1)          // 补上之前的jpc地址
        {
            elx--;
            code[exitlist[elx]].a=cx;
        }
        gen(lit,0,1);            // 短路跳转则把结果强制置为1
    }
}

void term(unsigned _int64 fsys)
{
    unsigned _int64 mulop;
    long elx1;

    factor(fsys|termbegsys);

    elx1=elx;                    // 记录下当前层开始的elx

    while(sym & termbegsys)
    {
        mulop = sym;

        if (sym==andsym)         // 短路计算，and前为0则不计算右边的factor
        {
            exitlist[elx]=cx;    // 将jpc的位置记录进exitlist
            elx++;               // 可能会有多个or
            gen(jpc,0,0);        // 为0则跳转，地址先空着
        }

        getsym();

        factor(fsys|termbegsys);

        switch(mulop)
        {
            case times:
                gen(opr,0,4);
                break;
            case slash:
                gen(opr,0,5);
                break;
            case andsym:
                gen(opr,0,17);
                break;
            case divsym:
                gen(opr,0,19);
                break;
            case modsym:
                gen(opr,0,20);
                break;
        }
    }

    if (elx>elx1)                // 语句中存在and短路跳转
    {
        gen(jmp,0,cx+2);         // 若是正常执行则跳过下一句指令
        while(elx>elx1)          // 补上之前的jpc地址
        {
            elx--;
            code[exitlist[elx]].a=cx;
        }
        gen(lit,0,0);            // 短路跳转把结果强制置为0
    }
}

void factor(unsigned _int64 fsys)
{
    long i;

    test(facbegsys, fsys, 24);

    while(sym & facbegsys)
    {
        switch(sym)
        {
            case ident:
            {
                i = position(id);

                if(i==0) error(11);
                else
                {
                    switch(table[i].kind)
                    {
                        case constant:
                            gen(lit, 0, table[i].val);
                            break;

                        case integer:
                        case boolean:
                            gen(lod, lev-table[i].level, table[i].addr);
                            break;

                        case proc:
                            error(21);
                            break;
                    }
                }

                getsym();
            } break;
            case number:
            {
                if(num>amax)
                {
                    error(31);
                    num=0;
                }

                gen(lit,0,num);
                getsym();
            } break;
            case lparen:
            {
                getsym();
                expression(rparen|fsys);

                if(sym==rparen) getsym();
                else error(22);
            } break;
            case oddsym:
            {
                getsym();

                if(sym == lparen) getsym();
                else error(35);

                simpexpression(fsys|rparen);
                gen(opr, 0, 6);

                if(sym == rparen) getsym();
                else error(22);
            } break;
            case falsesym:
            {
                gen(lit,0,0);
                getsym();
            } break;
            case truesym:
            {
                gen(lit,0,1);
                getsym();
            } break;
            case notsym:
            {
                getsym();

                factor(fsys);
                gen(opr,0,16);

            } break;
        }

        test(fsys,lparen,23);
    }
}

void readata()
{
    if (sym==ident)
    {
        long i=position(id);
        if(i==0) error(11);
        else if(table[i].kind!=integer&&table[i].kind!=real&&table[i].kind!=boolean)
        {
            error(12); i=0;
        }

        gen(opr,0,15);
        if(i!=0)
        {
            if (table[i].kind==boolean) gen(opr,0,21);
            gen(sto,lev-table[i].level,table[i].addr);
        }

        getsym();
    } else error(36);
}
