/***************************************************************
    This program leverages openEAR, the Munich open Emotion
    and Affect Recognition toolkit:

    Florian Eyben, Martin W�llmer, Bj�rn Schuller: 
    "openEAR - Introducing the Munich Open-Source Emotion and Affect Recognition Toolkit", 
    to appear in Proc. 4th International HUMAINE Association Conference on Affective Computing
    and Intelligent Interaction 2009 (ACII 2009), IEEE, Amsterdam, The Netherlands, 10.-12.09.2009
****************************************************************/
#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>

int main(int argc, char *argv[])
{
    char conf[1000];
	char *cmd = "openEAR-0.1.0/SMILExtract";
	char *nargv[5];

    if(!argv[1])
    {
        printf("usage: listen <option> [dir] [dir]\n");
        return 1;
    }

    if(argv[2] == "-L")
        // manual
        sprintf(conf, "-C config/emobase_live4.conf", argv[1]);
    else
        // manual
        sprintf(conf, "-C config/%s -I ../wav/%s -O output.csv", argv[1], argv[3], argv[4]);
    
    nargv[0] = cmd;
    nargv[1] = conf;
    nargv[2] = NULL;

	printf("[BEGIN]\n");
	printf("%s\n", conf);
	execvp(cmd, nargv);
	printf("[END]\n");
	return 0;
}