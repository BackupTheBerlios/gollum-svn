# include <p18f2420.h>

#define cm *2
#define ldrGauche 0
#define ldrDroit 1

#define LedAvantDroite PORTCbits.RC4 
#define LedArriere PORTCbits.RC5
#define LedAvantGauche PORTBbits.RB3

extern unsigned char finMouvement;
extern unsigned char mvtContinu ;
extern unsigned char chocAntenneGauche ;
extern unsigned char chocAntenneDroite ;
extern unsigned char chocRouleau ;
extern unsigned char mesure ;
extern unsigned char ledAvantDroiteClignoter ;
extern unsigned char ledAvantGaucheClignoter ;
extern unsigned char ledArriereClignoter ;

unsigned char MesureLumiere (unsigned char canal);
void Avance (void);
void AvanceNbrePas (unsigned int nbrePas);
void Stop (void);
void RotationDroite (unsigned char nbrePas);
void RotationGauche (unsigned char nbrePas);
void ReculeNbrePas (unsigned char nbrePas);
void Recule ();

void init(void);
