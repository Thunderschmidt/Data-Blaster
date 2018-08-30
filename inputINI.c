#include <stdio.h>
#include <allegro.h>
#include <math.h>
#include "inputINI.h"



static int checkstr(char arg[], char *wort)
{
    char spalte=0;
    while (arg[spalte]!=0)
    {
        if (arg[spalte]!=wort[spalte]) return(0);
        spalte++;
    }
    return(1);
}
    
void read_ini(FILE *in, INIT *init)
{
    char zeile[256], wort0[128], wort1[128];
    char spalte0, spalte1, end=0;
    init->aufl_w=640;
    init->aufl_h=480;
    while(!feof(in))
    {
 
        spalte0=spalte1=0;
        fgets(zeile, 256, in);
        while(zeile[spalte0]!= 61)
        {
            if (zeile[spalte0]!= 32)
            {
                wort0[spalte1]=zeile[spalte0];
                if ((wort0[spalte1]>=64) && (wort0[spalte1]<=90)) wort0[spalte1]+=32;
                spalte1++;
            }
            spalte0++;
        }
        wort0[spalte1]=0; 
        spalte1=0;
        spalte0++;
 
        while(zeile[spalte0]!= ';')
        {
            if (zeile[spalte0]!= 32)
            {
                wort1[spalte1]=zeile[spalte0];
                if ((wort1[spalte1]>=64) && (wort1[spalte1]<=90)) wort1[spalte1]+=32;
                spalte1++;
            }
            spalte0++;
        }
        wort1[spalte1]=0; 
    
        if (checkstr("eyeseperation", wort0))
        {
            init->aa=atof(wort1);
        }
        else if (checkstr("monitorwidth", wort0))
        {
            init->mb=atof(wort1);
        }
        else if (checkstr("monitordistance", wort0))
        {
            init->db=atof(wort1);
        }
        else if (checkstr("colordepth", wort0))
        {
            init->farbtiefe=atoi(wort1);
        }
        else if (checkstr("z-buffer", wort0))
        {
            init->z_puffer=atoi(wort1);
        }
        else if (checkstr("nearclip", wort0))
        {
            init->near_clip=atof(wort1);
        }
        else if (checkstr("farclip", wort0))
        {
            init->far_clip=atof(wort1);
        }
        else if (checkstr("resolution640x480", wort0))
        {
            if (checkstr("1", wort1)){init->aufl_w=640;init->aufl_h=480;}
        }
        else if (checkstr("resolution800x600", wort0))
        {
            if (checkstr("1", wort1)){init->aufl_w=800;init->aufl_h=600;}
        }
        else if (checkstr("resolution1024x768", wort0))
        {
            if (checkstr("1", wort0)){init->aufl_w=1024;init->aufl_h=768;}
        }
        else if (checkstr("rem", wort0))
        {
        }
        else printf("!!! '%s' unbekannt !!!\n", wort0);
    }

    init->an=init->aa/2;
    init->zoom=2*atan((init->mb/2+init->an)/init->db)*57,29578;
    init->ppm=init->aufl_w;
    init->ppm=init->ppm/init->mb;
    init->aap=init->aa*init->ppm;
    init->aspect=init->aufl_w+init->aap;
    init->aspect=init->aspect/init->aufl_h;
}



void lese_ini(const char datei[], INIT *init)
{
   FILE *in = stdin;
	if ( (in = fopen(datei,"r")) == NULL )
	{
		fprintf(stderr, " Konnte Quelldatei nicht finden \n");
	}
	read_ini(in, init);
	fclose(in);
}
