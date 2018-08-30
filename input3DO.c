#include <stdio.h>
#include <allegro.h>
#include "input3do.h"
#define SCALE 0.4

typedef struct TAGOBJECT
{
    long VersionSignature;			//  0
    long NumberOfVertexes;			//  4
    long NumberOfPrimitives;		//  8
    long OffsetToselectionPrimitive;// 12
    fixed XFromParent;				// 16
    fixed YFromParent;				// 20
    fixed ZFromParent;				// 24
    long OffsetToObjectName;		// 28
    long Always_0;				    // 32
    long OffsetToVertexArray;		// 36
    long OffsetToPrimitiveArray;	// 40
    long OffsetToSiblingObject;		// 44
    long OffsetToChildObject;		// 48
}TAGOBJECT;

typedef struct TAGPRIMITIVE
{
    long ColorIndex;
    long NumberOfVertexi;
    long Always_0;
    long OffsetToVertexIndexArray;
    long OffsetToTextureName;
    long Unknown_1;
    long Unknown_2;
    long Unknown_3;    
}TAGPRIMITIVE;

typedef struct TAGVERTEX
{
    fixed x;
    fixed y;
    fixed z;
}TAGVERTEX;


TAGVERTEX       *tagVertex;
O3D             *o3d;
TAGPRIMITIVE    tagPrimitive;
TAGOBJECT       tagObject;
short           obj_anz,
                objnr=0,
                lemit,
                remit;
static short    camera;
                
long filesize;
VECTOR min, max;

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

long checkoffset(long offset)
{
    if((offset<0)||(offset >=filesize))
    {
        printf("\n !!! OFFSET READ ERROR  !!!");
        return(0);
    }
    return(offset);
}
void count_object(long offset, FILE *in)
{
    long child, sibl, arr[60];
	fseek(in, offset, 0);
	fread(&tagObject.VersionSignature, 4, 13, in);
	sibl=checkoffset(tagObject.OffsetToSiblingObject); 
	child=checkoffset(tagObject.OffsetToChildObject); 
   	obj_anz++;
   	if (child > 0) count_object(child, in);
	if (sibl > 0) count_object(sibl, in);
}



void read_object(long offset, FILE *in, short o)
{
	int i, v;
	long child, sibl;

/****************************LESE HEADER****************************/

	fseek(in, offset, 0);										//lese Object-Header
	fread(&tagObject.VersionSignature, sizeof(tagObject), 1, in);

	o3d[o].pos.x=fixtof(tagObject.XFromParent)*SCALE;
	o3d[o].pos.y=fixtof(tagObject.YFromParent)*SCALE;
	o3d[o].pos.z=fixtof(tagObject.ZFromParent)*SCALE;
	o3d[o].vert_anz=tagObject.NumberOfVertexes;
	o3d[o].poly_anz=tagObject.NumberOfPrimitives;
	o3d[o].sibl_o=0;
	o3d[o].child_o=0;
	fseek(in, tagObject.OffsetToObjectName,0);
	fgets(o3d[o].name, 32, in);									//lese Namen
	if (checkstr("camera", o3d[o].name)) camera=o;                   
	else if (checkstr("lemit", o3d[o].name)) lemit=o;
	else if (checkstr("remit", o3d[o].name)) remit=o;
	

/***********************LESE PUNKTKOORDINATEN***********************/

	tagVertex=(TAGVERTEX*)malloc(tagObject.NumberOfVertexes*sizeof(TAGVERTEX));		  //öffne Zwischenppeicher für Punktkoordinaten
	o3d[o].vert=(VECTOR*)malloc(tagObject.NumberOfVertexes*sizeof(VECTOR));	  //öffne Speicher für Punktkoordinaten
	fseek(in, tagObject.OffsetToVertexArray, 0);						  //springe zum Punktkoordinaten-Offset
	fread(&tagVertex[0].x, sizeof(TAGVERTEX), tagObject.NumberOfVertexes, in);//lese Punktkoordinaten ein 

	for (v=0; v<tagObject.NumberOfVertexes; v++)
	{
		if((o3d[o].vert[v].x=fixtof(tagVertex[v].x)*SCALE)>max.x) max.x=o3d[o].vert[v].x;
		if(o3d[o].vert[v].x<min.x) min.x=o3d[o].vert[v].x;
		if((o3d[o].vert[v].y=fixtof(tagVertex[v].y)*SCALE)>max.y) max.y=o3d[o].vert[v].y;
		if(o3d[o].vert[v].y<min.y) min.y=o3d[o].vert[v].y;
		if((o3d[o].vert[v].z=fixtof(tagVertex[v].z)*SCALE)>max.z) max.z=o3d[o].vert[v].z;
		if(o3d[o].vert[v].z<min.z) min.z=o3d[o].vert[v].z;
	}
	free(tagVertex);

/*************************LESE INDEX-ARRAY**************************/

	o3d[o].poly=(POLY*)malloc(tagObject.NumberOfPrimitives*sizeof(POLY));				//öffne Speicher für 3D-Primitives
	for (i=0; i<tagObject.NumberOfPrimitives; i++)						
	{
		fseek(in, tagObject.OffsetToPrimitiveArray+sizeof(TAGPRIMITIVE)*i, 0);	//springe zum 3D-Primitive-Offset
		fread(&tagPrimitive.ColorIndex, sizeof(TAGPRIMITIVE), 1, in);			//lese 3D-Primitives ein 
		o3d[o].poly[i].color=15-(tagPrimitive.ColorIndex-80);							//die Farbe
        o3d[o].poly[i].ind_anz=tagPrimitive.NumberOfVertexi;				//meistens 4
		o3d[o].poly[i].i=(short*)malloc(tagPrimitive.NumberOfVertexi*sizeof(short));		//öffne Speicher für Punkt-Indizes
		fseek(in, tagPrimitive.OffsetToVertexIndexArray, 0);					//springe zum Vertex-Index-Array
		fread(&o3d[o].poly[i].i[0], sizeof(short),	                    //lese Vertex-Index-Arrays ein 
              tagPrimitive.NumberOfVertexi, in);
	}

/*************************LESE UNTEROBJEKTE*************************/

	sibl=checkoffset(tagObject.OffsetToSiblingObject); 
	child=checkoffset(tagObject.OffsetToChildObject);
 
  	if (child> 0)   //lese untergeordnetes Objekt
	{
		objnr++;
		o3d[o].child_o=objnr;
		read_object(child, in, objnr);
	} 
	if (sibl > 0)   //lese gleichwertiges Objekt
	{
		objnr++;
		o3d[o].sibl_o=objnr;
		read_object(sibl, in, objnr);
	}
}


char line_ok(int l, int i0, int i1, short *line)
{
	int z;
	for(z=0; z<l; z++)
	{
		if  (i0==line[z*2]) if (i1==line[z*2+1])return(0);
		if  (i0==line[z*2+1]) if (i1==line[z*2])return(0);
		if  (i0==i1) return(0);
	}
	return(1);
}

void find_lines()
{
	int o, i, p, l, i0, i1, error;
	for (o=0; o<obj_anz; o++)
	{
        o3d[o].line=(short*)malloc(o3d[o].poly_anz*16);
	    l=0;	  
	    for (p=0; p<o3d[o].poly_anz; p++)
	    {
			for (i=0; i<o3d[o].poly[p].ind_anz; i++)
			{
			   error=0;
			   i0=o3d[o].poly[p].i[i];
			   if ((i0>=o3d[o].vert_anz)||(i0 < 0))
			   {
                  printf("\n !!! READ ERROR in object %d !!!", o);
                  o3d[o].poly[p].i[i]=0;
                  i0=0;
                  error=1;
               }
               if ((i+1)==o3d[o].poly[p].ind_anz) i1=o3d[o].poly[p].i[0];
               else i1=o3d[o].poly[p].i[i+1];
      		   if ((i1>=o3d[o].vert_anz)|| (i1 < 0))
			   {
                  printf("\n !!! READ ERROR in object %d !!!", o);
                  o3d[o].poly[p].i[i+1]=0;
                  i0=1;
                  error=1;
               }
               if (!error && line_ok(l, i0, i1, o3d[o].line))
               {   
                  o3d[o].line[l*2]=i0;
                  o3d[o].line[l*2+1]=i1;
//                printf("%d %d %d \n", o, i0, i1);
                  l++;
               }
//             else printf("--------->%d %d %d \n", o, i0, i1);
            }
//          printf("\n");

        }
        o3d[o].line_anz=l;
        realloc(o3d[o].line, o3d[o].line_anz*4);
  }

}


void zeige_daten()
{
	int i, j, k;
	printf("\n Objekte insgesamt: %d\n", obj_anz);
	printf("\n Dateigroesse     : %d\n", filesize);
	printf(" X-Ausdehnung     : %f\n", max.x-min.x);
	printf(" Y-Ausdehnung     : %f\n", max.y-min.y);
	printf(" Z-Ausdehnung     : %f\n", max.z-min.z);
	printf("\n camera= %d\n", camera);
	printf(" lemit= %d\n", lemit);
	printf(" remit= %d\n", remit);
	
	for (i=0; i < obj_anz;i++)
	{
		printf(" \n Name             :        %s", o3d[i].name);
		printf(" \n Objekt-o.       :        %d", i);
		printf(" \n Punkt-Anzahl     :        %d", o3d[i].vert_anz);
		printf(" \n Linien-Anzahl    :        %d", o3d[i].line_anz);
		printf(" \n Polygon-Anzahl   :        %d", o3d[i].poly_anz);
		printf(" \n Nebenobjekt-o.  :        %d", o3d[i].sibl_o);
		printf(" \n Unterobjekt-o.  :        %d\n", o3d[i].child_o);
	}
}

void calc_normals()
{
    int o, p;
   	for (o=0; o<obj_anz; o++)
   	{
   	        o3d[o].n=(VECTOR*)malloc(o3d[o].poly_anz*sizeof(VECTOR));
            for(p=0; p<o3d[o].poly_anz; p++)
            {
            if(o3d[o].poly[p].ind_anz>=3)
            {
            
              cross_product_f
              (o3d[o].vert[o3d[o].poly[p].i[1]].x-o3d[o].vert[o3d[o].poly[p].i[0]].x,
               o3d[o].vert[o3d[o].poly[p].i[1]].y-o3d[o].vert[o3d[o].poly[p].i[0]].y,
               o3d[o].vert[o3d[o].poly[p].i[1]].z-o3d[o].vert[o3d[o].poly[p].i[0]].z,
               o3d[o].vert[o3d[o].poly[p].i[2]].x-o3d[o].vert[o3d[o].poly[p].i[0]].x,
               o3d[o].vert[o3d[o].poly[p].i[2]].y-o3d[o].vert[o3d[o].poly[p].i[0]].y,
               o3d[o].vert[o3d[o].poly[p].i[2]].z-o3d[o].vert[o3d[o].poly[p].i[0]].z,
               &o3d[o].n[p].x,
               &o3d[o].n[p].y,
               &o3d[o].n[p].z);
              normalize_vector_f(&o3d[o].n[p].x, &o3d[o].n[p].y, &o3d[o].n[p].z);
            }
            else
            {
                printf("!!! Normalvektor nicht errechenbar (Polygon < 3 Punkte)!!!\n");
                o3d[o].n[p].x=0;
                o3d[o].n[p].y=0;
                o3d[o].n[p].z=0;
            }
            }
    }
}

int lese_3do(MODEL *model, const char datei[])
{
    objnr=0;
    min.x=0;min.y=0; min.z=0;max=min;
    obj_anz=0;
    camera=0;
    lemit=0;
    remit=0;
	FILE *in = stdin;
	if ( (in = fopen(datei,"rb")) == NULL )
	{
		fprintf(stderr, " Konnte Quelldatei nicht finden \n");
	}
	fseek(in, 0, SEEK_END);
	filesize=ftell(in);
	count_object(0, in);
	o3d=(O3D*)malloc(obj_anz*sizeof(O3D));
	read_object(0, in, 0);
	fclose(in);
	find_lines();
   	calc_normals();
    model->obj_anz = obj_anz;
    model->o3d = o3d;
    model->max = max;
    model->min = min;
    model->size=max.x-min.x;
    model->camera = camera;
    model->lemit = lemit;
    model->remit = remit;
    if(model->size<(max.y-min.y)) model->size=max.y-min.y;
    if(model->size<(max.z-min.z)) model->size=max.z-min.z;
  //	zeige_daten();
	return(0);
}
