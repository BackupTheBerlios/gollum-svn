#include "gollum_lib.h"

/*
Toutes les fonctions de déplacement du robot sont non bloquantes, 
un appel à une nouvelle fonction annule la fonction précédente
*/

void main (void)
{
	init();

	LedAvantGauche = 0;
	LedAvantDroite = 0;
	LedArriere = 0;
	
	while (1)
	{	
		Avance();

		while ((finMouvement == 0))
		{	
			if (chocAntenneDroite)
			{
				ledAvantDroiteClignoter = 1;
				Stop ();
				ReculeNbrePas (10cm);
				while (finMouvement == 0);
				RotationGauche (20);
				while (finMouvement == 0);
				chocAntenneDroite = 0;
				ledAvantDroiteClignoter = 0;
				LedAvantDroite = 0;
			}
				
			if (chocAntenneGauche)
			{
				ledAvantGaucheClignoter = 1;
				Stop ();
				ReculeNbrePas (10cm);
				while (finMouvement == 0);
				RotationDroite (20);
				while (finMouvement == 0);
				chocAntenneGauche = 0;
				ledAvantGaucheClignoter = 0;
				LedAvantGauche = 0;
			}
			
			if (chocRouleau)
			{
				ledArriereClignoter = 1;
				Stop ();
				ReculeNbrePas (10cm);
				while (finMouvement == 0);
				RotationDroite (40);
				while (finMouvement == 0);
				chocRouleau = 0;
				ledArriereClignoter = 0;
				LedArriere = 0;
			}
			
			mesure = 127 + MesureLumiere(ldrGauche) - (MesureLumiere(ldrDroit)+20);
			if (mesure >= 150)
			{
				LedAvantGauche = 1;
				Stop();
				RotationGauche(10);
				while (finMouvement == 0);
				LedAvantGauche = 0;
			}
			if (mesure <= 104)
			{
				LedAvantDroite = 1;
				Stop();
				RotationDroite(10);
				while (finMouvement == 0);
				LedAvantDroite = 0;
			}
		}
	}
}
