#ifndef INPUT3DO
#define INPUT3DO

typedef struct VECTOR
{
    float x, y, z;
}VECTOR;

typedef struct POLY
{
	char color;
	char ind_anz;
	short *i;
}POLY;

typedef struct O3D
{
	char name[32];
	short vert_anz;
	short poly_anz;
	short line_anz;
	short sibl_o;
	short child_o;
	VECTOR pos;
	VECTOR *n;
	VECTOR *vert;
	POLY *poly;
	short *line;
} O3D;

typedef struct MODEL
{
   	char obj_anz;
   	O3D  *o3d;
   	float size;
   	VECTOR min;
   	VECTOR max;
   	char camera;
   	char lemit;
   	char remit;
} MODEL;

int lese_3do(MODEL *model, const char datei[]); 

#endif
