/*
    Exposant : 4 bits
    Mantisses : 4x3bits


    L'encodage effectif

    L'exposant 4 bit correspond à un niveau maximal (0 à -64dB)
    La mantisse 6+1 bit correspond à un multiplicateur n/63 (pour n=0...63)

    On obtient une dynamique maxiale de 100dB
    Le rapport signal/bruit est amélioré (tout comme la distorsion) grâce
    à un générateur de bruit blanc et un treble boost.

    Dynamique totale : 100 dB
    Rapport S/B mini : 54 dB
    Quant. Equival.  : 9...17bits
    
    
    Avantage du XD4e sur le XD4 :
    - L'exposant sur 4 bits utilise des paliers de 4,26dB pour une plage de 0 à -64dB
    - La 



    XD4e devient XD4

    Un bloc de 32 bits
    - Contenant un exposant et 4 mantisses

    
    Les possibilités

    Exposant    Mantisses
    4 bits      4 x 7 bits
    8 bits      4 x 6 bits
    12 bits     4 x 5 bits


    Le choix 4bits + 4x7bits semble interressant :

    L'exposant est une valeur en dB, il indique une valeur absolue
    Avec une échelle de 16 valeurs, on assigne 3 dB pour chaque valeur.
    Cela nous donne une plage de 45dB !

    Exposant = pow(10,-ExposantValue*0.15)

     0 =>   0dB FS
     1 =>  -3dB FS
     2 =>  -6dB FS
    14 => -42dB FS
    15 => -45dB FS


    La mantisse est en fait sur 6+1 bits
    La valeur est sur 6 bits (0...63)
    Le signe est sur 1 bit (0/1 pour -1 ou +1)

    Pour obtenir la valeur réelle, il faut utiliser un calcul assez complexe :

    ValeurReelle = Exposant * (1+MantisseValeur)/64 * MantisseSigne

    Ainsi, pour un bloc de 4 points, on dispose par point d'une profondeur de -36dB par rapport au total
    

    Théorie :
    On doit s'attendre à une capacité d'atténuation du bruit assez importante puisque l'on travaille
    sur des blocs de 4 échantillons.
    Certe le rapport S/B d'un bloc n'est pas exceptionnel puisque l'on frise avec 36dB, mais sur un nombre de
    blocs plus important, le rapport S/B doit atteindre dans le meilleur des cas, un peu plus de 80dB.
    Ainsi, on reste assez proche d'un encodage 16 bits.

    En modifiant légèrement la courbe de valeur mantisse, on doit arriver à passer de 36 à 45 dB.
    Cela porterait la rapport S/B maximal à 90 dB


    Pour un décodage accéléré, il est nécessaire de créer des LUT.
    On doit utiliser 16 LUT de 128 valeurs soit un total de 2048 valeurs soit 8Ko.
    Par rapport au XD4, on évite un calcul sur les flottants car la bonne valeur est directement extraite de la LUT.










*/

#include "sam_header.h"



void        SAM_XD4_Init        ( void );
DWORD       SAM_XD4_EncodeValue ( FLOAT32 f32Value1, FLOAT32 f32Value2, FLOAT32 f32Value3, FLOAT32 f32Value4 );
FLOAT32     SAM_XD4_DecodeValue ( DWORD dwXD4Data, DWORD dwValueIndex );

FLOAT32     XD4_fNoiseMul;
FLOAT32     XD4_fNoise;
DWORD       XD4_dwNoise;
FLOAT32     XD4_fNoiseLast;
FLOAT32     XD4_fExposant[16];
FLOAT32     XD4_fExposantDiv63Sign[32];
FLOAT32     XD4_fMantisse[128];

FLOAT32     XD4_fExposantMantisse[16*128];

/*
    Décodeur initial en 2 passes : Exposant + Mantisse
    Utilisation de 3 LUT pour un total de 592 octets
*/
DWORD       XD4_dwShiftRight[4] = { 4, 11, 18, 25 };
FLOAT32     SAM_XD4_DecodeValueNormal ( DWORD dwXD4Data, DWORD dwValueIndex )
{
    DWORD dwExponent;
    DWORD dwMantisse;
    
    //Lecture de l'exposant             
    dwExponent = dwXD4Data&0x0F;

    //Lecture de la mantisse
    dwMantisse = (dwXD4Data>>XD4_dwShiftRight[dwValueIndex])&0x7F;

    return XD4_fExposant[dwExponent] * XD4_fMantisse[dwMantisse];
}

/*
    Décodeur ultra-rapide en une seule passe
*/
DWORD       XD4_dwShiftRightFast[4] = { 0, 7, 14, 21 };
FLOAT32     SAM_XD4_DecodeValue ( DWORD dwXD4Data, DWORD dwValueIndex )
{
    DWORD dwIndex;
    
    //Lecture de l'exposant et de la mantisse
    dwIndex = (dwXD4Data&0x0F) | ((dwXD4Data>>XD4_dwShiftRightFast[dwValueIndex])&0x7F0);

    return XD4_fExposantMantisse[dwIndex];
}


DWORD       SAM_XD4_EncodeValue ( FLOAT32 f32Value1, FLOAT32 f32Value2, FLOAT32 f32Value3, FLOAT32 f32Value4 )
{
//    FLOAT32 fTempA, fTempB;
    FLOAT32 fAbs[4], fSgn[4];
    FLOAT32 fAbsMaxLevel;
    FLOAT32 fExposant;
    FLOAT32 fMulExposant;
    FLOAT32 fNoiseLevel;
    DWORD dwIndex;
    DWORD dwExponent;
    DWORD dwMantisse;
    DWORD dwEncodedValue;


    fAbs[0] = sam_ABS(f32Value1);
    fAbs[1] = sam_ABS(f32Value2);
    fAbs[2] = sam_ABS(f32Value3);
    fAbs[3] = sam_ABS(f32Value4);
    fSgn[0] = f32Value1;
    fSgn[1] = f32Value2;
    fSgn[2] = f32Value3;
    fSgn[3] = f32Value4;


    //Le niveau absolu
    fAbsMaxLevel = 0.000001F;
    for (dwIndex=0;dwIndex<4;dwIndex++)
    {
        if (fAbs[dwIndex]>fAbsMaxLevel) fAbsMaxLevel = fAbs[dwIndex];
    }
    fAbsMaxLevel = (float)log10 ( fAbsMaxLevel ) * 20;



    //Calcul de l'exposant
    fAbsMaxLevel = -(float)ceil(fAbsMaxLevel*0.234375);//33333333333);
    dwExponent = (DWORD)fAbsMaxLevel;
    if (dwExponent>15) dwExponent = 15;
    if (dwExponent< 0) dwExponent =  0;
    dwEncodedValue = dwExponent;

    //Récupération de l'exposant décodé
    fExposant = XD4_fExposant[dwExponent];
    fNoiseLevel = (fExposant*0.2F+0.0006F*0.8F)*0.000005F;//0.000010;
    fMulExposant = 63 / fExposant;

    //Calcul de chacune des mantisses
    for (dwIndex=0;dwIndex<4;dwIndex++)
    {
        //Calcul du bruit
        XD4_dwNoise         = ((XD4_dwNoise * 1664525)+1013904223);
        XD4_fNoiseLast      = ((float)(XD4_dwNoise&0xFFFF)) * fNoiseLevel; //0.0000015;//0.0000035;
        XD4_fNoise          += (XD4_fNoiseLast-XD4_fNoise)*0.9F;
        fSgn[dwIndex]       += XD4_fNoiseLast-XD4_fNoise;
        fAbs[dwIndex]       = sam_ABS(fSgn[dwIndex]);

        //fTempA = fAbs[dwIndex] / fExposant;
        //fTempA = fTempA * 63;
        //dwMantisse = floor(fTempA);

        dwMantisse = (DWORD)floor( fAbs[dwIndex] * fMulExposant );
        if (dwMantisse< 0) dwMantisse =  0;
        if (dwMantisse>63) dwMantisse = 63;

        //Le signe
        if (fSgn[dwIndex]<0) dwMantisse += 64;

        dwEncodedValue |= dwMantisse<<XD4_dwShiftRight[dwIndex];
        
    }

    return dwEncodedValue;
}

void        SAM_XD4_Init        ( void )
{
    DWORD dwIndex;
    DWORD dwMantisse, dwExposant;
    FLOAT32 f32TempA, f32TempB;

    XD4_fNoiseMul = 1.0F / (float)pow ( 2, 27 ); //30

    //Génération de l'exposant
    for (dwIndex=0;dwIndex<16;dwIndex++)
    {
        f32TempA                = (float)dwIndex;
        f32TempA                = (float)pow ( 10.0, -f32TempA*4.2666667*0.05F );
        XD4_fExposant[dwIndex]  = f32TempA;
        XD4_fExposantDiv63Sign[dwIndex]     = f32TempA / 63.0F;
        XD4_fExposantDiv63Sign[dwIndex+16]  = -f32TempA / 63.0F;
    }

    //Génération de la mantisse
    for (dwIndex=0;dwIndex<64;dwIndex++)
    {
        f32TempA                    = (float)dwIndex;
        f32TempA                    = (f32TempA)/63;
        XD4_fMantisse[dwIndex]      = f32TempA;
        XD4_fMantisse[dwIndex+64]   = -f32TempA;
    }

    //Génération du bloc mantisse+exposant
    for (dwMantisse=0;dwMantisse<=63;dwMantisse++)
    {
        f32TempA = (float)dwMantisse;
        f32TempA = (f32TempA)/63;
        for (dwExposant=0;dwExposant<16;dwExposant++)
        {
            f32TempB                = (float)dwExposant;
            f32TempB                = (float)pow ( 10.0, -f32TempB*4.2666667*0.05F );
            f32TempB                *= f32TempA;
            XD4_fExposantMantisse[dwExposant+((dwMantisse   )<<4)] =  f32TempB;
            XD4_fExposantMantisse[dwExposant+((dwMantisse+64)<<4)] = -f32TempB;
        }
    }
}




