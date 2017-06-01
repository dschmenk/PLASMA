#include <stdio.h>
#include "plasm.h"

int outflags = 0;

int main(int argc, char **argv)
{
    int j, i, flags = 0;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            j = 1;
            while (argv[i][j])
            {
                switch(argv[i][j++])
                {
                    case 'A':
                        outflags |= ACME;
                        break;
                    case 'M':
                        outflags |= MODULE;
                        break;
                    case 'O':
                        outflags |= OPTIMIZE;
                        break;
                    case 'W':
                        outflags |= WARNINGS;
                }
            }
        }
    }
    emit_flags(outflags);
    if (parse_module())
    {
        fprintf(stderr, "Compilation complete.\n");
    }
    return (0);
}
