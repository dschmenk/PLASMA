#include <stdio.h>
#include <stdlib.h>

#define PCM4(p) ((p) < 0xF8 ? (((p) + 8) >> 4) : 0x0F)

int main(int argc, char **argv)
{
    FILE *fp;
    char *wavfile = NULL;
    char *pcmfile = "out.pcm";
    unsigned char header[44], *samples, *rlepcm;
    unsigned int wavlen, pcmlen, prevpcm, pcm, run;
    int i;

    if (argc > 1)
    {
        wavfile = argv[1];
        if (!(fp = fopen(wavfile, "rb")))
        {
            fprintf(stderr, "Ubale to open %s.\n", wavfile);
            exit(-1);
        }
        if (argc > 2)
            pcmfile = argv[2];
    }
    else
    {
        fp = stdin;
    }
    fread(&header, 44, 1, fp);
    if (header[0] == 'R' && header[12] == 'f' && header[20] == 1 && header[34] == 8)
    {
        wavlen  = (header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24));
        fprintf(stderr, "WAV length: %d\n", wavlen);
        samples = malloc(wavlen);
        rlepcm  = malloc(wavlen);
        fread(samples, wavlen, 1, fp);
    }
    else
    {
        fprintf(stderr, "Not 8 bit/samples mono WAV header.\n");
        exit(-1);
    }
    if (wavfile)
        fclose(fp);
    if (!(fp = fopen(pcmfile, "wb")))
    {
        fprintf(stderr, "Unable to open %s for writing.\n", pcmfile);
        exit(-1);
    }
    prevpcm = PCM4(samples[0]);
    run     = 1;
    pcmlen  = 0;
    for (i = 1; i < wavlen; i++)
    {
        pcm = PCM4(samples[i]);
        if (pcm == prevpcm && run < 16)
            run++;
        else
        {
            rlepcm[pcmlen++] = ((run - 1) << 4) | prevpcm;
            prevpcm = pcm;
            run     = 1;
        }
    }
    fprintf(stderr, "RLE PCM length: %d\n", pcmlen);
    fwrite(rlepcm, pcmlen, 1, fp);
    fclose(fp);
    return 0;
}