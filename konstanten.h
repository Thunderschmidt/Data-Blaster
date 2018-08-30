#define TOTZONE                7
#define EMPF		         0.0001
#define STERNANZ        5000
#define MAXSHOTS        128
#define MAXEXPLODES     128
#define SHOTSPEED          64
#define SHOTDAMAGE        15
#define BOUNDING          6



#ifndef M_PI
#define M_PI   3.1415926535897932384626433832795
#endif
#define RAD_2_DEG(x) ((x) * 180 / M_PI)
#define DEG_2_RAD(x) ((x) / (180 / M_PI))

#define DNUM		     256
#define ROTACC              120
#define TRANSACC          40

#define SN          7
#define CROSSBOX    16
#define CROSSGRID   10
#define CROSSSIZE   0.15

#define WORLDBOX 8

#define DBOX_X	6
#define DBOX_Y	6
#define DBOX_Z	32
#define DBOX_ZZ	0.39
#define DNUM    256

#define XHEADING 0.48
#define YHEADING 0.32

#define MAXTIMEACTIONS 4

#define MAXPILES 16

#define CAMACC 0.8


const GLubyte col[4][16][3] =
{
{{ 0, 40, 40},
{ 11, 40, 40},
{ 21, 40, 40},
{ 32, 40, 40},
{ 43, 40, 39},
{ 53, 40, 39},
{ 64, 39, 39},
{ 75, 38, 39},
{ 85, 37, 38},
{ 96, 36, 37},
{107, 32, 34},
{117, 28, 34},
{128, 24, 33},
{139, 17, 29},
{149, 15, 29},
{160,  5, 20}},

{{ 0,  0,  0},
{  0,  0, 16},
{  0,  0, 32},
{  0,  0, 48},
{  0,  0, 64},
{  0,  0, 80},
{  0,  0, 96},
{  0,  0,112},
{  0,  0,128},
{  0,  0,144},
{  0,  0,160},
{  0,  0,176},
{  0,  0,192},
{  0,  0,208},
{  0,  0,224},
{  0,  0,240}},

{{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20},
{160,  5, 20}},

{{ 0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240},
{  0,  0,240}}
};


#define X .525731112119133606
#define Z .850650808352039932

 float icosahedron_v[12][3] = {
{-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
{0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
{Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
};
 GLubyte icosahedron_i[20][3] = {
{0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
{8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
{7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
{6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

const GLint zielkreuz[][3]={{-10,  0, 1}, {- 2,  0, 1},
                            { 10,  0, 1}, {  2,  0, 1},
                            {  0,-10, 1}, {  0, -2, 1},
                            {  0, 10, 1}, {  0,  2, 1}};
                            

const GLfloat light_ambient_l[] = {0.93, 1.0, 1.0, 1.0};
const GLfloat light_diffuse_l[] = {0.1 , 0.0, 0.0, 1.0};
const GLfloat light_ambient_r[] = {1.0 , 1.0, 0.93, 1.0};
const GLfloat light_diffuse_r[] = {0.0 , 0.0, 0.1, 1.0};

VECTOR null={0,0,0};


