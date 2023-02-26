#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "plasm.h"

int outflags = 0;
FILE *inputfile = NULL, *outputfile;
char *filename, asmfile[128], modfile[17];

int main(int argc, char **argv)
{
    int k, j, i, flags = 0;
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
                    case 'N':
                        outflags |= NO_COMBINE;
                        break;
                    case 'W':
                        outflags |= WARNINGS;
                        break;
                    case 'S':
                        outflags |= STREAM;
                        break;
                }
            }
        }
        else
        {
            if (inputfile == NULL)
            {
                filename = argv[i];
                inputfile = fopen(filename, "r");
                if (!inputfile)
                {
                    printf("Error opening input file: %s\n", filename);
                    exit(1);
                }
                strcpy(asmfile, filename);
                j = strlen(asmfile);
                while (j && asmfile[j] != '.')
                {
                    asmfile[j] = '\0';
                    j--;
                }
                asmfile[j+1] = 'a';
                while (j && asmfile[j-1] != '/')
                    j--;
                k = 1;
                while (asmfile[j] != '.')
                    modfile[k++] = toupper(asmfile[j++]);
                modfile[k] = '\0';
                modfile[0] = k - 1;
                if (outflags & STREAM)
                {
                    strcpy(asmfile, "STDOUT");
                    outputfile = stdout;
                }
                else
                {
                    outputfile = fopen(asmfile, "w");
                    if (!outputfile)
                    {
                        printf("Error opening output file %s\n", asmfile);
                        exit(1);
                    }
                }
            }
            else
            {
                printf("Too many input files\n");
                exit(1);
            }
        }
    }
    if (inputfile == NULL)
    {
        printf("Usage: %s [-AMONWS] <inputfile>\n", argv[0]);
        return (0);
    }
    emit_flags(outflags);
    if (parse_module())
    {
        fprintf(stderr, "Compilation complete.\n");
    }
    fclose(inputfile);
    fclose(outputfile);
    return (0);
}
