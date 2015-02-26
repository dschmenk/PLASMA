#include <stdio.h>
#include "tokens.h"
#include "symbols.h"
#include "lex.h"
#include "codegen.h"
#include "parse.h"

int infunc = 0, break_tag = 0, cont_tag = 0, stack_loop = 0;
t_token prevstmnt;

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
    {
        parse_error("Stack overflow\n");
        return;
    }
    opstack[opsptr]   = op;
    precstack[opsptr] = prec;
}
t_token pop_op(void)
{
    if (opsptr < 0)
    {
        parse_error("Stack underflow\n");
        return (0);
    }
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
    {
        parse_error("Stack overflow\n");
        return;
    }
    valstack[valptr]  = value;
    sizestack[valptr] = size;
    typestack[valptr] = type;
}
int pop_val(long *value, int *size, int *type)
{
    if (valptr < 0)
    {
        parse_error("Stack underflow\n");
        return (-1);
    }
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
    {
        parse_error("Bad constant operand");
        return (0);
    }
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
            {
                parse_error("Bad expression in parenthesis");
                return (0);
            }
            if (scantoken != CLOSE_PAREN_TOKEN)
            {
                parse_error("Missing closing parenthesis");
                return (0);
            }
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
            size  = tokenlen - 1;
            value = constval;
            type  = STRING_TYPE;
            if (mod)
            {
                parse_error("Invalid string modifiers");
                return (0);
            }
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
            else if ((type & (FUNC_TYPE | EXTERN_TYPE)) || ((type & ADDR_TYPE) && (mod == 8)))
                value = id_tag(tokenstr, tokenlen);
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
                        {
                            parse_error(": Invalid binary operation");
                            return (0);
                        }
                    push_op(scantoken, binary_ops_precedence[i]);
                    break;
                }
        }
    } while (matchop == 2);
    if (matchop == 0 && prevmatch == 0)
        return (0);
    if (matchop == 0 && prevmatch == 2)
    {
        parse_error("Missing operand");
        return (0);
    }
    while (optos < opsptr)
        if (!calc_op(pop_op()))
        {
            parse_error(": Invalid binary operation");
            return (0);
        }
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
int parse_expr(void);
int parse_term(void)
{
    /*
     * Parse terminal tokens.
     */
    switch (scan())
    {
        case CHAR_TOKEN:
        case INT_TOKEN:
        case ID_TOKEN:
        case STRING_TOKEN:
            break;
        case OPEN_PAREN_TOKEN:
            if (!parse_expr())
            {
                parse_error("Bad expression in parenthesis");
                return (0);
            }
            if (scantoken != CLOSE_PAREN_TOKEN)
            {
                parse_error("Missing closing parenthesis");
                return (0);
            }
            break;
        default:
            /*
             * Non-terminal token.
             */
            return (0);
    }
    return (1);
}
int parse_value(int rvalue)
{
    int cparams;
    int deref = rvalue;
    int optos = opsptr;
    int type = 0, value = 0, emit_value = 0;
    int ref_type, const_size;
    long ref_offset, const_offset;
    /*
     * Parse pre operand operators.
     */
    while (!parse_term())
    {
        switch (scantoken)
        {
            case ADD_TOKEN:
                /*
                 * Just ignore unary plus, it is a no-op.
                 */
                break;
            case BPTR_TOKEN:
                if (deref)
                    push_op(scantoken, 0);
                else
                {
                    deref++;
                    type |= BPTR_TYPE;
                }
                break;
            case WPTR_TOKEN:
                if (deref)
                    push_op(scantoken, 0);
                else
                {
                    deref++;
                    type |= WPTR_TYPE;
                }
                break;
            case AT_TOKEN:
                deref--;
                break;
            case NEG_TOKEN:
            case COMP_TOKEN:
            case LOGIC_NOT_TOKEN:
                push_op(scantoken, 0);
                break;
            default:
                return (0);
        }
    }
    /*
     * Determine which terminal type.
     */
    if (scantoken == INT_TOKEN || scantoken == CHAR_TOKEN)
    {
        value = constval;
        type |= CONST_TYPE;
    }
    else if (scantoken == ID_TOKEN)
    {
        if ((type |= id_type(tokenstr, tokenlen)) & CONST_TYPE)
            value = id_const(tokenstr, tokenlen);
        else if (type & VAR_TYPE)
            value = id_tag(tokenstr, tokenlen);
        else if (type & FUNC_TYPE)
            value = id_tag(tokenstr, tokenlen);
        else
        {
            printf("Bad ID type\n");
            return (0);
        }
    }
    else if (scantoken == CLOSE_PAREN_TOKEN)
    {
        // type |= WORD_TYPE;
        emit_value = 1;
    }
    else if (scantoken == STRING_TOKEN)
    {
        /*
         * This is a special case. Just emit the string and return
         */
        emit_conststr(constval, tokenlen - 1);
        scan();
        return WORD_TYPE;
    }
    else
        return (0);
    if (type & CONST_TYPE)
    {
        /*
         * Quick optimizations
         */
        while ((optos < opsptr)
               && ((tos_op() == NEG_TOKEN) || (tos_op() == COMP_TOKEN) || (tos_op() == LOGIC_NOT_TOKEN)))
        {
            switch (pop_op())
            {
                case NEG_TOKEN:
                    value = -value;
                    break;
                case COMP_TOKEN:
                    value = ~value;
                    break;
                case LOGIC_NOT_TOKEN:
                    value = value ? 0 : -1;
                    break;
            }
        }
    }
    /*
     * Parse post operand operators.
     */
    ref_type   = type & ~PTR_TYPE;
    ref_offset = 0;
    while (scan() == OPEN_PAREN_TOKEN
     || scantoken == OPEN_BRACKET_TOKEN
     || scantoken == PTRB_TOKEN
     || scantoken == PTRW_TOKEN
     || scantoken == DOT_TOKEN
     || scantoken == COLON_TOKEN)
    {
        switch (scantoken)
        {
            case OPEN_PAREN_TOKEN:
                /*
                 * Function call
                 */
                if (emit_value)
                {
                    if (ref_offset != 0)
                    {
                        emit_const(ref_offset);
                        emit_op(ADD_TOKEN);
                        ref_offset = 0;
                    }
                    if (ref_type & PTR_TYPE)
                        (ref_type & BPTR_TYPE) ? emit_lb() : emit_lw();
                    if (scan_lookahead() != CLOSE_PAREN_TOKEN)
                        emit_push();
                }
                cparams = 0;
                while (parse_expr())
                {
                    cparams++;
                    if (scantoken != COMMA_TOKEN)
                        break;
                }
                if (scantoken != CLOSE_PAREN_TOKEN)
                {
                    parse_error("Missing closing parenthesis");
                    return (0);
                }
                if (ref_type & (FUNC_TYPE | CONST_TYPE))
                    emit_call(value, ref_type);
                else
                {
                    if (!emit_value)
                    {
                        if (type & CONST_TYPE)
                            emit_const(value);
                        else if (type & VAR_TYPE)
                        {
                            if (type & LOCAL_TYPE)
                                emit_llw(value + ref_offset);
                            else
                                emit_law(value, ref_offset, type);
                            ref_offset = 0;
                        }
                    }
                    else
                        if (cparams)
                            emit_pull();
                    emit_ical();
                }
                emit_value = 1;
                ref_type   = 0;
                break;
            case OPEN_BRACKET_TOKEN:
                /*
                 * Array of arrays
                 */
                if (!emit_value)
                {
                    if (type & CONST_TYPE)
                        emit_const(value);
                    else if (type & ADDR_TYPE)
                    {
                        if (type & LOCAL_TYPE)
                            emit_localaddr(value + ref_offset);
                        else
                            emit_globaladdr(value, ref_offset, type);
                        ref_offset = 0;
                    }
                    else
                    {
                        parse_error("Bad index reference");
                        return (0);
                    }
                    emit_value = 1;
                }
                else
                {
                    if (ref_offset != 0)
                    {
                        emit_const(ref_offset);
                        emit_op(ADD_TOKEN);
                        ref_offset = 0;
                    }
                }
                while (parse_expr())
                {
                    if (scantoken != COMMA_TOKEN)
                        break;
                    emit_indexword();
                    emit_lw();
                }
                if (scantoken != CLOSE_BRACKET_TOKEN)
                {
                    parse_error("Missing closing bracket");
                    return (0);
                }
                if (ref_type & (WPTR_TYPE | WORD_TYPE))
                {
                    emit_indexword();
                    ref_type = WPTR_TYPE;
                }
                else
                {
                    emit_indexbyte();
                    ref_type = BPTR_TYPE;
                }
                break;
            case PTRB_TOKEN:
            case PTRW_TOKEN:
                /*
                 * Structure member pointer
                 */
                if (!emit_value)
                {
                    if (type & CONST_TYPE)
                        emit_const(value);
                    else if (type & ADDR_TYPE)
                    {
                        if (type & LOCAL_TYPE)
                            (ref_type & BYTE_TYPE) ? emit_llb(value + ref_offset) : emit_llw(value + ref_offset);
                        else
                            (ref_type & BYTE_TYPE) ? emit_lab(value, ref_offset, type) : emit_law(value, ref_offset, type);
                    }
                    emit_value = 1;
                }
                else
                {
                    if (ref_offset != 0)
                    {
                        emit_const(ref_offset);
                        emit_op(ADD_TOKEN);
                    }
                    if (ref_type & PTR_TYPE)
                        (ref_type & BPTR_TYPE) ? emit_lb() : emit_lw();
                }
                ref_offset = 0;
                ref_type = (scantoken == PTRB_TOKEN) ? BPTR_TYPE : WPTR_TYPE;
                if (!parse_const(&ref_offset))
                    scan_rewind(tokenstr);
                break;
            case DOT_TOKEN:
            case COLON_TOKEN:
                /*
                 * Structure member offset
                 */
                ref_type = (ref_type & (VAR_TYPE | CONST_TYPE)) 
                         ? ((scantoken == DOT_TOKEN) ? BYTE_TYPE : WORD_TYPE)
                         : ((scantoken == DOT_TOKEN) ? BPTR_TYPE : WPTR_TYPE);
                if (parse_const(&const_offset))
                    ref_offset += const_offset;
                else
                    scan_rewind(tokenstr);
                if (!emit_value)
                {
                    if (type & CONST_TYPE)
                    {
                        value += ref_offset;
                        ref_offset = 0;
                    }
                    else if (type & FUNC_TYPE)
                    {
                        emit_globaladdr(value, ref_offset, type);
                        ref_offset = 0;
                        emit_value = 1;
                    }
                }
                break;
        }
    }
    if (emit_value)
    {
        if (ref_offset != 0)
        {
            emit_const(ref_offset);
            emit_op(ADD_TOKEN);
            ref_offset = 0;
        }
        if (deref)
        {
            if (ref_type & BPTR_TYPE) emit_lb();
            else if (ref_type & WPTR_TYPE) emit_lw();
        }
    }
    else
    {
        if (deref)
        {
            if (type & CONST_TYPE)
            {
                emit_const(value);
                if (ref_type & VAR_TYPE)
                    (ref_type & BYTE_TYPE) ? emit_lb() : emit_lw();
            }
            else if (type & FUNC_TYPE)
                emit_call(value, ref_type);
            else if (type & VAR_TYPE)
            {
                if (type & LOCAL_TYPE)
                    (ref_type & BYTE_TYPE) ? emit_llb(value + ref_offset) : emit_llw(value + ref_offset);
                else
                    (ref_type & BYTE_TYPE) ? emit_lab(value, ref_offset, ref_type) : emit_law(value, ref_offset, ref_type);
            }
        }
        else
        {
            if (type & CONST_TYPE)
                emit_const(value);
            else if (type & ADDR_TYPE)
            {
                if (type & LOCAL_TYPE)
                    emit_localaddr(value + ref_offset);
                else
                    emit_globaladdr(value, ref_offset, ref_type);
            }
        }
    }
    while (optos < opsptr)
    {
        if (!emit_unaryop(pop_op()))
        {
            parse_error(": Invalid unary operation");
            return (0);
        }
    }
    if (type & PTR_TYPE)
        ref_type = type;
    return (ref_type ? ref_type : WORD_TYPE);
}
int parse_expr()
{
    int prevmatch;
    int matchop = 0;
    int optos = opsptr;
    int i;
    int prevtype, type = 0;
    do
    {
        /*
         * Parse sequence of double operand operations.
         */
        prevmatch = matchop;
        matchop   = 0;
        if (parse_value(1))
        {
            matchop = 1;
            for (i = 0; i < sizeof(binary_ops_table); i++)
                if (scantoken == binary_ops_table[i])
                {
                    matchop = 2;
                    if (binary_ops_precedence[i] >= tos_op_prec(optos))
                        if (!emit_op(pop_op()))
                        {
                            parse_error(": Invalid binary operation");
                            return (0);
                        }
                    push_op(scantoken, binary_ops_precedence[i]);
                    break;
                }
        }
    }
    while (matchop == 2);
    if (matchop == 0 && prevmatch == 2)
    {
        parse_error("Missing operand");
        return (0);
    }
    while (optos < opsptr)
        if (!emit_op(pop_op()))
        {
            parse_error(": Invalid binary operation");
            return (0);
        }
    return (matchop || prevmatch);
}
int parse_stmnt(void)
{
    int tag_prevbrk, tag_prevcnt, tag_else, tag_endif, tag_while, tag_wend, tag_repeat, tag_for, tag_choice, tag_of;
    int type, addr, step;
    char *idptr;

    /*
     * Optimization for last function LEAVE and OF clause.
     */
    if (scantoken != END_TOKEN && scantoken != DONE_TOKEN && scantoken != OF_TOKEN && scantoken != DEFAULT_TOKEN)
        prevstmnt = scantoken;
    switch (scantoken)
    {
        case IF_TOKEN:
            if (!parse_expr())
            {
                parse_error("Bad expression");
                return (0);
            }
            tag_else  = tag_new(BRANCH_TYPE);
            tag_endif = tag_new(BRANCH_TYPE);
            emit_brfls(tag_else);
            scan();
            do {
                while (parse_stmnt()) next_line();
                if (scantoken != ELSEIF_TOKEN)
                    break;
                emit_brnch(tag_endif);
                emit_codetag(tag_else);
                if (!parse_expr())
                {
                    parse_error("Bad expression");
                    return (0);
                }
                tag_else = tag_new(BRANCH_TYPE);
                emit_brfls(tag_else);
            }
            while (1);
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
            {
                parse_error("Missing IF/FIN");
                return (0);
            }
            break;
        case WHILE_TOKEN:
            tag_while   = tag_new(BRANCH_TYPE);
            tag_wend    = tag_new(BRANCH_TYPE);
            tag_prevcnt = cont_tag;
            cont_tag    = tag_while;
            tag_prevbrk = break_tag;
            break_tag   = tag_wend;
            emit_codetag(tag_while);
            if (!parse_expr())
            {
                parse_error("Bad expression");
                return (0);
            }
            emit_brfls(tag_wend);
            while (parse_stmnt()) next_line();
            if (scantoken != LOOP_TOKEN)
            {
                parse_error("Missing WHILE/END");
                return (0);
            }
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
            {
                parse_error("Missing REPEAT/UNTIL");
                return (0);
            }
            emit_codetag(cont_tag);
            cont_tag = tag_prevcnt;
            if (!parse_expr())
            {
                parse_error("Bad expression");
                return (0);
            }
            emit_brfls(tag_repeat);
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
            {
                parse_error("Missing FOR variable");
                return (0);
            }
            type = id_type(tokenstr, tokenlen);
            addr = id_tag(tokenstr, tokenlen);
            if (scan() != SET_TOKEN)
            {
                parse_error("Missing FOR =");
                return (0);
            }
            if (!parse_expr())
            {
                parse_error("Bad FOR expression");
                return (0);
            }
            emit_codetag(tag_for);
            if (type & LOCAL_TYPE)
                type & BYTE_TYPE ? emit_dlb(addr) : emit_dlw(addr);
            else
                type & BYTE_TYPE ? emit_dab(addr, type) : emit_daw(addr, type);
            if (scantoken == TO_TOKEN)
                step = 1;
            else if (scantoken == DOWNTO_TOKEN)
                step = -1;
            else
            {
                parse_error("Missing FOR TO");
                return (0);
            }
            if (!parse_expr())
            {
                parse_error("Bad FOR TO expression");
                return (0);
            }
            step > 0 ? emit_brgt(break_tag) : emit_brlt(break_tag);
            if (scantoken == STEP_TOKEN)
            {
                if (!parse_expr())
                {
                    parse_error("Bad FOR STEP expression");
                    return (0);
                }
                emit_op(step > 0 ? ADD_TOKEN : SUB_TOKEN);
            }
            else
                emit_unaryop(step > 0 ? INC_TOKEN : DEC_TOKEN);
            while (parse_stmnt()) next_line();
            if (scantoken != NEXT_TOKEN)
            {
                parse_error("Missing FOR/NEXT ");
                return (0);
            }
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
            if (!parse_expr())
            {
                parse_error("Bad CASE expression");
                return (0);
            }
            next_line();
            while (scantoken != ENDCASE_TOKEN)
            {
                if (scantoken == OF_TOKEN)
                {
                    if (!parse_expr())
                    {
                        parse_error("Bad CASE OF expression");
                        return (0);
                    }
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
                    {
                        parse_error("Bad CASE DEFAULT clause");
                        return (0);
                    }
                }
                else if (scantoken == EOL_TOKEN) 
                {
                    next_line();
                }
                else
                {
                    parse_error("Bad CASE clause");
                    return (0);
                }
            }
            if (tag_of)
                emit_codetag(tag_of);
            emit_codetag(break_tag);
            emit_drop();
            break_tag = tag_prevbrk;
            stack_loop--;
            break;
        case CONTINUE_TOKEN:
            if (cont_tag)
                emit_brnch(cont_tag);
            else
            {
                parse_error("CONTINUE without loop");
                return (0);
            }
            break;
        case BREAK_TOKEN:
            if (break_tag)
                emit_brnch(break_tag);
            else
            {
                parse_error("BREAK without loop");
                return (0);
            }
            break;
        case RETURN_TOKEN:
            if (infunc)
            {
                int i;
                for (i = 0; i < stack_loop; i++)
                    emit_drop();
                if (!parse_expr())
                    emit_const(0);
                emit_leave();
            }
            else
            {
                if (!parse_expr())
                    emit_const(0);
                emit_ret();
            }
            break;
        case EOL_TOKEN:
        case COMMENT_TOKEN:
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
        case ID_TOKEN:
            idptr = tokenstr;
            type = id_type(tokenstr, tokenlen);
            addr = id_tag(tokenstr, tokenlen);
            if (type & VAR_TYPE)
            {
                int elem_type    = type;
                long elem_offset = 0;
                if (scan() == DOT_TOKEN || scantoken == COLON_TOKEN)
                {
                    /*
                     * Structure member offset
                     */
                    int elem_size;
                    elem_type = (scantoken == DOT_TOKEN) ? BYTE_TYPE : WORD_TYPE;
                    if (!parse_const(&elem_offset))
                        scantoken = ID_TOKEN;
                    else
                        scan();
                }
                if (scantoken == SET_TOKEN)
                {
                    if (!parse_expr())
                    {
                        parse_error("Bad expression");
                        return (0);
                    }
                    if (type & LOCAL_TYPE)
                        (elem_type & BYTE_TYPE) ? emit_slb(addr + elem_offset) : emit_slw(addr + elem_offset);
                    else 
                        (elem_type & BYTE_TYPE) ? emit_sab(addr, elem_offset, type) : emit_saw(addr, elem_offset, type);
                    break;
                }
            }
            else if (type & FUNC_TYPE)
            {
                if (scan() == EOL_TOKEN)
                {
                    emit_call(addr, type);
                    emit_drop();
                    break;
                }
            }
            tokenstr = idptr;
        default:
            scan_rewind(tokenstr);
            if ((type = parse_value(0)) != 0)
            {
                if (scantoken == SET_TOKEN)
                {
                    if (!parse_expr())
                    {
                        parse_error("Bad expression");
                        return (0);
                    }
                    if (type & LOCAL_TYPE)
                        (type & (BYTE_TYPE | BPTR_TYPE)) ? emit_sb() : emit_sw();
                    else
                        (type & (BYTE_TYPE | BPTR_TYPE)) ? emit_sb() : emit_sw();
                }
                else
                {
                    if (type & BPTR_TYPE)
                        emit_lb();
                    else if (type & WPTR_TYPE)
                        emit_lw();
                    emit_drop();
                }
            }
            else
            {
                parse_error("Syntax error");
                return (0);
            }
    }
    if (scan() != EOL_TOKEN && scantoken != COMMENT_TOKEN)
    {
        parse_error("Extraneous characters");
        return (0);
    }
    return (1);
}
int parse_var(int type)
{
    char *idstr;
    long constval;
    int  consttype, constsize, arraysize, idlen = 0;
    long size = 1;
    
    if (scan() == OPEN_BRACKET_TOKEN)
    {
        size = 0;
        parse_constexpr(&size, &constsize);
        if (scantoken != CLOSE_BRACKET_TOKEN)
        {
            parse_error("Missing closing bracket");
            return (0);
        }
        scan();
    }
    if (scantoken == ID_TOKEN)
    {
        idstr = tokenstr;
        idlen = tokenlen;
        if (scan() == OPEN_BRACKET_TOKEN)
        {
            size = 0;
            parse_constexpr(&size, &constsize);
            if (scantoken != CLOSE_BRACKET_TOKEN)
            {
                parse_error("Missing closing bracket");
                return (0);
            }
            scan();
        }
    }
    if (type & WORD_TYPE)
        size *= 2;
    if (scantoken == SET_TOKEN)
    {
        if (type & (EXTERN_TYPE | LOCAL_TYPE))
        {
            parse_error("Cannot initiallize local/external variables");
            return (0);
        }
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
                {
                    parse_error("Bad array declaration");
                    return (0);
                }
            }
            if (size > arraysize)
                idglobal_size(PTR_TYPE, size, arraysize);
        }
        else
        {
            parse_error("Bad variable initializer");
            return (0);
        }
    }
    else if (idlen)
        id_add(idstr, idlen, type, size);
    return (1);
}
int parse_struc(void)
{
    long  size;
    int   type, constsize, offset = 0;
    char *idstr, strucid[80];
    int   idlen = 0, struclen = 0;

    if (scan() == ID_TOKEN)
    {
        struclen = tokenlen;
        for (idlen = 0; idlen < struclen; idlen++)
            strucid[idlen] = tokenstr[idlen];
    }
    while (next_line() == BYTE_TOKEN || scantoken == WORD_TOKEN)
    {
        size = 1;
        type = scantoken == BYTE_TOKEN ? BYTE_TYPE : WORD_TYPE;
        if (scan() == OPEN_BRACKET_TOKEN)
        {
            size = 0;
            parse_constexpr(&size, &constsize);
            if (scantoken != CLOSE_BRACKET_TOKEN)
            {
                parse_error("Missing closing bracket");
                return (0);
            }
            scan();
        }
        do {
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
                    {
                        parse_error("Missing closing bracket");
                        return (0);
                    }
                    scan();
                }                   
            }
            if (type & WORD_TYPE)
                size *= 2;
            if (idlen)
                idconst_add(idstr, idlen, offset);
            offset += size;
        } while (scantoken == COMMA_TOKEN);
        if (scantoken != EOL_TOKEN && scantoken != COMMENT_TOKEN)
            return (0);
    }
    if (struclen)
        idconst_add(strucid, struclen, offset);
    return (scantoken == END_TOKEN);
}
int parse_vars(int type)
{
    long value;
    int idlen, size;
    char *idstr;
    
    switch (scantoken)
    {
        case SYSFLAGS_TOKEN:
            if (type & (EXTERN_TYPE | LOCAL_TYPE))
            {
                parse_error("sysflags must be global");
                return (0);
            }
            if (!parse_constexpr(&value, &size))
            {
                parse_error("Bad constant");
                return (0);
            }
            emit_sysflags(value);
            break;
        case CONST_TOKEN:
            if (scan() != ID_TOKEN)
            {
                parse_error("Missing variable");
                return (0);
            }
            idstr = tokenstr;
            idlen = tokenlen;
            if (scan() != SET_TOKEN)
            {
                parse_error("Bad LValue");
                return (0);
            }
            if (!parse_constexpr(&value, &size))
            {
                parse_error("Bad constant");
                return (0);
            }
            idconst_add(idstr, idlen, value);
            break;
        case STRUC_TOKEN:
            if (!parse_struc())
            {
                parse_error("Bad structure definition");
                return (0);
            }
            break;
        case EXPORT_TOKEN:
            if (type & (EXTERN_TYPE | LOCAL_TYPE))
            {
                parse_error("Cannot export local/imported variables");
                return (0);
            }
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
            type |= (scantoken == BYTE_TOKEN) ? BYTE_TYPE : WORD_TYPE;
            if (!parse_var(type))
                return (0);
            while (scantoken == COMMA_TOKEN)
            {
                if (!parse_var(type))
                    return (0);
            }
            break;
        case PREDEF_TOKEN:
            /*
             * Pre definition.
             */
            if (scan() == ID_TOKEN)
            {
                type |= PREDEF_TYPE;
                idstr = tokenstr;
                idlen = tokenlen;
                idfunc_add(tokenstr, tokenlen, type, tag_new(type));
                while (scan() == COMMA_TOKEN)
                {
                    if (scan() == ID_TOKEN)
                    {
                        idstr = tokenstr;
                        idlen = tokenlen;
                        idfunc_add(tokenstr, tokenlen, type, tag_new(type));
                    }
                    else
                    {
                        parse_error("Bad function pre-declaration");
                        return (0);
                    }
                }
            }
            else
            {
                parse_error("Bad function pre-declaration");
                return (0);
            }
        case EOL_TOKEN:
        case COMMENT_TOKEN:
            return (1);
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
        {
            parse_error("Bad import definition");
            return (0);
        }
        emit_moddep(tokenstr, tokenlen);
        scan();
        while (parse_vars(EXTERN_TYPE)) next_line();
        if (scantoken != END_TOKEN)
        {
            parse_error("Syntax error");
            return (0);
        }
        if (scan() != EOL_TOKEN && scantoken != COMMENT_TOKEN)
        {
            parse_error("Extraneous characters");
            return (0);
        }
    }
    if (scantoken == EOL_TOKEN || scantoken == COMMENT_TOKEN)
        return (1);
    emit_moddep(0, 0);
    return (0);
}
int parse_defs(void)
{
    char c;
    int func_tag, cfnparms, type = GLOBAL_TYPE;
    static char bytecode = 0;
    if (scantoken == EXPORT_TOKEN)
    {
        if (scan() != DEF_TOKEN && scantoken != ASM_TOKEN)
        {
            parse_error("Bad export definition");
            return 0;
        }
        type = EXPORT_TYPE;
    }
    if (scantoken == DEF_TOKEN)
    {
        if (scan() != ID_TOKEN)
        {
            parse_error("Missing function name");
            return (0);
        }
        emit_bytecode_seg();
        bytecode    = 1;
        cfnparms    = 0;
        infunc      = 1;
        type       |= DEF_TYPE;
        if (idglobal_lookup(tokenstr, tokenlen) >= 0)
        {
            if (!(id_type(tokenstr, tokenlen) & PREDEF_TYPE))
            {
                parse_error("Mismatch function type");
                return (0);
            }
            emit_idfunc(id_tag(tokenstr, tokenlen), PREDEF_TYPE, tokenstr);
            func_tag = tag_new(type);
            idfunc_set(tokenstr, tokenlen, type, func_tag); // Override any predef type & tag
        }
        else
        {
            func_tag = tag_new(type);
            idfunc_add(tokenstr, tokenlen, type, func_tag);
        }
        c = tokenstr[tokenlen];
        tokenstr[tokenlen] = '\0';
        emit_idfunc(func_tag, type, tokenstr);
        emit_def(tokenstr, 1);
        tokenstr[tokenlen] = c;
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
            {
                parse_error("Bad function parameter list");
                return (0);
            }
            scan();
        }
        while (parse_vars(LOCAL_TYPE)) next_line();
        emit_enter(cfnparms);
        prevstmnt = 0;
        while (parse_stmnt()) next_line();
        infunc = 0;
        if (scantoken != END_TOKEN)
        {
            parse_error("Syntax error");
            return (0);
        }
        if (scan() != EOL_TOKEN && scantoken != COMMENT_TOKEN)
        {
            parse_error("Extraneous characters");
            return (0);
        }
        if (prevstmnt != RETURN_TOKEN)
        {
            emit_const(0);
            emit_leave();
        }
        return (1);
    }
    else if (scantoken == ASM_TOKEN)
    {
        if (scan() != ID_TOKEN)
        {
            parse_error("Missing function name");
            return (0);
        }
        if (bytecode)
        {
            parse_error("ASM code only allowed before DEF code");
            return (0);
        }
        cfnparms    = 0;
        infunc      = 1;
        type       |= ASM_TYPE;
        if (idglobal_lookup(tokenstr, tokenlen) >= 0)
        {
            if (!(id_type(tokenstr, tokenlen) & PREDEF_TYPE))
            {
                parse_error("Mismatch function type");
                return (0);
            }
            emit_idfunc(id_tag(tokenstr, tokenlen), PREDEF_TYPE, tokenstr);
            func_tag = tag_new(type);
            idfunc_set(tokenstr, tokenlen, type, func_tag); // Override any predef type & tag
        }
        else
        {
            func_tag = tag_new(type);
            idfunc_add(tokenstr, tokenlen, type, func_tag);
        }
        c = tokenstr[tokenlen];
        tokenstr[tokenlen] = '\0';
        emit_idfunc(func_tag, type, tokenstr);
        emit_def(tokenstr, 0);
        tokenstr[tokenlen] = c;
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
            }
            while (scantoken == COMMA_TOKEN);
            if (scantoken != CLOSE_PAREN_TOKEN)
            {
                parse_error("Bad function parameter list");
                return (0);
            }
            scan();
        }
        do
        {
            if (scantoken == EOL_TOKEN || scantoken == COMMENT_TOKEN)
                next_line();
            else if (scantoken != END_TOKEN)
            {
                emit_asm(inputline);
                next_line();
            }
        }
        while (scantoken != END_TOKEN);
        return (1);
    }
    if (scantoken == EOL_TOKEN || scantoken == COMMENT_TOKEN)
        return (1);
    return (0);
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
            emit_def("_INIT", 1);
            prevstmnt = 0;
            while (parse_stmnt()) next_line();
            if (scantoken != DONE_TOKEN)
                parse_error("Missing DONE statement");
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
