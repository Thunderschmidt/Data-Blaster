#ifndef INPUTINI
#define INPUTINI

typedef struct INIT
{
    float 	     aa;        //Augenabstand
	float        mb;        //Monitorbreite
	float        db;        //Distanz zum Bildschirm
	float        an;        //Augen-Nasenwurzelabstand
	float        zoom;		//Kamera-Winkel
    float	     ppm;		//Pixel pro Meter
	int          aap;		//Augenabstand in Pixeln
	float        aspect;	//Verh�ltnis X:Y der Kamera-Pyramide
	float        near_clip; //
	float        far_clip;  //Bis zu welcher Entfernung werden Objekte dargestellt?
    short        aufl_w;    //Bildschirmaufl�sung in der Breite
    short        aufl_h;    //Bildschirmaufl�sung in der H�he
    short        farbtiefe; //Farbqualit�t (8, 16, 24 oder 32)
    short        z_puffer;  //Aufl�sung des Tiefenpuffers (8, 16, 24 oder 32)
}INIT;


void lese_ini(const char datei[], INIT *init);

#endif
