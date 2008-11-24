# include "gollum_lib.h"

#define motG1 PORTCbits.RC0
#define motG2 PORTCbits.RC1
#define motG3 PORTCbits.RC2
#define motG4 PORTCbits.RC3

#define motD1 PORTBbits.RB4
#define motD2 PORTBbits.RB5
#define motD3 PORTBbits.RB6
#define motD4 PORTBbits.RB7

#define AVANCE_GAUCHE 0
#define RECULE_GAUCHE 1
#define AVANCE_DROITE 1
#define RECULE_DROITE 0

unsigned char finMouvement = 1;
unsigned char mvtContinu = 0;
unsigned char chocAntenneGauche = 0;
unsigned char chocAntenneDroite = 0;
unsigned char chocRouleau = 0;
unsigned char tempsEcoule = 0;
unsigned char ledAvantDroiteClignoter = 0 ;
unsigned char ledAvantGaucheClignoter = 0 ;
unsigned char ledArriereClignoter = 0 ;

unsigned int nbrePasTotal = 0;
unsigned int nbrePasCourant = 0;
unsigned char sensMoteurGauche = 0;
unsigned char sensMoteurDroit = 0;
unsigned char etatGauche = 0;
unsigned char etatDroit = 0;
unsigned char mesure = 0;
unsigned int i = 0;
unsigned int clignotementOn = 0;
unsigned int prescalTmr1 = 0;

void moteur_isr(void);
void clignoter_isr(void);
void microswitch_isr(void);

unsigned char MesureLumiere (unsigned char canal)
{
	ADCON0 = ( (ADCON0 & 0b11000011) | (canal*4) ); // oui, c'est clair !
	ADCON0bits.GO_DONE = 1;
	while (ADCON0bits.GO_DONE == 1);
	return (ADRESH);
}

void Avance (void)
{
	finMouvement = 0;
	mvtContinu = 1;
	sensMoteurGauche = AVANCE_GAUCHE;
	sensMoteurDroit  = AVANCE_DROITE;
	T0CONbits.TMR0ON = 1;
}

void AvanceNbrePas (unsigned int nbrePas)
{
	finMouvement = 0;
	mvtContinu = 0;
	nbrePasCourant = 0; // mis à 0 au départ au cas où on aurait décidé de passer d'un mvt continu à mvt fini
	nbrePasTotal = nbrePas;
	sensMoteurGauche = AVANCE_GAUCHE;
	sensMoteurDroit  = AVANCE_DROITE;
	
	TMR0H = 255;
	
	T0CONbits.TMR0ON = 1;
}

void Stop (void)
{
	T0CONbits.TMR0ON = 0;
	TMR0H=0;
	TMR0L=0;
	nbrePasCourant=0;
	finMouvement=1;
	motG1 = 1; //limite la consommation
	motG2 = 1;
	motG3 = 1;
	motG4 = 1;
	motD1 = 1;
	motD2 = 1;
	motD3 = 1;
	motD4 = 1;
}

void RotationDroite (unsigned char nbrePas) // 48 pas/tour => 7.5° / pas
{
	finMouvement = 0;
	mvtContinu = 0;
	nbrePasCourant = 0; // mis à 0 au départ au cas où on aurait décidé de passer d'un mvt continu à mvt fini
	nbrePasTotal = nbrePas;
	sensMoteurGauche = AVANCE_GAUCHE;
	sensMoteurDroit  = RECULE_DROITE;
	
	TMR0H = 255;
	
	T0CONbits.TMR0ON = 1;
}

void RotationGauche (unsigned char nbrePas)
{
	finMouvement = 0;
	mvtContinu = 0;
	nbrePasCourant = 0; // mis à 0 au départ au cas où on aurait décidé de passer d'un mvt continu à mvt fini
	nbrePasTotal = nbrePas;
	sensMoteurGauche = RECULE_GAUCHE;
	sensMoteurDroit  = AVANCE_DROITE;
	
	TMR0H = 255;
	
	T0CONbits.TMR0ON = 1;
}

void ReculeNbrePas (unsigned char nbrePas)
{
	finMouvement = 0;
	mvtContinu = 0;
	nbrePasCourant = 0; // mis à 0 au départ au cas où on aurait décidé de passer d'un mvt continu à mvt fini
	nbrePasTotal = nbrePas;
	sensMoteurGauche = RECULE_GAUCHE;
	sensMoteurDroit  = RECULE_DROITE;
	
	TMR0H = 255;
	
	T0CONbits.TMR0ON = 1;
}

void Recule ()
{
	finMouvement = 0;
	mvtContinu = 1;
	sensMoteurGauche = RECULE_GAUCHE;
	sensMoteurDroit  = RECULE_DROITE;
	T0CONbits.TMR0ON = 1;
}


#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
	if (INTCONbits.TMR0IF ==1)
		_asm GOTO moteur_isr _endasm
	else if	(PIR1bits.TMR1IF == 1)
		_asm GOTO clignoter_isr _endasm
	else
		_asm GOTO microswitch_isr _endasm
}
#pragma code //??

#pragma interrupt microswitch_isr
void microswitch_isr (void)
{
	INTCONbits.GIE = 0;
	
	if (INTCONbits.INT0IF == 1)
	{
		INTCONbits.INT0IF = 0;
		chocAntenneGauche = 1;	// Cette variable est globale et peut être lue depuis le main MAIS doit être remise à 0 en soft : c'est juste une duplication du flag
	}
		
	if (INTCON3bits.INT1IF == 1)
	{
		INTCON3bits.INT1IF = 0;	
		chocAntenneDroite = 1;	
	}
	
	if (INTCON3bits.INT2IF == 1)
	{
		INTCON3bits.INT2IF = 0;
		chocRouleau = 1;
	}
	
	INTCONbits.GIE = 1;
}

#pragma interrupt clignoter_isr
void clignoter_isr (void)
{
	INTCONbits.GIE = 0;
	
	// prescale software (on ne peut descendre que jusqu'à 1/8)
	if (prescalTmr1 % 4)
	{
		if (ledAvantDroiteClignoter == 1)
		{
			LedAvantDroite = clignotementOn;
		}
		if (ledAvantGaucheClignoter == 1)
		{
			LedAvantGauche = clignotementOn;
		}
		if (ledArriereClignoter == 1)
		{
			LedArriere = clignotementOn;
		}
		clignotementOn = !clignotementOn;
	}
	prescalTmr1++;
	
	PIR1bits.TMR1IF = 0;
	
	INTCONbits.GIE = 1;
}

#pragma interrupt moteur_isr
void moteur_isr (void)
{
	INTCONbits.GIE = 0;
	INTCONbits.TMR0IF = 0;

	if ((nbrePasCourant == nbrePasTotal) && (mvtContinu ==0))
	{
		T0CONbits.TMR0ON = 0;
		TMR0H=0;
		TMR0L=0;
		nbrePasCourant=0;
		motG1 = 1;
		motG2 = 1;
		motG3 = 1;
		motG4 = 1;
		motD1 = 1;
		motD2 = 1;
		motD3 = 1;
		motD4 = 1;
   		finMouvement=1;
	}
	else
	{

		if (sensMoteurGauche == 1)	// le moteur gauche doit tourner dans le sens de la marche du robot
			++ etatGauche;
		else
			-- etatGauche;
		
		//mode bipolaire a pas entier couple maximal

		switch (etatGauche & 3) //etat modulo 4
		{
			case 0 :
			{
				motG1 = 0;				
				motG2 = 1;
				motG3 = 1;				
				motG4 = 0;
				break;
			}

			case 1 :
			{
				motG1 = 0;				
				motG2 = 1;
				motG3 = 0;
				motG4 = 1;
				break;
			}
		
			case 2 :
			{
				motG1 = 1;
				motG2 = 0;
				motG3 = 0;				
				motG4 = 1;
				break;
			}

			case 3 :
			{
				motG1 = 1;				
				motG2 = 0;
				motG3 = 1;
				motG4 = 0;
				break;
			}
		
		}

		if (sensMoteurDroit == 1)	// le moteur droit doit tourner dans le sens de la marche du robot
			++ etatDroit;
		else
			-- etatDroit;

		switch (etatDroit & 3)
		{
			case 0 :
			{
				motD1 = 0;				
				motD2 = 1;
				motD3 = 1;				
				motD4 = 0;
				break;
			}

			case 1 :
			{
				motD1 = 0;				
				motD2 = 1;
				motD3 = 0;
				motD4 = 1;
				break;
			}
		
			case 2 :
			{
				motD1 = 1;
				motD2 = 0;
				motD3 = 0;				
				motD4 = 1;
				break;
			}

			case 3 :
			{
				motD1 = 1;				
				motD2 = 0;
				motD3 = 1;
				motD4 = 0;
				break;
			}
		}
	
	++ nbrePasCourant;
	}	

	INTCONbits.GIE = 1; //modification vvdaele bug ??
}


void init(void)
{
	// Configuration des entrées et sorties
	TRISC=0b10000000;
	TRISB=0b00000111;
	TRISA=0b00000111;
	
	motG1 = 0;
	motG2 = 1;
	motG3 = 1;
	motG4 = 0;
	
	motD1 = 0;
	motD2 = 1;
	motD3 = 1;
	motD4 = 0;
	
	// config interrupt
	RCONbits.IPEN = 0;			// mode classique d'interruption (pas de priorité haute ou basse)
	INTCONbits.PEIE = 1;	
	
	// Config INT0
	INTCON2bits.INTEDG0 = 0; 	// interruption sur flanc descendant
	INTCONbits.INT0IE = 1;
	// Flag : INTCONbits.INT0IF
	
	// Config INT1
	INTCON2bits.INTEDG1 = 0;
	INTCON3bits.INT1IE = 1;
	// Flag : INTCON3bits.INT1IF
	
	// Config INT2
	//INTCON2bits.INTEDG2 = 0; //existe pas!!
	INTCON3bits.INT2IE = 1;
	// Flag : INTCON3bits.INT2IF
	
	// config timer 0
	T0CONbits.TMR0ON = 0;
	T0CONbits.T08BIT = 0;		// timer0 configuré en 16 bits
	T0CONbits.T0CS = 0;			// timer0 incrémenté sur horloge interne
	T0CONbits.PSA = 0;			// 0 : préscalaire actif, 1 : préscalaire inactif
	T0CONbits.T0PS2 = 0; 		// préscalaire : 000 = 1/2	111 = 1/256 => 1/16 ici
	T0CONbits.T0PS1 = 1;
	T0CONbits.T0PS0 = 1;
	INTCONbits.TMR0IE = 1;		// l'interruption externe T0 est autorisée
	// Flag : INTCONbits.TMR0IF
	
	// config timer 1 -- pour le clignotement des leds
	T1CONbits.TMR1ON = 1;
	T1CONbits.RD16 = 0;
	T1CONbits.TMR1CS = 0;		// incrémenté sur l'horloge interne, si 1 alors les deux bits suivants
								// sont utilisés (je crois)
	//T1CONbits.T1RUN = 1;		// incrémenté sur l'oscillateur et non sur une autre source
	//T1CONbits.T1SYNC = 1;		// pas horloge externe pas synchronisée
	T1CONbits.T1CKPS1 = 1;		// préscalaire : 00 = 1/1  11 = 1/8 => 1/8 ici
	T1CONbits.T1CKPS0 = 1;
	T1CONbits.T1OSCEN = 0;
	PIE1bits.TMR1IE = 1;
	
	//sert a quoi ???????
	/*// config CCP1
	CCP1CONbits.CCP1M3 = 1; 	
	CCP1CONbits.CCP1M2 = 0;
	CCP1CONbits.CCP1M1 = 1;
	CCP1CONbits.CCP1M0 = 0;
	
	//sert a quoi ???????
	// config timer 1
	T1CONbits.T1CKPS1 = 1;		// prescale du timer 1 réglé à 8
	T1CONbits.T1CKPS0 = 1;
	T1CONbits.TMR1CS = 0;
	//T1CONbits.TMR1ON = 1;
	*/
	
	// config A/D
	ADCON1bits.PCFG3 = 1;		// AN0, 1, 2, 3, 4, 5 en mode analogique
	ADCON1bits.PCFG2 = 0;
	ADCON1bits.PCFG1 = 0;
	ADCON1bits.PCFG0 = 1;
	ADCON1bits.VCFG0 = 0;      // référence de tension 0 -> 5V
	ADCON1bits.VCFG1 = 0;
	ADCON2bits.ADCS2 = 1; 		// fréquence de conversion = Fosc/64 (requis pour le 40 MHz)
	ADCON2bits.ADCS1 = 1;
	ADCON2bits.ADCS0 = 0;
	ADCON0bits.ADON = 1;		// CAN prêt
	ADCON2bits.ADFM = 0;		// bits justifiés à gauche, lire les 8 bits les plus significatifs dans ADRESH
	//PIE1bits.ADIE = 1;
	// Flag PIR1bits.ADIF
	// ADCON0bits.ADGO : bit d'activation du convertisseur
	
	// fin config interrupt
	//PIE1bits.TMR1IE = 1;
	INTCONbits.GIE = 1;			// masque global d'interruption désactivé

}
