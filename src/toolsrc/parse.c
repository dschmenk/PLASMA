#include <stdio.h>
#include <string.h>
#include "plasm.h"
#define LVALUE      0
#define RVALUE      1
#define MAX_LAMBDA  64

int infunc = 0, break_tag = 0, cont_tag = 0, stack_loop = 0;
long infuncvals = 0;
t_token prevstmnt;
static int      lambda_num = 0;
static int      lambda_cnt = 0;
static t_opseq *lambda_seq[MAX_LAMBDA];
static char     lambda_id[MAX_LAMBDA][16];
static int      lambda_tag[MAX_LAMBDA];
static int      lambda_cparams[MAX_LAMBDA];
t_token binary_ops_table[] = {
    /* Highest precedence */
    MUL_TOKEN, DIV_TOKEN, MOD_TOKEN,
    ADD_TOKEN, SUB_TOKEN,
    SHR_TOKEN, SHL_TOKEN,
    AND_TOKEN,
    EOR_TOKEN,
    OR_TOKEN,
    GT_TOKEN, GE_TOKEN, LT_TOKEN, LE_TOKEN,
    EQ_TOKEN, NE_TOKEN,
    LOGIC_AND_TOKEN,
    LOGIC_OR_TOKEN
    /* Lowest precedence  */
};
t_token binary_ops_precedence[] = {
    /* Highest precedence */
    1, 1, 1,
    2, 2,
    3, 3,
    4,
    5,
    6,
    7, 7, 7, 7,
    8, 8,
    9,
    10
    /* Lowest precedence  */
};

t_token opstack[16];
int precstack[16];
int opsptr = -1;
void push_op(t_token op, int prec)
{
    if (++opsptr == 16)
        parse_error("Stack overflow");
    opstack[opsptr]   = op;
    precstack[opsptr] = prec;
}
t_token pop_op(void)
{
    if (opsptr < 0)
        parse_error("Stack underflow");
    return opstack[opsptr--];
}
t_token tos_op(void)
{
    return opsptr < 0 ? 0 : opstack[opsptr];
}
int tos_op_prec(int tos)
{
    return opsptr <= tos ? 100 : precstack[opsptr];
}
long valstack[16];
int typestack[16];
int sizestack[16];
int valptr = -1;
void push_val(long value, int size, int type)
{
    if (++valptr == 16)
        parse_error("Stack overflow");
    valstack[valptr]  = value;
    sizestack[valptr] = size;
    typestack[valptr] = type;
}
int pop_val(long *value, int *size, int *type)
{
    if (valptr < 0)
        parse_error("Stack underflow");
    *value = valstack[valptr];
    *size  = sizestack[valptr];
    *type  = typestack[valptr];
    return valptr--;
}
/*
 * Constant expression parsing
 */
int calc_op(t_token op)
{
    long val1, val2;
    int size1, size2, type1, type2;
    if (!pop_val(&val2, &size2, &type2))
        return 0;
    pop_val(&val1, &size1, &type1);
    if (type1 != CONST_TYPE || type2 != CONST_TYPE)
        parse_error("Bad constant operand");
    switch (op)
    {
        case MUL_TOKEN:
            val1 *= val2;
            break;
        case DIV_TOKEN:
            val1 /= val2;
            break;
        case MOD_TOKEN:
            val1 %= val2;
            break;
        case ADD_TOKEN:
            val1 += val2;
            break;
        case SUB_TOKEN:
            val1 -= val2;
            break;
        case SHL_TOKEN:
            val1 <<= val2;
            break;
        case SHR_TOKEN:
            val1 >>= val2;
            break;
        case AND_TOKEN:
            val1 &= val2;
            break;
        case OR_TOKEN:
            val1 |= val2;
            break;
        case EOR_TOKEN:
            val1 ^= val2;
            break;
        default:
            return (0);
    }
    size1 = size1 > size2 ? size1 : size2;
    push_val(val1, size1, type1);
    return (1);
}
int parse_constexpr(long *value, int *size);
int parse_constterm(long *value, int *size)
{
    int type;
    /*
     * Parse terminal tokens.
     */
    switch (type = scan())
    {
        case CHAR_TOKEN:
        case INT_TOKEN:
        case ID_TOKEN:
        case STRING_TOKEN:
            break;
        case OPEN_PAREN_TOKEN:
            if (!(type = parse_constexpr(value, size)))
                parse_error("Bad expression in parenthesis");
            if (scantoken != CLOSE_PAREN_TOKEN)
                parse_error("Missing closing parenthesis");
            break;
        default:
            /*
             * Non-terminal token.
             */
            return (0);
    }
    return (type);
}
int parse_constval(void)
{
    int mod = 0, type, size;
    long value;

    value = 0;
    size  = 1;
    while (!(type = parse_constterm(&value, &size)))
    {
        switch (scantoken)
        {
            case ADD_TOKEN:
                /*
                 * Just ignore unary plus, it is a no-op.
                 */
                break;
            case NEG_TOKEN:
                mod |= 1;
                break;
            case COMP_TOKEN:
                mod |= 2;
                break;
            case LOGIC_NOT_TOKEN:
                mod |= 4;
                break;
            case AT_TOKEN:
                mod |= 8;
                break;
            default:
                return (0);
        }
    }
    /*
     * Determine which terminal type.
     */
    switch (scantoken)
    {
        case CLOSE_PAREN_TOKEN:
            break;
        case STRING_TOKEN:
            size  = 1;
            value = constval;
            type  = STRING_TYPE;
            if (mod)
                parse_error("Invalid string modifiers");
            break;
        case CHAR_TOKEN:
            size  = 1;
            value = constval;
            type  = CONST_TYPE;
            break;
        case INT_TOKEN:
            size  = 2;
            value = constval;
            type  = CONST_TYPE;
            break;
        case ID_TOKEN:
            size = 2;
            type = id_type(tokenstr, tokenlen);
            if (type & CONST_TYPE)
                value = id_const(tokenstr, tokenlen);
            else if (type & (FUNC_TYPE | ADDR_TYPE))
            {
                if (mod != 8)
                    parse_error("Invalid address constant");
                value = id_tag(tokenstr, tokenlen);
            }
            else
                return (0);
            break;
        default:
            return (0);
    }
    if (mod & 1)
        value = -value;
    if (mod & 2)
        value = ~value;
    if (mod & 4)
        value = value ? 0 : -1;
    push_val(value, size, type);
    return (type);
}
int parse_constexpr(long *value, int *size)
{
    int prevmatch;
    int matchop = 0;
    int optos = opsptr;
    int i;
    int type = CONST_TYPE;
    *value = 0;
    *size  = 1;
    do
    {
        /*
         * Parse sequence of double operand operations.
         */
        prevmatch = matchop;
        matchop   = 0;
        if (parse_constval())
        {
            matchop = 1;
            scan();
            for (i = 0; i < sizeof(binary_ops_table); i++)
                if (scantoken == binary_ops_table[i])
                {
                    matchop = 2;
                    if (binary_ops_precedence[i] >= tos_op_prec(optos))
                        if (!calc_op(pop_op()))
                            parse_error("Invalid binary operation");
                    push_op(scantoken, binary_ops_precedence[i]);
                    break;
                }
        }
    } while (matchop == 2);
    if (matchop == 0 && prevmatch == 0)
        return (0);
    if (matchop == 0 && prevmatch == 2)
        parse_error("Missing operand");
    while (optos < opsptr)
        if (!calc_op(pop_op()))
            parse_error("Invalid binary operation");
    pop_val(value, size, &type);
    return (type);
}
int parse_const(long *value)
{
    /*
     * Get simple constant.
     */
    switch (scan())
    {
        case CHAR_TOKEN:
        case INT_TOKEN:
            *value = constval;
            break;
        case ID_TOKEN:
            if (id_type(tokenstr, tokenlen) & CONST_TYPE)
            {
                *value = id_const(tokenstr, tokenlen);
                break;
            }
        default:
            *value = 0;
            return (0);
    }
    return (CONST_TYPE);
}
/*
 * Normal expression parsing
 */
int parse_lambda(void);
t_opseq *parse_expr(t_opseq *codeseq, int *stackdepth);
t_opseq *parse_list(t_opseq *codeseq, int *stackdepth)
{
    int parmdepth;
    t_opseq *parmseq;
    if (stackdepth)
        *stackdepth = 0;
    while ((parmseq = parse_expr(codeseq, &parmdepth)))
    {
        codeseq = parmseq;
        if (stackdepth)
            *stackdepth += parmdepth;
        if (scantoken != COMMA_TOKEN)
            break;
    }
    return (codeseq);
}
t_opseq *parse_value(t_opseq *codeseq, int rvalue, int *stackdepth)
{
    int deref = rvalue;
    int type = 0, value = 0;
    int cfnparms = 0;
    long cfnvals = 1;
    long const_offset;
    t_opseq *uopseq = NULL;
    t_opseq *valseq = NULL;
    t_opseq *idxseq = NULL;

    if (stackdepth)
        *stackdepth = 1;
    /*
     * Parse pre operators.
     */
    while (scan())
    {
        if (scantoken == ADD_TOKEN)
        {
            /*
             * Just ignore unary plus, it is a no-op.
             */
        }
        else if (scantoken == AT_TOKEN)
        {
            if (deref-- == 0)
                parse_error("Invalid ADDRESS-OF op");
        }
        else if (scantoken == BPTR_TOKEN || scantoken == WPTR_TOKEN)
        {
            if (type & BPTR_TYPE)
                parse_error("Byte value used as pointer");
            else
                type = scantoken == BPTR_TOKEN ? BPTR_TYPE : WPTR_TYPE;
            deref++;
        }
        else if (scantoken == NEG_TOKEN || scantoken == COMP_TOKEN || scantoken == LOGIC_NOT_TOKEN)
        {
            if (!rvalue)
                parse_error("Invalid op for LVALUE");
            uopseq = gen_uop(uopseq, scantoken);
        }
        else
            break;
    }
    /*
     * Determine which value type.
     */
    if (scantoken == STRING_TOKEN)
    {
        /*
         * This is a special case. Just emit the string and return
         */
        codeseq = gen_str(codeseq, constval);
        scan();
        return (codeseq);
    }
    if (scantoken == INT_TOKEN || scantoken == CHAR_TOKEN)
    {
        value = constval;
        valseq = gen_const(NULL, value);
        deref--;
    }
    else if (scantoken == ID_TOKEN)
    {
        if ((type |= id_type(tokenstr, tokenlen)) & CONST_TYPE)
        {
            value  = id_const(tokenstr, tokenlen);
            valseq = gen_const(NULL, value);
            deref--;
        }
        else
        {
            value = id_tag(tokenstr, tokenlen);
            if (type & LOCAL_TYPE)
                valseq = gen_lcladr(NULL, value);
            else
                valseq = gen_gbladr(NULL, value, type);
            if (type & FUNC_TYPE)
            {
                cfnparms = funcparms_cnt(type);
                cfnvals  = funcvals_cnt(type);
            }
        }
    }
    else if (scantoken == LAMBDA_TOKEN)
    {
        if (!rvalue) // Lambdas can't be LVALUEs
        {
            release_seq(uopseq);
            return (codeseq);
        }
        value  = parse_lambda();
        valseq = gen_gbladr(NULL, value, FUNC_TYPE);
        deref--;
    }
    else if (scantoken == OPEN_PAREN_TOKEN)
    {
        if (!(valseq = parse_expr(NULL, stackdepth)))
            parse_error("Bad expression in parenthesis");
        if (scantoken != CLOSE_PAREN_TOKEN)
            parse_error("Missing closing parenthesis");
        deref--;
    }
    else if (scantoken == DROP_TOKEN)
    {
        if (rvalue)
            parse_error("DROP is LVALUE only");
        codeseq = gen_drop(codeseq);
        scan();
        return (codeseq);
    }
    else
    {
        release_seq(uopseq);
        release_seq(codeseq);
        return (NULL);
    }
    /*
     * Parse post operators.
     */
    while (scan())
    {
        if (scantoken == OPEN_PAREN_TOKEN)
        {
            /*
             * Function call - parameters generate before call address
             */
            valseq = cat_seq(parse_list(NULL, &value), valseq);
            if (scantoken != CLOSE_PAREN_TOKEN)
                parse_error("Missing function call closing parenthesis");
            if (type & FUNC_TYPE)
            {
                if (cfnparms != value) // Can't check parm count on function pointers
                    parse_error("Parameter count mismatch");
            }
            else
            {
                if (scan() == POUND_TOKEN)
                {
                    /*
                     * Set function pointer return vals count - can't do this to regular function call
                     */
                    if (!parse_const(&cfnvals))
                        parse_error("Invalid def return value count");
                }
                else
                    scan_rewind(tokenstr);
                if (type & WORD_TYPE)
                    valseq = gen_lw(valseq);
                else if (type & BYTE_TYPE)
                    parse_error("Using BYTE value as a pointer");
                else
                    deref++;
            }
            valseq = gen_icall(valseq);
            if (stackdepth)
                *stackdepth += cfnvals - 1;
            cfnparms = 0; cfnvals = 1;
            type &= PTR_TYPE;
            deref--;
        }
        else if (scantoken == OPEN_BRACKET_TOKEN)
        {
            /*
             * Array of arrays
             */
            if (type & FUNC_TYPE)
            {
                /*
                 * Function address dereference
                 */
                cfnparms = 0; cfnvals = 1;
            }
            while ((valseq = parse_expr(valseq, stackdepth)) && scantoken == COMMA_TOKEN)
            {
                valseq = gen_idxw(valseq);
                valseq = gen_lw(valseq); // Multi-dimenstion arrays are array pointers to arrays
            }
            if (scantoken != CLOSE_BRACKET_TOKEN)
                parse_error("Missing closing bracket");
            if (type & WORD_TYPE)
            {
                valseq = gen_idxw(valseq);
            }
            else
            {
                valseq = gen_idxb(valseq);
                if (!(type & BYTE_TYPE))
                {
                    type = (type & PTR_TYPE) | BYTE_TYPE;
                    deref++;
                }
            }
        }
        else if (scantoken == PTRB_TOKEN || scantoken == PTRW_TOKEN)
        {
            /*
             * Pointer to structure/array
             */
            if (type & FUNC_TYPE)
            {
                /*
                 * Function call dereference
                 */
                if (cfnparms)
                    parse_error("Parameter count mismatch");
                valseq = gen_icall(valseq);
                if (stackdepth)
                    *stackdepth += cfnvals - 1;
                cfnparms = 0; cfnvals = 1;
            }
            else if (type & WORD_TYPE)
            {
                /*
                 * Pointer dereference
                 */
                valseq = gen_lw(valseq);
            }
            else if (type & BYTE_TYPE)
                parse_error("Using BYTE value as a pointer");
            else
                deref++;
            type = (type & PTR_TYPE) | (scantoken == PTRB_TOKEN) ? BYTE_TYPE : WORD_TYPE; // Type override
            if (!parse_const(&const_offset))
            {
                /*
                 * Setting type override for following operations
                 */
                scan_rewind(tokenstr);
            }
            else if (const_offset != 0)
            {
                /*
                 * Structure member pointer
                 */
                valseq = gen_const(valseq, const_offset);
                valseq = gen_op(valseq, ADD_TOKEN);
            }
        }
        else if (scantoken == DOT_TOKEN || scantoken == COLON_TOKEN)
        {
            /*
             * Structure/array offset
             */
            if (type & FUNC_TYPE)
            {
                /*
                 * Function address dereference
                 */
                cfnparms = 0; cfnvals = 1;
            }
            else if (!(type & VAR_TYPE))
                deref++;
            type = (type & PTR_TYPE) | ((scantoken == DOT_TOKEN) ? BYTE_TYPE : WORD_TYPE); // Type override
            if (!parse_const(&const_offset))
            {
                /*
                 * Setting type override for following operations
                 */
                scan_rewind(tokenstr);
            }
            else if (const_offset != 0)
            {
                /*
                 * Structure member offset
                 */
                valseq = gen_const(valseq, const_offset);
                valseq = gen_op(valseq, ADD_TOKEN);
            }
        }
        else
            break;
    }
    /*
     * Probably parsing RVALUE as LVALUE
     */
    if (deref < 0)
    {
        release_seq(valseq);
        release_seq(uopseq);
        return (NULL);
    }
    /*
     * Resolve outstanding dereference pointer loads
     */
    while (deref > rvalue)
    {
        if (type & FUNC_TYPE)
        {
            if (cfnparms)
                parse_error("Parameter count mismatch");
            valseq = gen_icall(valseq);
            if (stackdepth)
                *stackdepth += cfnvals - 1;
            cfnparms = 0; cfnvals = 1;
            type &= ~FUNC_TYPE;
        }
        else //if (type & VAR_TYPE)
            valseq = gen_lw(valseq);
        //else
        //    {fprintf(stderr,"deref=%d",deref);parse_error("What are we dereferencing #1?");}
        deref--;
    }
    if (deref)
    {
        if (type & FUNC_TYPE)
        {
            if (cfnparms)
                parse_error("Parameter count mismatch");
            valseq = gen_icall(valseq);
            if (stackdepth)
                *stackdepth += cfnvals - 1;
            cfnparms = 0; cfnvals = 1;
            type &= ~FUNC_TYPE;
        }
        else if (type & (BYTE_TYPE | BPTR_TYPE))
            valseq = gen_lb(valseq);
        else if (type & (WORD_TYPE | WPTR_TYPE))
            valseq = gen_lw(valseq);
        else
            parse_error("What are we dereferencing?");
    }
    /*
     * Output pre-operations
     */
     valseq = cat_seq(valseq, uopseq);
    /*
     * Wrap up LVALUE store
     */
    if (!rvalue)
    {
        if (type & (BYTE_TYPE | BPTR_TYPE))
            valseq = gen_sb(valseq);
        else if (type & (WORD_TYPE | WPTR_TYPE))
            valseq = gen_sw(valseq);
        else
        {
            release_seq(valseq);
            return (NULL); // Function or const cannot be LVALUE, must be RVALUE
        }
        if (stackdepth)
            (*stackdepth)--;
    }
    return (cat_seq(codeseq, valseq));
}
t_opseq *parse_expr(t_opseq *codeseq, int *stackdepth)
{
    int prevmatch;
    int matchop = 0;
    int optos = opsptr;
    int i, valdepth;
    int prevtype, type = 0;
    t_opseq *valseq;

    if (stackdepth)
        *stackdepth = 0;
    do
    {
        /*
         * Parse sequence of double operand operations.
         */
        prevmatch = matchop;
        matchop   = 0;
        if ((valseq = parse_value(NULL, RVALUE, &valdepth)))
        {
            codeseq = cat_seq(codeseq, valseq);
            matchop = 1;
            if (stackdepth)
                *stackdepth += valdepth;
            for (i = 0; i < sizeof(binary_ops_table); i++)
                if (scantoken == binary_ops_table[i])
                {
                    matchop = 2;
                    if (binary_ops_precedence[i] >= tos_op_prec(optos))
                    {
                        codeseq = gen_op(codeseq, pop_op());
                        if (stackdepth)
                            (*stackdepth)--;
                    }
                    push_op(scantoken, binary_ops_precedence[i]);
                    break;
                }
        }
    } while (matchop == 2);
    if (matchop == 0 && prevmatch == 2)
        parse_error("Missing operand");
    while (optos < opsptr)
    {
        codeseq = gen_op(codeseq, pop_op());
        if (stackdepth)
            (*stackdepth)--;
    }
    /*
     * Look for ternary operator
     */
    if (scantoken == TERNARY_TOKEN)
    {
        int tag_else, tag_endtri;
        int stackdepth1;

        if (*stackdepth != 1)
            parse_error("Ternary op must evaluate to single value");
        tag_else   = tag_new(BRANCH_TYPE);
        tag_endtri = tag_new(BRANCH_TYPE);
        codeseq    = gen_brfls(codeseq, tag_else);
        codeseq    = parse_expr(codeseq, &stackdepth1);
        if (scantoken != TRIELSE_TOKEN)
            parse_error("Missing '::' in ternary op");
        codeseq = gen_brnch(codeseq, tag_endtri);
        codeseq = gen_codetag(codeseq, tag_else);
        codeseq = parse_expr(codeseq, stackdepth);
        if (stackdepth1 != *stackdepth)
            parse_error("Inconsistent value counts in ternary op");
        codeseq = gen_codetag(codeseq, tag_endtri);
    }
    return (codeseq);
}
t_opseq *parse_set(t_opseq *codeseq)
{
    char *setptr = tokenstr;
    int lparms = 0, rparms = 0;
    int i;
    int lambda_set = lambda_cnt;
    t_opseq *setseq[16], *rseq = NULL;

    while ((setseq[lparms] = parse_value(NULL, LVALUE, NULL)))
    {
        lparms++;
        if (scantoken != COMMA_TOKEN)
            break;
    }
    if (lparms == 0 || scantoken != SET_TOKEN)
    {
        scan_rewind(setptr);
        while (lparms--)
            release_seq(setseq[lparms]);
        while (lambda_cnt > lambda_set)
        {
            lambda_cnt--;
            lambda_num--;
            release_seq(lambda_seq[lambda_cnt]);
        }
        return (NULL);
    }
    rseq = parse_list(NULL, &rparms);
    if (lparms > rparms)
        parse_error("Set value list underflow");
    codeseq = cat_seq(codeseq, rseq);
    if (lparms < rparms)
    {
        parse_warn("Silently dropping extra rvalues");
        for (i = rparms - lparms; i > 0; i--)
            codeseq = gen_drop(codeseq);
    }
    while (lparms--)
        codeseq = cat_seq(codeseq, setseq[lparms]);
    return (codeseq);
}
int parse_stmnt(void)
{
    int tag_prevbrk, tag_prevcnt, tag_else, tag_endif, tag_while, tag_wend, tag_repeat, tag_for, tag_choice, tag_of;
    int type, addr, step, cfnvals;
    char *idptr;
    t_opseq *seq;

    /*
     * Optimization for last function LEAVE and OF clause.
     */
    if (scantoken != END_TOKEN && scantoken != DONE_TOKEN && scantoken != OF_TOKEN && scantoken != DEFAULT_TOKEN)
        prevstmnt = scantoken;
    switch (scantoken)
    {
        case IF_TOKEN:
            if (!(seq = parse_expr(NULL, &cfnvals)))
                parse_error("Bad expression");
            if (cfnvals > 1)
            {
                parse_warn("Expression value overflow");
                while (cfnvals-- > 1) seq = gen_drop(seq);
            }
            tag_else  = tag_new(BRANCH_TYPE);
            tag_endif = tag_new(BRANCH_TYPE);
            seq = gen_brfls(seq, tag_else);
            emit_seq(seq);
            do
            {
                while (parse_stmnt()) next_line();
                if (scantoken != ELSEIF_TOKEN)
                    break;
                emit_brnch(tag_endif);
                emit_codetag(tag_else);
                if (!(seq = parse_expr(NULL, &cfnvals)))
                    parse_error("Bad expression");
                if (cfnvals > 1)
                {
                    parse_warn("Expression value overflow");
                    while (cfnvals-- > 1) seq = gen_drop(seq);
                }
                tag_else = tag_new(BRANCH_TYPE);
                seq = gen_brfls(seq, tag_else);
                emit_seq(seq);
            } while (1);
            if (scantoken == ELSE_TOKEN)
            {
                emit_brnch(tag_endif);
                emit_codetag(tag_else);
                scan();
                while (parse_stmnt()) next_line();
                emit_codetag(tag_endif);
            }
            else
            {
                emit_codetag(tag_else);
                emit_codetag(tag_endif);
            }
            if (scantoken != FIN_TOKEN)
                parse_error("Missing IF/FIN");
            break;
        case WHILE_TOKEN:
            tag_while   = tag_new(BRANCH_TYPE);
            tag_wend    = tag_new(BRANCH_TYPE);
            tag_prevcnt = cont_tag;
            cont_tag    = tag_while;
            tag_prevbrk = break_tag;
            break_tag   = tag_wend;
            emit_codetag(tag_while);
            if (!(seq = parse_expr(NULL, &cfnvals)))
                parse_error("Bad expression");
            if (cfnvals > 1)
            {
                parse_warn("Expression value overflow");
                while (cfnvals-- > 1) seq = gen_drop(seq);
            }
            seq = gen_brfls(seq, tag_wend);
            emit_seq(seq);
            while (parse_stmnt()) next_line();
            if (scantoken != LOOP_TOKEN)
                parse_error("Missing WHILE/END");
            emit_brnch(tag_while);
            emit_codetag(tag_wend);
            break_tag = tag_prevbrk;
            cont_tag  = tag_prevcnt;
            break;
        case REPEAT_TOKEN:
            tag_prevbrk = break_tag;
            break_tag   = tag_new(BRANCH_TYPE);
            tag_repeat  = tag_new(BRANCH_TYPE);
            tag_prevcnt = cont_tag;
            cont_tag    = tag_new(BRANCH_TYPE);
            emit_codetag(tag_repeat);
            scan();
            while (parse_stmnt()) next_line();
            if (scantoken != UNTIL_TOKEN)
                parse_error("Missing REPEAT/UNTIL");
            emit_codetag(cont_tag);
            cont_tag = tag_prevcnt;
            if (!(seq = parse_expr(NULL, &cfnvals)))
                parse_error("Bad expression");
            if (cfnvals > 1)
            {
                parse_warn("Expression value overflow");
                while (cfnvals-- > 1) seq = gen_drop(seq);
            }
            seq = gen_brfls(seq, tag_repeat);
            emit_seq(seq);
            emit_codetag(break_tag);
            break_tag = tag_prevbrk;
            break;
        case FOR_TOKEN:
            stack_loop++;
            tag_prevbrk = break_tag;
            break_tag   = tag_new(BRANCH_TYPE);
            tag_for     = tag_new(BRANCH_TYPE);
            tag_prevcnt = cont_tag;
            cont_tag    = tag_for;
            if (scan() != ID_TOKEN)
                parse_error("Missing FOR variable");
            type = id_type(tokenstr, tokenlen);
            addr = id_tag(tokenstr, tokenlen);
            if (scan() != SET_TOKEN)
                parse_error("Missing FOR =");
            if (!(seq = parse_expr(NULL, &cfnvals)))
                parse_error("Bad FOR expression");
            if (cfnvals > 1)
            {
                parse_warn("Expression value overflow");
                while (cfnvals-- > 1) seq = gen_drop(seq);
            }
            emit_seq(seq);
            emit_codetag(tag_for);
            if (type & LOCAL_TYPE)
                type & BYTE_TYPE ? emit_dlb(addr) : emit_dlw(addr);
            else
                type & BYTE_TYPE ? emit_dab(addr, 0, type) : emit_daw(addr, 0, type);
            if (scantoken == TO_TOKEN)
                step = 1;
            else if (scantoken == DOWNTO_TOKEN)
                step = -1;
            else
                parse_error("Missing FOR TO");
            if (!(seq = parse_expr(NULL, &cfnvals)))
                parse_error("Bad FOR TO expression");
            if (cfnvals > 1)
            {
                parse_warn("Expression value overflow");
                while (cfnvals-- > 1) seq = gen_drop(seq);
            }
            emit_seq(seq);
            step > 0 ? emit_brgt(break_tag) : emit_brlt(break_tag);
            if (scantoken == STEP_TOKEN)
            {
                if (!(seq = parse_expr(NULL, &cfnvals)))
                    parse_error("Bad FOR STEP expression");
                if (cfnvals > 1)
                {
                    parse_warn("Expression value overflow");
                    while (cfnvals-- > 1) seq = gen_drop(seq);
                }
                emit_seq(seq);
                emit_op(step > 0 ? ADD_TOKEN : SUB_TOKEN);
            }
            else
                emit_unaryop(step > 0 ? INC_TOKEN : DEC_TOKEN);
            while (parse_stmnt()) next_line();
            if (scantoken != NEXT_TOKEN)
                parse_error("Missing FOR/NEXT");
            emit_brnch(tag_for);
            cont_tag = tag_prevcnt;
            emit_codetag(break_tag);
            emit_drop();
            break_tag = tag_prevbrk;
            stack_loop--;
            break;
        case CASE_TOKEN:
            stack_loop++;
            tag_prevbrk = break_tag;
            break_tag   = tag_new(BRANCH_TYPE);
            tag_choice  = tag_new(BRANCH_TYPE);
            tag_of      = tag_new(BRANCH_TYPE);
            if (!(seq = parse_expr(NULL, &cfnvals)))
                parse_error("Bad CASE expression");
            if (cfnvals > 1)
            {
                parse_warn("Expression value overflow");
                while (cfnvals-- > 1) seq = gen_drop(seq);
            }
            emit_seq(seq);
            next_line();
            while (scantoken != ENDCASE_TOKEN)
            {
                if (scantoken == OF_TOKEN)
                {
                    if (!(seq = parse_expr(NULL, &cfnvals)))
                        parse_error("Bad CASE OF expression");
                    if (cfnvals > 1)
                    {
                        parse_warn("Expression value overflow");
                        while (cfnvals-- > 1) seq = gen_drop(seq);
                    }
                    emit_seq(seq);
                    emit_brne(tag_choice);
                    emit_codetag(tag_of);
                    while (parse_stmnt()) next_line();
                    tag_of = tag_new(BRANCH_TYPE);
                    if (prevstmnt != BREAK_TOKEN) // Fall through to next OF if no break
                        emit_brnch(tag_of);
                    emit_codetag(tag_choice);
                    tag_choice = tag_new(BRANCH_TYPE);
                }
                else if (scantoken == DEFAULT_TOKEN)
                {
                    emit_codetag(tag_of);
                    tag_of = 0;
                    scan();
                    while (parse_stmnt()) next_line();
                    if (scantoken != ENDCASE_TOKEN)
                        parse_error("Bad CASE DEFAULT clause");
                }
                else if (scantoken == EOL_TOKEN)
                    next_line();
                else
                    parse_error("Bad CASE clause");
            }
            if (tag_of)
                emit_codetag(tag_of);
            emit_codetag(break_tag);
            emit_drop();
            break_tag = tag_prevbrk;
            stack_loop--;
            break;
        case BREAK_TOKEN:
            if (break_tag)
                emit_brnch(break_tag);
            else
                parse_error("BREAK without loop");
            break;
        case CONTINUE_TOKEN:
            if (cont_tag)
                emit_brnch(cont_tag);
            else
                parse_error("CONTINUE without loop");
            break;
        case RETURN_TOKEN:
            if (infunc)
            {
                int i;
                for (i = 0; i < stack_loop; i++)
                    emit_drop();
                cfnvals = 0;
                emit_seq(parse_list(NULL, &cfnvals));
                if (cfnvals > infuncvals)
                    parse_error("Too many return values");
                if (cfnvals < infuncvals)
                {
                    parse_warn("Too few return values");
                    while (cfnvals++ < infuncvals)
                        emit_const(0);
                }
                emit_leave();
            }
            else
            {
                if (!(seq = parse_expr(NULL, &cfnvals)))
                    emit_const(0);
                else
                {
                     if (cfnvals > 1)
                     {
                        parse_warn("Expression value overflow");
                        while (cfnvals-- > 1) seq = gen_drop(seq);
                    }
                    emit_seq(seq);
                }
                emit_ret();
            }
            break;
        case EOL_TOKEN:
            return (1);
        case ELSE_TOKEN:
        case ELSEIF_TOKEN:
        case FIN_TOKEN:
        case LOOP_TOKEN:
        case UNTIL_TOKEN:
        case NEXT_TOKEN:
        case OF_TOKEN:
        case DEFAULT_TOKEN:
        case ENDCASE_TOKEN:
        case END_TOKEN:
        case DONE_TOKEN:
        case DEF_TOKEN:
            return (0);
        default:
            scan_rewind(tokenstr);
            if (!emit_seq(parse_set(NULL)))
            {
                t_opseq *rseq;
                int stackdepth = 0;
                idptr = tokenstr;
                if ((rseq = parse_value(NULL, RVALUE, &stackdepth)))
                {
                    if (scantoken == INC_TOKEN || scantoken == DEC_TOKEN)
                    {
                        emit_seq(rseq);
                        emit_unaryop(scantoken);
                        scan_rewind(idptr);
                        emit_seq(parse_value(NULL, LVALUE, NULL));
                    }
                    else
                    {
                        while (stackdepth)
                        {
                            rseq = cat_seq(rseq, gen_drop(NULL));
                            stackdepth--;
                        }
                        emit_seq(rseq);
                    }
                }
                else
                    parse_error("Syntax error");
            }
    }
    return (scan() == EOL_TOKEN);
}
int parse_var(int type, long basesize)
{
    char *idstr;
    long constval;
    long size = 1;
    int  consttype, constsize, arraysize, idlen = 0;

    if (scan() == ID_TOKEN)
    {
        idstr = tokenstr;
        idlen = tokenlen;
        if (scan() == OPEN_BRACKET_TOKEN)
        {
            size = 0;
            parse_constexpr(&size, &constsize);
            if (scantoken != CLOSE_BRACKET_TOKEN)
                parse_error("Missing closing bracket");
            scan();
        }
    }
    size *= basesize;
    if (scantoken == SET_TOKEN)
    {
        if (type & (EXTERN_TYPE | LOCAL_TYPE))
            parse_error("Cannot initiallize local/external variables");
        if (idlen)
            idglobal_add(idstr, idlen, type, 0);
        if ((consttype = parse_constexpr(&constval, &constsize)))
        {
            /*
             * Variable initialization.
             */
            arraysize = emit_data(type, consttype, constval, constsize);
            while (scantoken == COMMA_TOKEN)
            {
                if ((consttype = parse_constexpr(&constval, &constsize)))
                    arraysize += emit_data(type, consttype, constval, constsize);
                else
                    parse_error("Bad array declaration");
            }
            if (size > arraysize)
                idglobal_size(PTR_TYPE, size, arraysize);
        }
        else
            parse_error("Bad variable initializer");
    }
    else
    {
        if (idlen)
            id_add(idstr, idlen, type, size);
        else
            emit_data(0, 0, 0, size);
    }
    return (1);
}
int parse_struc(void)
{
    long  basesize, size;
    int   type, constsize, offset = 0;
    char *idstr, strucid[80];
    int   idlen = 0, struclen = 0;

    if (scan() == ID_TOKEN)
    {
        struclen = tokenlen;
        for (idlen = 0; idlen < struclen; idlen++)
            strucid[idlen] = tokenstr[idlen];
        scan();
    }
    while (next_line() == BYTE_TOKEN || scantoken == WORD_TOKEN || scantoken == EOL_TOKEN)
    {
        if (scantoken == EOL_TOKEN)
            continue;
        basesize = 1;
        type = scantoken == BYTE_TOKEN ? BYTE_TYPE : WORD_TYPE;
        if (scan() == OPEN_BRACKET_TOKEN)
        {
            basesize = 0;
            parse_constexpr(&basesize, &constsize);
            if (scantoken != CLOSE_BRACKET_TOKEN)
                parse_error("Missing closing bracket");
            scan();
        }
        do
        {
            size  = 1;
            idlen = 0;
            if (scantoken == ID_TOKEN)
            {
                idstr = tokenstr;
                idlen = tokenlen;
                if (scan() == OPEN_BRACKET_TOKEN)
                {
                    size = 0;
                    parse_constexpr(&size, &constsize);
                    if (scantoken != CLOSE_BRACKET_TOKEN)
                        parse_error("Missing closing bracket");
                    scan();
                }
            }
            size *= basesize;
            if (type & WORD_TYPE)
                size *= 2;
            if (idlen)
                idconst_add(idstr, idlen, offset);
            offset += size;
        } while (scantoken == COMMA_TOKEN);
    }
    if (struclen)
        idconst_add(strucid, struclen, offset);
    if (scantoken != END_TOKEN)
        parse_error("Missing STRUC/END");
    scan();
    return (1);
}
int parse_vars(int type)
{
    long value;
    int idlen, size, cfnparms;
    long cfnvals;
    char *idstr;

    switch (scantoken)
    {
        case SYSFLAGS_TOKEN:
            if (type & (EXTERN_TYPE | LOCAL_TYPE))
                parse_error("SYSFLAGS must be global");
            if (!parse_constexpr(&value, &size))
                parse_error("Bad constant");
            emit_sysflags(value);
            break;
        case CONST_TOKEN:
            if (scan() != ID_TOKEN)
                parse_error("Missing variable");
            idstr = tokenstr;
            idlen = tokenlen;
            if (scan() != SET_TOKEN)
                parse_error("Bad LValue");
            if (!parse_constexpr(&value, &size))
                parse_error("Bad constant");
            idconst_add(idstr, idlen, value);
            break;
        case STRUC_TOKEN:
            parse_struc();
            break;
        case EXPORT_TOKEN:
            if (type & (EXTERN_TYPE | LOCAL_TYPE))
                parse_error("Cannot export local/imported variables");
            type  = EXPORT_TYPE;
            idstr = tokenstr;
            if (scan() != BYTE_TOKEN && scantoken != WORD_TOKEN)
            {
                /*
                 * This could be an exported definition.
                 */
                scan_rewind(idstr);
                scan();
                return (0);
            }
            /*
             * Fall through to BYTE or WORD declaration.
             */
        case BYTE_TOKEN:
        case WORD_TOKEN:
            type   |= (scantoken == BYTE_TOKEN) ? BYTE_TYPE : WORD_TYPE;
            cfnvals = 1; // Just co-opt a long variable for this case
            if (scan() == OPEN_BRACKET_TOKEN)
            {
                //
                // Get base size for variables
                //
                cfnvals = 0;
                parse_constexpr(&cfnvals, &size);
                if (scantoken != CLOSE_BRACKET_TOKEN)
                    parse_error("Missing closing bracket");
            }
            else
                scan_rewind(tokenstr);
            if (type & WORD_TYPE)
                cfnvals *= 2;
            do parse_var(type, cfnvals); while (scantoken == COMMA_TOKEN);
            break;
        case PREDEF_TOKEN:
            /*
             * Pre definition.
             */
            do
            {
                if (scan() == ID_TOKEN)
                {
                    type  = (type & ~FUNC_PARMVALS) | PREDEF_TYPE;
                    idstr = tokenstr;
                    idlen = tokenlen;
                    cfnparms = 0;
                    cfnvals  = 1; // Default to one return value for compatibility
                    if (scan() == OPEN_PAREN_TOKEN)
                    {
                        do
                        {
                            if (scan() == ID_TOKEN)
                            {
                                cfnparms++;
                                scan();
                            }
                        } while (scantoken == COMMA_TOKEN);
                        if (scantoken != CLOSE_PAREN_TOKEN)
                            parse_error("Bad function parameter list");
                        scan();
                    }
                    if (scantoken == POUND_TOKEN)
                    {
                        if (!parse_const(&cfnvals))
                            parse_error("Invalid def return value count");
                        scan();
                    }
                    type |= funcparms_type(cfnparms) | funcvals_type(cfnvals);
                    idfunc_add(idstr, idlen, type, tag_new(type));
                }
                else
                    parse_error("Bad function pre-declaration");
            } while (scantoken == COMMA_TOKEN);
        case EOL_TOKEN:
            break;
        default:
            return (0);
    }
    return (1);
}
int parse_mods(void)
{
    if (scantoken == IMPORT_TOKEN)
    {
        if (scan() != ID_TOKEN)
            parse_error("Bad import definition");
        emit_moddep(tokenstr, tokenlen);
        scan();
        while (parse_vars(EXTERN_TYPE)) next_line();
        if (scantoken != END_TOKEN)
            parse_error("Missing END");
        scan();
    }
    if (scantoken == EOL_TOKEN)
        return (1);
    emit_moddep(0, 0);
    return (0);
}
int parse_lambda(void)
{
    int func_tag;
    int cfnparms;

    if (!infunc)
        parse_error("Lambda functions only allowed inside definitions");
    idlocal_save();
    /*
     * Parse parameters and return value count
     */
    cfnparms = 0;
    if (scan() == OPEN_PAREN_TOKEN)
    {
        do
        {
            if (scan() == ID_TOKEN)
            {
                cfnparms++;
                idlocal_add(tokenstr, tokenlen, WORD_TYPE, 2);
                scan();
            }
        } while (scantoken == COMMA_TOKEN);
        if (scantoken != CLOSE_PAREN_TOKEN)
            parse_error("Bad function parameter list");
    }
    else
        parse_error("Missing parameter list in lambda function");
    if (scan_lookahead() == OPEN_PAREN_TOKEN)
    {
        /*
         * Function call - parameters generate before call address
         */
        scan();
        lambda_seq[lambda_cnt] = parse_list(NULL, NULL);
        if (scantoken != CLOSE_PAREN_TOKEN)
            parse_error("Missing closing lambda function parenthesis");
    }
    else
    {
        lambda_seq[lambda_cnt] = parse_expr(NULL, NULL);
        scan_rewind(tokenstr);
    }
    sprintf(lambda_id[lambda_cnt], "_LAMBDA%04d", lambda_num++);
    if (idglobal_lookup(lambda_id[lambda_cnt], strlen(lambda_id[lambda_cnt])) >= 0)
    {
        func_tag = lambda_tag[lambda_cnt];
        idfunc_set(lambda_id[lambda_cnt], strlen(lambda_id[lambda_cnt]), DEF_TYPE | funcparms_type(cfnparms), func_tag); // Override any predef type & tag
    }
    else
    {
        func_tag = tag_new(DEF_TYPE);
        lambda_tag[lambda_cnt]     = func_tag;
        lambda_cparams[lambda_cnt] = cfnparms;
        idfunc_add(lambda_id[lambda_cnt], strlen(lambda_id[lambda_cnt]), DEF_TYPE | funcparms_type(cfnparms), func_tag);
    }
    lambda_cnt++;
    idlocal_restore();
    return (func_tag);
}
int parse_defs(void)
{
    char c, *idstr;
    int idlen, func_tag, cfnparms, cfnvals, type = GLOBAL_TYPE, pretype;
    static char bytecode = 0;
    if (scantoken == EXPORT_TOKEN)
    {
        if (scan() != DEF_TOKEN && scantoken != ASM_TOKEN)
            parse_error("Bad export definition");
        type = EXPORT_TYPE;
    }
    if (scantoken == DEF_TOKEN)
    {
        if (scan() != ID_TOKEN)
            parse_error("Missing function name");
        emit_bytecode_seg();
        lambda_cnt  = 0;
        bytecode    = 1;
        cfnparms    = 0;
        infuncvals  = 1; // Defaut to one return value for compatibility
        infunc      = 1;
        type       |= DEF_TYPE;
        idstr       = tokenstr;
        idlen       = tokenlen;
        idlocal_reset();
        /*
         * Parse parameters and return value count
         */
        if (scan() == OPEN_PAREN_TOKEN)
        {
            do
            {
                if (scan() == ID_TOKEN)
                {
                    cfnparms++;
                    idlocal_add(tokenstr, tokenlen, WORD_TYPE, 2);
                    scan();
                }
            } while (scantoken == COMMA_TOKEN);
            if (scantoken != CLOSE_PAREN_TOKEN)
                parse_error("Bad function parameter list");
            scan();
        }
        if (scantoken == POUND_TOKEN)
        {
            if (!parse_const(&infuncvals))
                parse_error("Invalid def return value count");
            scan();
        }
        type |= funcparms_type(cfnparms) | funcvals_type(infuncvals);
        if (idglobal_lookup(idstr, idlen) >= 0)
        {
            pretype = id_type(idstr, idlen);
            if (!(pretype & PREDEF_TYPE))
                parse_error("Mismatch function type");
            if ((pretype & FUNC_PARMVALS) != (type & FUNC_PARMVALS))
                parse_error("Mismatch function params/return values");
            emit_idfunc(id_tag(idstr, idlen), PREDEF_TYPE, idstr, 0);
            func_tag = tag_new(type);
            idfunc_set(idstr, idlen, type, func_tag); // Override any predef type & tag
        }
        else
        {
            func_tag = tag_new(type);
            idfunc_add(idstr, idlen, type, func_tag);
        }
        c = idstr[idlen];
        idstr[idlen] = '\0';
        emit_idfunc(func_tag, type, idstr, 1);
        idstr[idlen] = c;
        /*
         * Parse local vars
         */
        while (parse_vars(LOCAL_TYPE)) next_line();
        emit_enter(cfnparms);
        prevstmnt = 0;
        while (parse_stmnt()) next_line();
        infunc = 0;
        if (scantoken != END_TOKEN)
            parse_error("Missing END");
        scan();
        if (prevstmnt != RETURN_TOKEN)
        {
            if (infuncvals)
               parse_warn("No return values");
            for (cfnvals = 0; cfnvals < infuncvals; cfnvals++)
                emit_const(0);
            emit_leave();
        }
        while (lambda_cnt--)
            emit_lambdafunc(lambda_tag[lambda_cnt], lambda_id[lambda_cnt], lambda_cparams[lambda_cnt], lambda_seq[lambda_cnt]);
        return (1);
    }
    else if (scantoken == ASM_TOKEN)
    {
        if (scan() != ID_TOKEN)
            parse_error("Missing function name");
        if (bytecode)
            parse_error("ASM code only allowed before DEF code");
        cfnparms    = 0;
        infuncvals  = 1; // Defaut to one return value for compatibility
        infunc      = 1;
        type       |= ASM_TYPE;
        idstr       = tokenstr;
        idlen       = tokenlen;
        idlocal_reset();
        if (scan() == OPEN_PAREN_TOKEN)
        {
            do
            {
                if (scan() == ID_TOKEN)
                {
                    cfnparms++;
                    scan();
                }
            }
            while (scantoken == COMMA_TOKEN);
            if (scantoken != CLOSE_PAREN_TOKEN)
                parse_error("Bad function parameter list");
            scan();
        }
        if (scantoken == POUND_TOKEN)
        {
            if (!parse_const(&infuncvals))
                parse_error("Invalid def return value count");
            scan();
        }
        type |= funcparms_type(cfnparms) | funcvals_type(infuncvals);
        if (idglobal_lookup(idstr, idlen) >= 0)
        {
            pretype = id_type(idstr, idlen);
            if (!(pretype & PREDEF_TYPE))
                parse_error("Mismatch function type");
            if ((pretype & FUNC_PARMVALS) != (type & FUNC_PARMVALS))
                parse_error("Mismatch function params/return values");
            emit_idfunc(id_tag(idstr, idlen), PREDEF_TYPE, idstr, 0);
            func_tag = tag_new(type);
            idfunc_set(idstr, idlen, type, func_tag); // Override any predef type & tag
        }
        else
        {
            func_tag = tag_new(type);
            idfunc_add(idstr, idlen, type, func_tag);
        }
        c = idstr[idlen];
        idstr[idlen] = '\0';
        emit_idfunc(func_tag, type, idstr, 0);
        idstr[idlen] = c;
        do
        {
            if (scantoken != END_TOKEN && scantoken != EOL_TOKEN)
            {
                scantoken = EOL_TOKEN;
                emit_asm(inputline);
            }
            next_line();
        } while (scantoken != END_TOKEN);
        scan();
        return (1);
    }
    return (scantoken == EOL_TOKEN);
}
int parse_module(void)
{
    emit_header();
    if (next_line())
    {
        while (parse_mods())            next_line();
        while (parse_vars(GLOBAL_TYPE)) next_line();
        while (parse_defs())            next_line();
        if (scantoken != DONE_TOKEN && scantoken != EOF_TOKEN)
        {
            emit_bytecode_seg();
            emit_start();
            idlocal_reset();
            emit_idfunc(0, 0, NULL, 1);
            prevstmnt = 0;
            while (parse_stmnt()) next_line();
            if (scantoken != DONE_TOKEN)
                parse_error("Missing DONE");
            if (prevstmnt != RETURN_TOKEN)
            {
                emit_const(0);
                emit_ret();
            }
        }
    }
    emit_trailer();
    return (0);
}
