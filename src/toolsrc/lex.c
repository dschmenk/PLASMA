#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "plasm.h"

char *statement, *tokenstr, *scanpos = "", *strpos = "";
t_token scantoken, prevtoken;
int tokenlen;
long constval;
FILE* inputfile;
char *filename;
int lineno = 0;
FILE* outer_inputfile = NULL;
char* outer_filename;
int outer_lineno;
t_token keywords[] = {
    IF_TOKEN,               'I', 'F',
    ELSE_TOKEN,             'E', 'L', 'S', 'E',
    ELSEIF_TOKEN,           'E', 'L', 'S', 'I', 'F',
    FIN_TOKEN,              'F', 'I', 'N',
    WHILE_TOKEN,            'W', 'H', 'I', 'L', 'E',
    LOOP_TOKEN,             'L', 'O', 'O', 'P',
    CASE_TOKEN,             'W', 'H', 'E', 'N',
    OF_TOKEN,               'I', 'S',
    DEFAULT_TOKEN,          'O', 'T', 'H', 'E', 'R', 'W', 'I', 'S', 'E',
    ENDCASE_TOKEN,          'W', 'E', 'N', 'D',
    FOR_TOKEN,              'F', 'O', 'R',
    TO_TOKEN,               'T', 'O',
    DOWNTO_TOKEN,           'D', 'O', 'W', 'N', 'T', 'O',
    STEP_TOKEN,             'S', 'T', 'E', 'P',
    NEXT_TOKEN,             'N', 'E', 'X', 'T',
    REPEAT_TOKEN,           'R', 'E', 'P', 'E', 'A', 'T',
    UNTIL_TOKEN,            'U', 'N', 'T', 'I', 'L',
    BREAK_TOKEN,            'B', 'R', 'E', 'A', 'K',
    CONTINUE_TOKEN,         'C', 'O', 'N', 'T', 'I', 'N', 'U', 'E',
    ASM_TOKEN,              'A', 'S', 'M',
    DEF_TOKEN,              'D', 'E', 'F',
    EXPORT_TOKEN,           'E', 'X', 'P', 'O', 'R', 'T',
    IMPORT_TOKEN,           'I', 'M', 'P', 'O', 'R', 'T',
    INCLUDE_TOKEN,          'I', 'N', 'C', 'L', 'U', 'D', 'E',
    RETURN_TOKEN,           'R', 'E', 'T', 'U', 'R', 'N',
    END_TOKEN,              'E', 'N', 'D',
    DONE_TOKEN,             'D', 'O', 'N', 'E',
    LOGIC_NOT_TOKEN,        'N', 'O', 'T',
    LOGIC_AND_TOKEN,        'A', 'N', 'D',
    LOGIC_OR_TOKEN,         'O', 'R',
    BYTE_TOKEN,             'B', 'Y', 'T', 'E',
    WORD_TOKEN,             'W', 'O', 'R', 'D',
    CONST_TOKEN,            'C', 'O', 'N', 'S', 'T',
    STRUC_TOKEN,            'S', 'T', 'R', 'U', 'C',
    PREDEF_TOKEN,           'P', 'R', 'E', 'D', 'E', 'F',
    SYSFLAGS_TOKEN,         'S', 'Y', 'S', 'F', 'L', 'A', 'G', 'S',
    EOL_TOKEN
};

extern int outflags;

void parse_error(const char *errormsg)
{
    char *error_carrot = statement;

    fprintf(stderr, "\n%s %4d: %s\n%*s       ", filename, lineno, statement, (int)strlen(filename), "");
    for (error_carrot = statement; error_carrot != tokenstr; error_carrot++)
        putc(*error_carrot == '\t' ? '\t' : ' ', stderr);
    fprintf(stderr, "^\nError: %s\n", errormsg);
    exit(1);
}
void parse_warn(const char *warnmsg)
{
    if (outflags & WARNINGS)
    {
        char *error_carrot = statement;

        fprintf(stderr, "\n%s %4d: %s\n%*s       ", filename, lineno, statement, (int)strlen(filename), "");
        for (error_carrot = statement; error_carrot != tokenstr; error_carrot++)
            putc(*error_carrot == '\t' ? '\t' : ' ', stderr);
        fprintf(stderr, "^\nWarning: %s\n", warnmsg);
    }
}
int hexdigit(char ch)
{
    ch = toupper(ch);
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        return -1;
}

t_token scan(void)
{
    prevtoken = scantoken;
    /*
     * Skip whitespace.
     */
    while (*scanpos && (*scanpos == ' ' || *scanpos == '\t')) scanpos++;
    tokenstr = scanpos;
    /*
     * Scan for token based on first character.
     */
    if (scantoken == EOF_TOKEN)
        ;
    else if (*scanpos == '\0' || *scanpos == '\n' || *scanpos == ';')
        scantoken = EOL_TOKEN;
    else if ((scanpos[0] >= 'a' && scanpos[0] <= 'z')
          || (scanpos[0] >= 'A' && scanpos[0] <= 'Z')
          || (scanpos[0] == '_'))
    {
        /*
         * ID,  either variable name or reserved word.
         */
        int keypos = 0, matchpos = 0;

        do
        {
            scanpos++;
        }
        while ((*scanpos >= 'a' && *scanpos <= 'z')
            || (*scanpos >= 'A' && *scanpos <= 'Z')
            || (*scanpos == '_')
            || (*scanpos >= '0' && *scanpos <= '9'));
        scantoken = ID_TOKEN;
        tokenlen = scanpos - tokenstr;
        /*
         * Search for matching keyword.
         */
        while (keywords[keypos] != EOL_TOKEN)
        {
            while (keywords[keypos + 1 + matchpos] == toupper(tokenstr[matchpos]))
                matchpos++;
            if (IS_TOKEN(keywords[keypos + 1 + matchpos]) && (matchpos == tokenlen))
            {
                /*
                 * A match.
                 */
                scantoken = keywords[keypos];
                break;
            }
            else
            {
                /*
                 * Find next keyword.
                 */
                keypos  += matchpos + 1;
                matchpos = 0;
                while (!IS_TOKEN(keywords[keypos])) keypos++;
            }
        }
    }
    else if (scanpos[0] >= '0' && scanpos[0] <= '9')
    {
        /*
         * Number constant.
         */
        for (constval = 0; *scanpos >= '0' && *scanpos <= '9'; scanpos++)
            constval = constval * 10 + *scanpos - '0';
        scantoken = INT_TOKEN;
    }
    else if (scanpos[0] == '$')
    {
        /*
         * Hexadecimal constant.
         */
        constval = 0;
        while (scanpos++)
        {
            if (hexdigit(*scanpos) >= 0)
                constval = constval * 16 + hexdigit(*scanpos);
            else
                break;
        }
        scantoken = INT_TOKEN;
    }
    else if (scanpos[0] == '\'')
    {
        /*
         * Character constant.
         */
        scantoken = CHAR_TOKEN;
        if (scanpos[1] != '\\')
        {
            constval =  scanpos[1];
            if (scanpos[2] != '\'')
            {
                parse_error("Bad character constant");
                return (-1);
            }
            scanpos += 3;
        }
        else
        {
            switch (scanpos[2])
            {
                case 'n':
                    constval = 0x0D;
                    break;
                case 'r':
                    constval = 0x0A;
                    break;
                case 't':
                    constval = '\t';
                    break;
                case '\'':
                    constval = '\'';
                    break;
                case '\\':
                    constval = '\\';
                    break;
                case '0':
                    constval = '\0';
                    break;
                default:
                    parse_error("Bad character constant");
                    return (-1);
            }
            if (scanpos[3] != '\'')
            {
                parse_error("Bad character constant");
                return (-1);
            }
            scanpos += 4;
        }
    }
    else if (scanpos[0] == '\"') // Hack for string quote char in case we have to rewind later
    {
        int scanoffset;
        /*
         * String constant.
         */
        scantoken   = STRING_TOKEN;
        constval    = (long)strpos++;
        scanpos++;
        while (*scanpos &&  *scanpos != '\"')
        {
            if (*scanpos == '\\')
            {
                scanoffset = 2;
                switch (scanpos[1])
                {
                    case 'n':
                        *strpos++ = 0x0D;
                        break;
                    case 'r':
                        *strpos++ = 0x0A;
                        break;
                    case 't':
                        *strpos++ = '\t';
                        break;
                    case '\'':
                        *strpos++ = '\'';
                        break;
                    case '\"':
                        *strpos++ = '\"';
                        break;
                    case '\\':
                        *strpos++ = '\\';
                        break;
                    case '0':
                        *strpos++ = '\0';
                        break;
                    case '$':
                        if (hexdigit(scanpos[2]) < 0 || hexdigit(scanpos[3]) < 0) {
                            parse_error("Bad string constant");
                            return (-1);
                        }
                        *strpos++ = hexdigit(scanpos[2]) * 16 + hexdigit(scanpos[3]);
                        scanoffset = 4;
                        break;
                    default:
                        parse_error("Bad string constant");
                        return (-1);
                }
                scanpos += scanoffset;
            }
            else
                *strpos++ = *scanpos++;
        }
        if (!*scanpos)
        {
            parse_error("Unterminated string");
            return (-1);
        }
        *((unsigned char *)constval) = (long)strpos - constval - 1;
        *strpos++ = '\0';
        scanpos++;
    }
    else
    {
        /*
         * Potential two and three character tokens.
         */
        switch (scanpos[0])
        {
            case '>':
                if (scanpos[1] == '>')
                {
                    scantoken = SHR_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '=')
                {
                    scantoken = GE_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = GT_TOKEN;
                    scanpos++;
                }
                break;
            case '<':
                if (scanpos[1] == '<')
                {
                    scantoken = SHL_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '=')
                {
                    scantoken = LE_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '>')
                {
                    scantoken = NE_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = LT_TOKEN;
                    scanpos++;
                }
                break;
            case '=':
                if (scanpos[1] == '=')
                {
                    scantoken = EQ_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '>')
                {
                    scantoken = PTRW_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = SET_TOKEN;
                    scanpos++;
                }
                break;
            case '+':
                if (scanpos[1] == '+')
                {
                    scantoken = INC_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = ADD_TOKEN;
                    scanpos++;
                }
                break;
            case '-':
                if (scanpos[1] == '-')
                {
                    scantoken = DEC_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '>')
                {
                    scantoken = PTRB_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = SUB_TOKEN;
                    scanpos++;
                }
                break;
            case '/':
                if (scanpos[1] == '/')
                    scantoken = EOL_TOKEN;
                else
                {
                    scantoken = DIV_TOKEN;
                    scanpos++;
                }
                break;
            default:
                /*
                 * Simple single character tokens.
                 */
                scantoken = TOKEN(*scanpos++);
        }
    }
    tokenlen = scanpos - tokenstr;
    return (scantoken);
}
void scan_rewind(char *backptr)
{
    scanpos = backptr;
}
int scan_lookahead(void)
{
    char *backscan = scanpos;
    char *backtkn  = tokenstr;
    char *backstr  = strpos;
    int prevtoken  = scantoken;
    int prevlen    = tokenlen;
    int look       = scan();
    scanpos        = backscan;
    tokenstr       = backtkn;
    strpos         = backstr;
    scantoken      = prevtoken;
    tokenlen       = prevlen;
    return (look);
}
char inputline[512];
char conststr[1024];
int next_line(void)
{
    int len;
    t_token token;
    char* new_filename;
    strpos = conststr;
    if (inputfile == NULL)
    {
        /*
         * First-time init
         */
        inputfile = stdin;
        filename = "<stdin>";
    }
    if (*scanpos == ';')
    {
        statement = ++scanpos;
        scantoken = EOL_TOKEN;
    }
    else
    {
        statement = inputline;
        scanpos   = inputline;
        /*
         * Read next line from the current file, and strip newline from the end.
         */
        if (fgets(inputline, 512, inputfile) == NULL)
        {
            inputline[0] = 0;
            /*
             * At end of file, return to previous file if any, else return EOF_TOKEN
             */
            if (outer_inputfile != NULL)
            {
                fclose(inputfile);
                free(filename);
                inputfile = outer_inputfile;
                filename = outer_filename;
                lineno = outer_lineno - 1; // -1 because we're about to incr again
                outer_inputfile = NULL;
            }
            else
            {
                scantoken = EOF_TOKEN;
                return EOF_TOKEN;
            }
        }
        len = strlen(inputline);
        if (len > 0 && inputline[len-1] == '\n')
            inputline[len-1] = '\0';
        lineno++;
        scantoken = EOL_TOKEN;
        printf("; %s: %04d: %s\n", filename, lineno, inputline);
    }
    token = scan();
    /*
     * Handle single level of file inclusion
     */
    if (token == INCLUDE_TOKEN)
    {
        token = scan();
        if (token != STRING_TOKEN)
        {
            parse_error("Missing include filename");
            scantoken = EOF_TOKEN;
            return EOF_TOKEN;
        }
        if (outer_inputfile != NULL)
        {
            parse_error("Only one level of includes allowed");
            scantoken = EOF_TOKEN;
            return EOF_TOKEN;
        }
        outer_inputfile = inputfile;
        outer_filename = filename;
        outer_lineno = lineno;
        new_filename = (char *) malloc(*((unsigned char *)constval) + 1);
        strncpy(new_filename, (char *)(constval + 1), *((unsigned char *)constval) + 1);
        inputfile = fopen(new_filename, "r");
        if (inputfile == NULL)
        {
            parse_error("Error opening include file");
            scantoken = EOF_TOKEN;
            return EOF_TOKEN;
        }
        filename = new_filename;
        lineno = 0;
        return next_line();
    }
    return token;
}
