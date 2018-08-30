#include <stdio.h>
#include <math.h>
#include <allegro.h>
#include <alleggl.h>
#include <GL/glu.h>
#include "input3DO.h"
#include "konstanten.h"
#include "piles.h"
#include "inputINI.h"

#define GRIDSIZE_X   4
#define GRIDSIZE_Y   3
#define GRIDSIZE_Z   64

int pile_counter;


float game_speed=1;
int speed_counter;
int proceeder=0;
int step_counter=980;
int cursor_x, cursor_y;
int sec;
int sec_sub;
bool gameovr=FALSE;

struct camera
{
    VECTOR stereo;
    VECTOR pos;
    MATRIX_f m;
    MATRIX_f md;
    VECTOR pos_ist, pos_soll;
} camera;

struct shots
{
    int x, y;
    float z;
} shots[MAXSHOTS];
int shotsactive=0;


INIT init;


float        fps_rate = 60.0,
             sinus10[36][2],					//Daten für Zielmonitor
             yrot=0, fmult, fcount;
int          frame_count = 0,
             frame_count_time = 0,
             input_delay=0,
             button_delay=0;
volatile int chrono = 0;
short        jc[3];
bool         joyda=0,
             button_pressed=0;
char         crosscolor[CROSSBOX*CROSSBOX*CROSSBOX];
GLubyte      circleindex[36];

GLuint  tex;
GLuint  r_kam_list;
GLuint  l_kam_list;
GLuint  gridstrip_list;
GLuint  draw_model_list;
GLfloat glm[16];
SAMPLE *sound[11];

float gridscale;

struct pile
{
    int data[8][6];
    int pos;
    bool active;
} pile[MAXPILES];

int proc_counter=0;

struct colmap
{
    int depth[GRIDSIZE_X*2][GRIDSIZE_Y*2];
    bool active[GRIDSIZE_X*2][GRIDSIZE_Y*2];
    char pile[GRIDSIZE_X*2][GRIDSIZE_Y*2];

}colmap;

struct explosion
{
    bool active;
    char property;
    short anz;
    int t;
    float partikel[64];
    float mult;
    float speed;
    VECTOR pos;
    MATRIX_f m;
}explosion[MAXEXPLODES];
VECTOR partikel[64];

int points=0, lifes=3;
MODEL       model[10];
MODEL       font3d;


void draw_object(short mod, short o, char c)
{
    int i, p;
 	for (p = 0; p < model[mod].o3d[o].poly_anz; p++)
	{ 
	    glNormal3fv(&model[mod].o3d[o].n[p].x);
		glColor3ubv(col[c][model[mod].o3d[o].poly[p].color]);
		glVertexPointer(3, GL_FLOAT, 0, model[mod].o3d[o].vert);
		glDrawElements(GL_POLYGON, model[mod].o3d[o].poly[p].ind_anz, GL_UNSIGNED_SHORT, model[mod].o3d[o].poly[p].i);
	}
}


void draw_char(unsigned char a)
{
   int i;
   if((a >= 33) && (a <=96)) a-= 33;
   else if((a >= 97) && (a <=122)) a-= 65;
   else if((a >= 123) && (a <=128)) a-= 63;
   else if (a=='ß') a=71;
   else if (a=='ä' ||a=='Ä') a=72;
   else if (a=='ö' ||a=='Ö') a=73;
   else if (a=='ü' ||a=='Ü') a=74;
   else if (a==223) a=73;
   glVertexPointer(3, GL_FLOAT, 0, font3d.o3d[a].vert);
   for(i=0;i<font3d.o3d[a].poly_anz;i++)
   glDrawElements(GL_QUADS, 4, GL_UNSIGNED_SHORT, font3d.o3d[a].poly[i].i);
}

void draw_string(char string[])
{
    int i=0, column=0;
    while(string[i]!='\0')
    {
       if(string[i]=='\n')
       {
          glTranslatef(-column*4,-6, 0);
          column=0;
       }
       else if(string[i]==' ')
       {
          glTranslatef(4,0,0);
          column++;
       }
       else
       {
          draw_char(string[i]);
          glTranslatef(4,0,0);
          column++;
       }
       i++;       
    }
}



void mouseclick()
{
    do
    {
    poll_mouse();
  	poll_joystick();
  	}
    while(!((mouse_b & 1) || joy[0].button[0].b || joy[0].button[1].b || joy[0].button[2].b || joy[0].button[3].b));
    do
    {
    poll_mouse();
    poll_joystick();
    }
    while((mouse_b & 1) || joy[0].button[0].b || joy[0].button[1].b || joy[0].button[2].b || joy[0].button[3].b);
    play_sample (sound[1], 64, 128, 1000, 0);

    return;
}

void draw_integer(int number)
{
   int a;
   int i=0, p;

   char str[16], b[2], c;
   b[1]='\n';
   sprintf(str, "%d", number);
   b[0]=str[0];
    while(b[0]!='\0')
    {
       a=atoi(b);
 	   for (p = 0; p < model[0].o3d[a].poly_anz; p++)
	   { 
		  glVertexPointer(3, GL_FLOAT, 0, model[0].o3d[a].vert);
		  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, model[0].o3d[a].poly[p].i);
	   }
	   glTranslatef(8,0,0);
	   i++;
       b[0]=str[i];
    }
}



void init_sound()
{
    if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL) < 0)
    {
       printf ("Error initializing sound card");
    }	
    sound[0] = load_wav ("sounds/hubschuss.wav");
    sound[1] = load_wav ("sounds/hihat.wav");
    sound[2] = load_wav ("sounds/synsam.wav");
    sound[3] = load_wav ("sounds/gameover.wav");
    sound[4] = load_wav ("sounds/hae.wav");
    sound[5] = load_wav ("sounds/katakis14.wav");
    sound[6] = load_wav ("sounds/katakis12.wav");
    sound[7] = load_wav ("sounds/katakis13.wav");
    sound[8] = load_wav ("sounds/neu.wav");
    sound[9] = load_wav ("sounds/melo1.wav");
    sound[10] = load_wav ("sounds/melo2.wav");

}

void destroy_samples()
{
    int i;
    for (i=0;i<11;i++)destroy_sample(sound[i]);
}

void ende()
{
    destroy_samples();
	allegro_exit();
}
void wait(int time)
{
    int t=chrono;
    while((chrono-time)<t){}
} 

void glMultMatrix_allegro(MATRIX_f *m)
{
    allegro_gl_MATRIX_f_to_GLfloat(m, glm);
    glMultMatrixf(glm);
}

void the_timer(void)
{
	chrono++;
} END_OF_FUNCTION(the_timer);

void invert_matrix(MATRIX_f *in, MATRIX_f *out)
{
	out->v[0][0]=in->v[0][0]; out->v[0][1]=in->v[1][0]; out->v[0][2]=in->v[2][0];
	out->v[1][0]=in->v[0][1]; out->v[1][1]=in->v[1][1]; out->v[1][2]=in->v[2][1];
	out->v[2][0]=in->v[0][2]; out->v[2][1]=in->v[1][2]; out->v[2][2]=in->v[2][2];
}

void draw_grid_strip()
{
   int x, y;
      glBegin(GL_LINE_LOOP);
      glVertex3f(-GRIDSIZE_X*gridscale, GRIDSIZE_Y*gridscale,0);
      glVertex3f(-GRIDSIZE_X*gridscale,-GRIDSIZE_Y*gridscale,0);
      glVertex3f( GRIDSIZE_X*gridscale,-GRIDSIZE_Y*gridscale,0);
      glVertex3f( GRIDSIZE_X*gridscale, GRIDSIZE_Y*gridscale,0);
      glEnd();
   glBegin(GL_LINES);
   for(x=-GRIDSIZE_X; x<GRIDSIZE_X; x++)
   {
      glVertex3f( x*gridscale, GRIDSIZE_Y*gridscale,0*gridscale);
      glVertex3f( x*gridscale, GRIDSIZE_Y*gridscale,-1*gridscale);
      glVertex3f( x*gridscale,-GRIDSIZE_Y*gridscale,0*gridscale);
      glVertex3f( x*gridscale,-GRIDSIZE_Y*gridscale,-1*gridscale);
  }
   for(y=-GRIDSIZE_Y; y<GRIDSIZE_Y; y++)
   {
      glVertex3f(-GRIDSIZE_X*gridscale, y*gridscale,0*gridscale);
      glVertex3f(-GRIDSIZE_X*gridscale, y*gridscale,-1*gridscale);
      glVertex3f( GRIDSIZE_X*gridscale, y*gridscale,0*gridscale);
      glVertex3f( GRIDSIZE_X*gridscale, y*gridscale,-1*gridscale);
   }
   glEnd();
}

void draw_grid()
{
   int x, y, z;
   for(z=0; z<GRIDSIZE_Z; z++)
   {
      glBegin(GL_LINE_LOOP);
      glVertex3i(-GRIDSIZE_X, GRIDSIZE_Y,-z);
      glVertex3i(-GRIDSIZE_X,-GRIDSIZE_Y,-z);
      glVertex3i( GRIDSIZE_X,-GRIDSIZE_Y,-z);
      glVertex3i( GRIDSIZE_X, GRIDSIZE_Y,-z);
      glEnd();
   }
   glBegin(GL_LINES);
   for(x=-GRIDSIZE_X; x<GRIDSIZE_X; x++)
   {
      glVertex3i( x, GRIDSIZE_Y,0);
      glVertex3i( x, GRIDSIZE_Y,-GRIDSIZE_Z);
      glVertex3i( x,-GRIDSIZE_Y,0);
      glVertex3i( x,-GRIDSIZE_Y,-GRIDSIZE_Z);
  }
   for(y=-GRIDSIZE_Y; y<GRIDSIZE_Y; y++)
   {
      glVertex3i(-GRIDSIZE_X, y,0);
      glVertex3i(-GRIDSIZE_X, y,-GRIDSIZE_Z);
      glVertex3i( GRIDSIZE_X, y,0);
      glVertex3i( GRIDSIZE_X, y,-GRIDSIZE_Z);
   }
   glEnd();
}

void init_lighting()
{
    GLfloat lmodel_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_position[] = {1, 1, 1, 0};
    GLfloat light_specular[] = {0, 0, 0, 0};
    GLfloat light_ambient[] = {0.9, 1, 1, 1};
    GLfloat light_diffuse[] = {0.1, 0, 0, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

 void set_view_l()
{
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient_l);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse_l);
	glViewport(-init.aap, 0, SCREEN_W+init.aap, SCREEN_H);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(init.zoom, init.aspect, 0.01, init.far_clip);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
//	glTranslatef(init.an, 0, 0);
}

void move_camera_l()
{
    glMultMatrix_allegro(&camera.m);
	glTranslatef(	-camera.pos_ist.x+init.an,
			        -camera.pos_ist.y,
		            -camera.pos.z);
    glPushMatrix();
}

void set_view_r()
{
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient_r);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse_r);
	glViewport(0,  0, SCREEN_W+init.aap, SCREEN_H);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(init.zoom, init.aspect, 0.01, init.far_clip);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
//	glTranslatef(-init.an, 0, 0);

}

void move_camera_r()
{
    glMultMatrix_allegro(&camera.m);
	glTranslatef(	-camera.pos_ist.x-init.an,
			        -camera.pos_ist.y,
		            -camera.pos.z);
    glPushMatrix();
}
void accelerate(float soll, float *ist, float a)
{
	a*=fmult;
	if (*ist<soll )
	{
		*ist+=a;
		if (*ist > soll) *ist=soll;
	}
	if (*ist > soll)
	{
		*ist-=a;
		if (*ist < soll) *ist=soll;
	}
}
void draw_complete_grid(bool c)
{
    int z;
    for (z=0; z<GRIDSIZE_Z; z++)
    {
       glColor3ubv(col[c][14-z/6]);
       glPushMatrix();
       glTranslatef(0,0,-z*gridscale);
	   glCallList(gridstrip_list);
       glPopMatrix();
    }
}
void draw_box(bool c, int i)
{
    glColor3ubv(col[c][10+i]); //vorne

    glBegin(GL_QUADS);
       glVertex3i(0,0,1);
       glVertex3i(1,0,1);
       glVertex3i(1,1,1);
       glVertex3i(0,1,1);

     
 //     glVertex3i(0,0,1);
 //     glVertex3i(0,1,1);
  //    glVertex3i(1,1,1);
  //    glVertex3i(1,0,1);
     glColor3ubv(col[c][5+i]);  //links


       glVertex3i(0,0,0);
       glVertex3i(0,0,1);
       glVertex3i(0,1,1);
       glVertex3i(0,1,0);


      glVertex3i(1,0,0);  //rechts
       glVertex3i(1,1,0);
       glVertex3i(1,1,1);
       glVertex3i(1,0,1);
     glColor3ubv(col[c][7+i]);  

       glVertex3i(0,0,0); //unten
       glVertex3i(1,0,0);
       glVertex3i(1,0,1);
       glVertex3i(0,0,1);

       glVertex3i(0,1,0);  //oben
       glVertex3i(0,1,1);
       glVertex3i(1,1,1);
       glVertex3i(1,1,0);
    glEnd();
}

void draw_light_box(bool c)
{
    glColor3ubv(col[c][15]); //vorne

    glBegin(GL_QUADS);
       glVertex3i(0,0,1);
       glVertex3i(1,0,1);
       glVertex3i(1,1,1);
       glVertex3i(0,1,1);

     glColor3ubv(col[c][10]);  //links


       glVertex3i(0,0,0);
       glVertex3i(0,0,1);
       glVertex3i(0,1,1);
       glVertex3i(0,1,0);


      glVertex3i(1,0,0);  //rechts
       glVertex3i(1,1,0);
       glVertex3i(1,1,1);
       glVertex3i(1,0,1);
       glColor3ubv(col[c][11]);  

       glVertex3i(0,0,0); //unten
       glVertex3i(1,0,0);
       glVertex3i(1,0,1);
       glVertex3i(0,0,1);

       glVertex3i(0,1,0);  //oben
       glVertex3i(0,1,1);
       glVertex3i(1,1,1);
       glVertex3i(1,1,0);
    glEnd();
}

void just_draw_box()
{
    glBegin(GL_QUADS);
       glVertex3i(0,0,1);
       glVertex3i(1,0,1);
       glVertex3i(1,1,1);
       glVertex3i(0,1,1);

       glVertex3i(0,0,0);
       glVertex3i(0,0,1);
       glVertex3i(0,1,1);
       glVertex3i(0,1,0);

       glVertex3i(1,0,0);  //rechts
       glVertex3i(1,1,0);
       glVertex3i(1,1,1);
       glVertex3i(1,0,1);

       glVertex3i(0,0,0); //unten
       glVertex3i(1,0,0);
       glVertex3i(1,0,1);
       glVertex3i(0,0,1);

       glVertex3i(0,1,0);  //oben
       glVertex3i(0,1,1);
       glVertex3i(1,1,1);
       glVertex3i(1,1,0);
    glEnd();
}

int count_pile(char a)
{
    int x,y, i=0;
    for (x=0; x<(GRIDSIZE_X*2); x++)
    for (y=0; y<(GRIDSIZE_Y*2); y++)
    i+=pile[i].data[x][y];
    return(i);
}
void init_explode()
{
    int i;
   	for (i=0; i<64; i++)
	{
		partikel[i].x=0; 
		while (((partikel[i].x*partikel[i].x + 
                 partikel[i].y*partikel[i].y +
                 partikel[i].z*partikel[i].z  ) > 262144) || !partikel[i].x)
		{
      			partikel[i].x = rand() % 1024 - 512;
      			partikel[i].y = rand() % 1024 - 512;
      			partikel[i].z = rand() % 1024 - 512;
		}
		normalize_vector_f(&partikel[i].x, &partikel[i].y, &partikel[i].z);
	}
}

void start_explode(float x, float y, float z)
{
    int i, j;
	for (i=0; i<MAXEXPLODES; i++)
	{
		if (!explosion[i].active)
		{
		    explosion[i].active=1;
		    explosion[i].pos.x=x;
		    explosion[i].pos.y=y;
		    explosion[i].pos.z=z;
			explosion[i].t=chrono;
			explosion[i].mult=0.1;
			explosion[i].anz=40;

			for(j=0;j<explosion[i].anz;j++)
            {explosion[i].partikel[j]=rand() % 1024;explosion[i].partikel[j]/=2048;}
			break;
		}
	}
}

void draw_explode(int j, char c)
{
    int i;
    float mult;
   	glColor3ubv(col[c][14]);
   	for (i=0; i<explosion[j].anz;i++)
   	{
   	   mult=explosion[j].partikel[i]*explosion[j].mult;
   	   glTranslatef(partikel[i].x*mult, partikel[i].y*mult, partikel[i].z*mult);
   	   glBegin(GL_QUADS);
   	   glVertex3f(-0.05, 0.05,0);
   	   glVertex3f(-0.05,-0.05,0);
   	   glVertex3f( 0.05,-0.05,0);
   	   glVertex3f( 0.05, 0.05,0);
   	   glEnd();
    }
}

void proceed_explode()
{
    int i, j;
    float add;
  	for (i=0; i<MAXEXPLODES; i++)
	{
		if (explosion[i].active)
		{
		  add=(chrono-explosion[i].t+400);
		  add=0.8/add;
		  accelerate(0, &explosion[i].speed, 30);
		  if (rand() % 3 ==1) explosion[i].anz--;
		  for(j=0;j<explosion[i].anz;j++) explosion[i].mult+=add;
		  if ((chrono-explosion[i].t)>1800)explosion[i].active=0;
        }
    }
}

void clear_pile(char i)
{
    int x, y, z;
    for (x=0; x<(GRIDSIZE_X*2); x++)
    for (y=0; y<(GRIDSIZE_Y*2); y++)
    pile[i].data[x][y]=0;
    pile[i].pos=-10000;
    pile[i].active=0;
}



void make_pile(char depth, int pos)
{
    int x, y, j, k, i=rand()%8+1, l;
    for(l=0;l<MAXPILES;l++)
    if(!pile[l].active)
    {
       pile[l].pos=pos;
       pile[l].active=1;
       for (x=0; x<(GRIDSIZE_X*2); x++)
       for (y=0; y<(GRIDSIZE_Y*2); y++)
       {
         j=rand()%depth+1;
         pile[l].data[x][y]=piledata[i][y][x]*j;
       }
       break;          
    }
}

void clear_piles()
{
 int i;
 for(i=0;i++;i<MAXPILES)
 clear_pile(i);
}
   
void clear_colmap()
{
    char x,y;
    for (x=0; x<(GRIDSIZE_X*2); x++)
    for (y=0; y<(GRIDSIZE_Y*2); y++)
    {
    colmap.active[x][y]=0;
    colmap.depth[x][y]=-100000;
    colmap.pile[x][y]=0;
    }
}

void make_colmap()
{
   char x,y,z,i;
   clear_colmap();
   for(i=0; i<MAXPILES; i++)
   if(pile[i].active)
   {
       for (x=0; x<(GRIDSIZE_X*2); x++)
       for (y=0; y<(GRIDSIZE_Y*2); y++)
       if(pile[i].data[x][y])
       {
       if((pile[i].data[x][y]+pile[i].pos)>colmap.depth[x][y])
       {
          colmap.depth[x][y]=pile[i].data[x][y]+pile[i].pos;
          colmap.pile[x][y]=i;
          colmap.active[x][y]=1;
       }
       }
   }
}


void start_shot()  //Für die Schüsse ist die Bildschirmebene null
{
    shots[shotsactive].x=cursor_x;
    shots[shotsactive].y=cursor_y;
    shots[shotsactive].z=0;
    shotsactive++;
}
void delete_shot(int i)
{
    shots[i].x=shots[shotsactive-1].x;
    shots[i].y=shots[shotsactive-1].y;
    shots[i].z=shots[shotsactive-1].z;
    shotsactive--;
}
    
void proceed_shots()
{
    int i;
    for (i=0; i<shotsactive; i++)
    {
       shots[i].z-=SHOTSPEED*fmult;
       if(shots[i].z<-GRIDSIZE_Z) delete_shot(i);
    }
}

void draw_shots(bool c)
{
    int i;
    for (i=0; i<shotsactive; i++)
    {
    glPushMatrix();
    glScalef(gridscale, gridscale, gridscale);    
    glTranslatef(shots[i].x, shots[i].y, shots[i].z);
    draw_box(c, 1);
    glPopMatrix(); 
    }
}
bool check_pile(char a)
{
    int x, y, z, check_z=0;
    int i;
    for (x=0; x<GRIDSIZE_X*2; x++)
    for (y=0; y<GRIDSIZE_Y*2; y++)
    if(pile[a].data[x][y])
    {    z=pile[a].data[x][y];
        if (check_z)if(check_z!=z)return(0);
        check_z=z;
    }
    return(1);
}        
        
void inc(char a, char x, char y)
{
    pile[a].data[x][y]++;
}

void start()
{

    int z;
      	   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      	          allegro_gl_flip();
      	   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	   glDisable(GL_LIGHTING);
	   
    play_sample (sound[5],64, 50, 1000, 0);

    for (z=0; z<GRIDSIZE_Z; z++)
    {
       glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
 	   glCallList(l_kam_list);
 	   move_camera_l();
       glColor3ubv(col[0][14-z/6]);
       glPushMatrix();
       glTranslatef(0,0,-z*gridscale);
	   glCallList(gridstrip_list);
       glPopMatrix();
       
       glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
	   glCallList(r_kam_list);
 	   move_camera_r();
       glColor3ubv(col[1][14-z/6]);
       glPushMatrix();
       glTranslatef(0,0,-z*gridscale);
	   glCallList(gridstrip_list);
       glPopMatrix();
       
       glFlush();
       allegro_gl_flip();
       wait(18);
    }
        play_sample (sound[2],64, 50, 1000, 0);

        for (z=512;z>20;z-=7)
    {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 	glCallList(l_kam_list);
 	move_camera_l();
    draw_complete_grid(0);
    glScalef(gridscale*0.18, gridscale*0.18, gridscale*0.18);
 	glTranslatef(0,0,-z);
    draw_object(2,0,0);


    glClear(GL_DEPTH_BUFFER_BIT);


    glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
    glCallList(r_kam_list);
    move_camera_r();
    
    draw_complete_grid(1);
    glScalef(gridscale*0.18, gridscale*0.18, gridscale*0.18);
   	glTranslatef(0,0,-z);
    draw_object(2,0,1);
    
       glFlush();
       allegro_gl_flip();
   }
          wait(500);
}

void gameover()
{
    int z;
 //   glEnable(GL_DEPTH_TEST);
     play_sample (sound[3],64, 50, 1000, 0);

    for (z=512;z>20;z-=7)
    {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 	glCallList(l_kam_list);
 	move_camera_l();
    draw_complete_grid(0);
    glScalef(gridscale*0.18, gridscale*0.18, gridscale*0.18);
 	glTranslatef(0,0,-z);
    draw_object(1,0,0);


    glClear(GL_DEPTH_BUFFER_BIT);


    glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
    glCallList(r_kam_list);
    move_camera_r();
    
    draw_complete_grid(1);
    glScalef(gridscale*0.18, gridscale*0.18, gridscale*0.18);
   	glTranslatef(0,0,-z);
    draw_object(1,0,1);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    
	glViewport(0, 0, init.aufl_w, init.aufl_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0 , init.aufl_w/4, 0, init.aufl_h/4);
 	glColor4ub(200, 0, 200,0);
 	
 	glPushMatrix();
    glTranslatef(112,0,0);
    glDisable(GL_BLEND);
	draw_integer(points);
    glEnable(GL_BLEND);
    glPopMatrix();
    
       glFlush();
       allegro_gl_flip();
   }
           wait(1400);
           mouseclick();
           points=0;
           lifes=3;
           gameovr=TRUE;
}

void explode_pile(char a)
{
    int x, y, z;
    for (x=0; x<GRIDSIZE_X*2; x++)
    for (y=0; y<GRIDSIZE_Y*2; y++)
    for (z=0; z<pile[a].data[x][y];z++)
    start_explode(x-3.5, y-2.5, z+pile[a].pos);
}
    
void check_collision()
{
    int x, y, z;
    int i, j;
    char a;
    for (i=0; i<shotsactive; i++)
    {
       x=shots[i].x+4; 
       y=shots[i].y+3;
       if(colmap.active[x][y])
       {
       if(shots[i].z<=colmap.depth[x][y])
       {
          delete_shot(i);
          inc(colmap.pile[x][y], x, y);
          play_sample (sound[6],(64+shots[i].z+16)/3, 128, 1000, 0);
//          start_explode(shots[i].x,shots[i].y,shots[i].z);
          points+=10;
          if(check_pile(colmap.pile[x][y]))
          {
            explode_pile(colmap.pile[x][y]);
            pile[colmap.pile[x][y]].active=0;
            play_sample (sound[7],(64+shots[i].z+16)/2, 128, 800, 0);
            points+=count_pile(colmap.pile[x][y])*100;
          }
       }
       
       }
    }
}
void draw_piles(bool c)
{
    int x, y, z, i;
    float w;
    for(i=0;i<MAXPILES;i++)
    {
    if(pile[i].active)
    {
    for (x=0; x<GRIDSIZE_X*2; x++)
    for (y=0; y<GRIDSIZE_Y*2; y++)
    if(pile[i].data[x][y])
    for(z=0; z<pile[i].data[x][y]; z++)

    {
    glPushMatrix();
    glScalef(gridscale, gridscale, gridscale);    
    glTranslatef(x-4, y-3, z + pile[i].pos);
     if(((y-3)==cursor_y)&&((x-4)==cursor_x) && (chrono%3==0))
  //  if(chrono%3==0)
    {
       draw_light_box(c);
    }
    else draw_box(c, (z%5)*2-4);
    glPopMatrix();
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
   	glPolygonMode(GL_FRONT, GL_LINE);            
    for (x=0; x<GRIDSIZE_X*2; x++)
    for (y=0; y<GRIDSIZE_Y*2; y++)
    if(pile[i].data[x][y])
    for(z=0; z<pile[i].data[x][y]; z++)

    {
    glPushMatrix();
    glScalef(gridscale, gridscale, gridscale);    
    glTranslatef(x-4, y-3, z + pile[i].pos);
    w=-(pile[i].pos+z);
    w=1.7-(w/48);
//    glLineWidth(-10.0/pile[i].pos);
glLineWidth(w);
    draw_light_box(c);
    glPopMatrix();
    }
   	glPolygonMode(GL_FRONT, GL_FILL);            
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 1);

    }
    } 
}

int get_depth(char a)
{
    int x, y, z=0;
    for (x=0; x<GRIDSIZE_X*2; x++)
    for (y=0; y<GRIDSIZE_Y*2; y++)
    if (pile[a].data[x][y]>z)z=pile[a].data[x][y];
    return(z);
}
void proceed_piles()
{
    int i;
    for(i=0;i<MAXPILES; i++)
    if(pile[i].active)
    {
       pile[i].pos++;
       if((pile[i].pos+get_depth(i)) >0)
       {
              lifes--;
              if (lifes>0)
              {
              play_sample(sound[4],128, 128, 900, 0);
              wait(1000);
              pile[i].active=0;
              }
              else
              {
              play_sample(sound[4],128, 128, 900, 0);
              wait(100);
              pile[i].active=0;
              gameover();
              //	allegro_exit();
              //  exit;
              }
        }      
    }
}

#define MOUSE
void get_cursor()
{
    float x,y;
    int xi, yi;
    #ifdef MOUSE
    poll_mouse();
    x=mouse_x;
    y=mouse_y;
    xi=(x/init.aufl_w*GRIDSIZE_X*2-GRIDSIZE_X)*2;
    yi=-(y/init.aufl_h*GRIDSIZE_Y*2-GRIDSIZE_Y)*2;
    #endif
    #ifndef MOUSE
	poll_joystick();
    y=joy[0].stick[0].axis[1].pos;
    x=-joy[0].stick[0].axis[0].pos;
    x=-(x/110)*GRIDSIZE_X ;
    y=(y/110)*GRIDSIZE_Y;
    xi=x;
    yi=y;
	#endif
    if(xi<-GRIDSIZE_X)xi=-GRIDSIZE_X;
    else if(xi>=GRIDSIZE_X)xi=GRIDSIZE_X-1;
    if(yi<-GRIDSIZE_Y)yi=-GRIDSIZE_Y;
    else if(yi>=GRIDSIZE_Y)yi=GRIDSIZE_Y-1;
    cursor_x=xi;
    cursor_y=yi;
}

void proceed_camera()
{
    camera.pos_soll.x=(cursor_x+0.5)*gridscale*0.79;
    camera.pos_soll.y=(cursor_y+0.5)*gridscale*0.82;
    accelerate(camera.pos_soll.x, &camera.pos_ist.x, CAMACC);
    accelerate(camera.pos_soll.y, &camera.pos_ist.y, CAMACC);
}

void draw_cursor(bool c)
{
    int z;
    glPushMatrix();
    glScalef(gridscale, gridscale, gridscale);    
    glTranslatef(cursor_x, cursor_y, 0);

                
    glColor4ub(col[c][13][0], col[c][13][1], col[c][13][2], 55);
    just_draw_box();
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT, GL_LINE);            
    glColor4ub(col[c][15][0], col[c][15][1], col[c][15][2], 255);
    just_draw_box();
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();   
}


void render()
{
    int j, u, i;
    char buf[32];
	glEnable(GL_DEPTH_TEST);
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glCallList(l_kam_list);
	move_camera_l();
//	  glScalef(gridscale, gridscale, gridscale);
    glDisable (GL_LINE_SMOOTH);
    draw_complete_grid(0);
    glEnable (GL_LINE_SMOOTH);
   glColor3ubv(col[0][12]);
 

 
    draw_shots(0);
    draw_piles(0);
    
    for(i=0;i<MAXEXPLODES;i++)if(explosion[i].active)
    {
        glPushMatrix();
        glScalef(gridscale, gridscale, gridscale); 
        glTranslatef(explosion[i].pos.x, explosion[i].pos.y, explosion[i].pos.z); 
        draw_explode(i, 0);
        glPopMatrix();
    }
         draw_cursor(0);    
    
////////////////////////////////////////////////////////////////////////////////
	glClear(GL_DEPTH_BUFFER_BIT );
	glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
	glCallList(r_kam_list);
	move_camera_r();
  //glScalef(gridscale, gridscale, gridscale);

    glDisable (GL_LINE_SMOOTH);
    draw_complete_grid(1);
    glEnable (GL_LINE_SMOOTH);
        glColor3ubv(col[1][12]);

 
    draw_shots(1);
    draw_piles(1);
    
    for(i=0;i<MAXEXPLODES;i++)if(explosion[i].active)
    {
        glPushMatrix();
        glScalef(gridscale, gridscale, gridscale); 
        glTranslatef(explosion[i].pos.x, explosion[i].pos.y, explosion[i].pos.z); 
        draw_explode(i, 1);
        glPopMatrix();
    }
        draw_cursor(1);    

////////////////////////////////////////////////////////////////////////////////
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glViewport(0, 0, init.aufl_w, init.aufl_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0 , init.aufl_w/4, 0, init.aufl_h/4);
 	glColor4ub(200, 0, 200, 256);
 	glDisable(GL_BLEND);
 	glDisable(GL_DEPTH_TEST);
 	glPushMatrix();
    glTranslatef(0,0,0);
    sprintf(buf,"points:%d", points);
//	draw_integer(points);
    draw_string(buf);
 	glPopMatrix();
 	glPushMatrix();
    glTranslatef(132,0,0);
    sprintf(buf,"lifes:%d", lifes);
    draw_string(buf);
   	glPopMatrix();
 	glPushMatrix();
    glTranslatef(72,0,0);
    sprintf(buf,"sec:%d", sec-sec_sub);
    draw_string(buf);
   	glPopMatrix();

   	glEnable(GL_BLEND);
 	glEnable(GL_DEPTH_TEST);
        glFlush();
//        	scare_mouse();

	allegro_gl_flip();
}

bool buttonpressedflag;

void user_input()
{
   if ((chrono-input_delay)>100)
   {
      poll_mouse();
      if (joy[0].button[0].b || joy[0].button[1].b || joy[0].button[2].b || joy[0].button[3].b || (mouse_b & 1))//mouse_b & 1)
      {
         if(!buttonpressedflag)
         {
            start_shot();
            play_sample (sound[0],16, 128, 2000, 0);
            input_delay=chrono;
            buttonpressedflag=1;
         }
      }
      else buttonpressedflag=0;
   }
   
}


void init_gfx()
{
	allegro_gl_clear_settings();
	init.farbtiefe=32;
	allegro_gl_set (AGL_COLOR_DEPTH, init.farbtiefe);
	allegro_gl_set (AGL_Z_DEPTH, init.z_puffer);
	allegro_gl_set (AGL_DOUBLEBUFFER, 1);
	allegro_gl_set (AGL_RENDERMETHOD, 1);
	allegro_gl_set (GFX_OPENGL_FULLSCREEN, TRUE);
	allegro_gl_use_mipmapping(TRUE);
 	if (set_gfx_mode(GFX_OPENGL, 1024, 768, 0, 0) != 0)
	{
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message ("Error setting OpenGL graphics mode:\n%s\nAllegro GL error : %s\n", allegro_error, allegro_gl_error);
		exit(1);
	}
	glPolygonMode(GL_FRONT, GL_FILL);            //Fülle die sichtbaren Polygone aus
    glPolygonMode(GL_BACK, GL_POINT);            //errechne für die unsichtbaren Polys nur die Punkte
	glCullFace(GL_BACK);                         //zeichne nur die Vorderseite der Polygone
	glEnable(GL_CULL_FACE);                      //aktiviere BackFaceCulling
	glFrontFace(GL_CCW);                         //die sichtbaren Polygone sind die linksgedrehten
	glShadeModel(GL_FLAT);                       //heißt: kein Gouraud-Shading
	glAlphaFunc(GL_GREATER, 0.5);                //Pixel mit einem Alpha unter 0.5 werden ignoriert
	glClearColor(0.156, 0.156, 0.156, 1.0);      //Hintergrundfarbe
    glEnableClientState (GL_VERTEX_ARRAY);
       
glEnable (GL_LINE_SMOOTH);
glEnable (GL_POINT_SMOOTH);
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
glLineWidth (1.0);

}

void init_display_lists()
{
    l_kam_list = glGenLists (1);
    glNewList(l_kam_list, GL_COMPILE);
    set_view_l();
    glEndList();

    r_kam_list = glGenLists (1);
    glNewList(r_kam_list, GL_COMPILE);
    set_view_r();
    glEndList();
    
    gridstrip_list = glGenLists (1);
    glNewList(gridstrip_list, GL_COMPILE);
    draw_grid_strip();
    glEndList();
    
}

void proceed_game()
{
    sec=chrono/1000;
    if(chrono>proc_counter)
	{
	   proc_counter=chrono+step_counter;
	   proceed_piles();
	   proceeder++;
       play_sample (sound[1],8, 128, 1000, 0);

    }
    if(chrono>speed_counter)
	{
       speed_counter=chrono+5200;
       game_speed*=0.9995;
       step_counter*=game_speed;
    } 
    if(proceeder>17)
	{
	   proceeder=0;
	   make_pile(3,-64);
    }
    
}
void datablaster()
{
int i;
gameovr=FALSE;
clear_piles();
for (i=0;i++;i<MAXPILES)
{
   pile[i].active=FALSE;
   pile[i].pos=-10000;
   
}
clear_colmap();
pile_counter=0;
game_speed=1;
speed_counter=0;
proceeder=0;
step_counter=980;
frame_count=0;
shotsactive=0;

       start();
 	    sec_sub=chrono/1000; 
	    fcount=chrono;
    while(!key[KEY_ESC] && !gameovr)
    { 
        frame_count++;
	    fmult=(chrono-fcount)/1000;
	    fcount=chrono;
	    proceed_game();
        get_cursor();
	    user_input();
	    proceed_shots();
	    make_colmap();
	    check_collision();
	    proceed_explode();
	    proceed_camera();
	    render();
   	}
}


void db_launcher()
{
   gameovr=TRUE;
   make_pile(3, -30);
   make_pile(3, -56);
   while(gameovr)
   {
      datablaster();

   }
}


int main(void)
{
    int i;
	allegro_init();
	install_allegro_gl();
	install_keyboard();
	install_mouse();
	install_int(the_timer, 1);
	LOCK_FUNCTION(the_timer);
	LOCK_VARIABLE(chrono);
	lese_ini("init.ini", &init);
	    
    init.aa                   =    0.040;
	init.mb                   =    0.32;
	init.db                   =    0.55;
	init.an                   =    0.020;
    init.ppm                  = 2000;
	init.aap                  =  80;
	
    init_gfx();
    init_lighting();
   	install_joystick(JOY_TYPE_AUTODETECT);
    init_sound();
    play_sample (sound[8], 32, 128, 1000, 1);
    init_explode();
	lese_3do(&model[0], "3do/ziffern.3do");
	lese_3do(&model[1], "3do/gameover.3do");
	lese_3do(&model[2], "3do/level1.3do");
	lese_3do(&model[3], "3do/helm.3do");
	lese_3do(&font3d, "3do/font.3do");
    camera.m=identity_matrix_f;
    camera.pos.x=0;
    camera.pos.y=0;
//    camera.pos.z=0.45; //35
    camera.pos.z=0.31; //35
    gridscale=init.mb/(GRIDSIZE_X); ////////////////////////!!!!!!!!!!

    init_display_lists();
    db_launcher  ();
	return 0;
}
END_OF_MAIN();
