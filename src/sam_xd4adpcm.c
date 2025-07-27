/*

    On encode dans un bloc de 16 octets
    
    
    On doit stocker 20 échantillons par bloc
    
    
    
    
    
    
    L'encodage effectif

    L'exposant 4 bit correspond à un niveau maximal (0 à -64dB)
    La mantisse 6+1 bit correspond à un multiplicateur n/63 (pour n=0...63)

    On obtient une dynamique maxiale de 100dB
    Le rapport signal/bruit est amélioré (tout comme la distorsion) grâce
    à un générateur de bruit blanc et un treble boost.

    Dynamique totale : 100 dB
    Rapport S/B mini : 54 dB
    Quant. Equival.  : 9...17bits



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



#define XD4ADPCM_EXPONENT_STEP      (4.26666667F)       //Pas en dB de l'exposant
#define XD4ADPCM_OFFSET_SET         (6.0F)              //Pas en dB de l'offset
#define XD4ADPCM_FLOOR_LEVEL        (0.000001F)         //-120dB

float XD4ADPCM_fOffset[16];
float XD4ADPCM_fExponent[16];
float XD4ADPCM_fMantissa[64];
DWORD XD4ADPCM_dwShiftRight[4] = { 8, 14, 20, 26 };

DWORD       SAM_XD4ADPCM_EncodeValue ( FLOAT32 f32Value1, FLOAT32 f32Value2, FLOAT32 f32Value3, FLOAT32 f32Value4 )
{
    FLOAT32 fTempA;
    FLOAT32 fValue[4];
    FLOAT32 fAbs[4], fSgn[4];
    FLOAT32 fAbsMaxLevel;
//    FLOAT32 fExposant;
//    FLOAT32 fNoiseLevel;
    FLOAT32 fCurrentValue;
    FLOAT32 fNeededValue;
    FLOAT32 fLowestError;
    FLOAT32 fOffsetLevel;
    DWORD dwIndex;
    DWORD dwExponent;
    DWORD dwMantisse;
    DWORD dwEncodedValue;
    DWORD dwBestMantisse;
    DWORD dwOffset;

    //Copie des valeurs
    fValue[0] = f32Value1;
    fValue[1] = f32Value2;
    fValue[2] = f32Value3;
    fValue[3] = f32Value4;

    //Mesure l'offset
    fOffsetLevel = 0;
    for (dwIndex=0;dwIndex<4;dwIndex++)
    {
        fOffsetLevel += fValue[dwIndex];
    }
    fOffsetLevel = fOffsetLevel * 0.25F;
    fTempA = sam_ABS ( fOffsetLevel ) + XD4ADPCM_FLOOR_LEVEL;
    fTempA = -(float)floor ( log10 ( fTempA ) * 20 / XD4ADPCM_OFFSET_SET );
    dwOffset = (DWORD)fTempA;
    if (dwOffset>7) dwOffset=7;
    if (dwOffset<0) dwOffset=0;
    if (fOffsetLevel<0) dwOffset+=8;

    //Application de l'offset
    fOffsetLevel = XD4ADPCM_fOffset[dwOffset];
    for (dwIndex=0;dwIndex<4;dwIndex++)
    {
        fValue[dwIndex] -= fOffsetLevel;
    }

    //Calcule les valeurs absolues et les signes
    for (dwIndex=0;dwIndex<4;dwIndex++)
    {
        fAbs[dwIndex] = sam_ABS(fValue[dwIndex]);
        if (fValue[dwIndex]>=0) fSgn[dwIndex] =  1.0F;
        else                    fSgn[dwIndex] = -1.0F;
    }

    //Le niveau absolu
    fAbsMaxLevel = XD4ADPCM_FLOOR_LEVEL;
    for (dwIndex=0;dwIndex<4;dwIndex++)
    {
        if (fAbs[dwIndex]>fAbsMaxLevel) fAbsMaxLevel = fAbs[dwIndex];
    }
    fAbsMaxLevel = (float)log10 ( fAbsMaxLevel ) * 20;

    //Calcul de l'exposant
    fAbsMaxLevel = -(float)ceil(fAbsMaxLevel/XD4ADPCM_EXPONENT_STEP);
    dwExponent = (DWORD)fAbsMaxLevel;
    if (dwExponent>15) dwExponent = 15;
    if (dwExponent< 0) dwExponent =  0;

    //Les premiers 8 bits à encoder...
    dwEncodedValue = dwExponent | (dwOffset<<4);


    /*
    //Calcul de l'exposant initial
    fExposant = log10 ( fAbs[0] + 0.0000001F ) * 20;
    fExposant = -ceil(fExposant/XD4ADPCM_EXPONENT_STEP);
    dwExponent = (DWORD)fExposant;
    if (dwExponent>15) dwExponent = 15;
    if (dwExponent< 0) dwExponent =  0;

    //Encodage de l'exposant
    dwEncodedValue = dwExponent;
    */

    //Lecture de la valeur initiale
    fCurrentValue = 0; //XD4ADPCM_fExponent[dwExponent];

    //Traîtement de toutes les mantisses
    for (dwIndex=0;dwIndex<4;dwIndex++)
    {
        dwBestMantisse = 0;
        fNeededValue   = fValue[dwIndex];
        fLowestError   = 1000000000.0F;
        for (dwMantisse=0;dwMantisse<64;dwMantisse++)
        {
            fTempA = (fCurrentValue + XD4ADPCM_fMantissa[dwMantisse])*XD4ADPCM_fExponent[dwExponent];
            if (sam_ABS(fTempA-fNeededValue)<fLowestError)
            {
                fLowestError = sam_ABS(fTempA-fNeededValue);
                dwBestMantisse = dwMantisse;
            }
        }

        //Application de la mantisse
        fCurrentValue += XD4ADPCM_fMantissa[dwBestMantisse];

        //Encodage de la mantisse
        dwEncodedValue |= dwBestMantisse<<XD4ADPCM_dwShiftRight[dwIndex];
    }

    return dwEncodedValue;
}


void        SAM_XD4ADPCM_Init        ( void )
{
    DWORD dwIndex;
//    DWORD dwMantisse, dwExposant;
    FLOAT32 f32TempA;

    //XD4_fNoiseMul = 1 / pow ( 2, 27 ); //30

    //Génération de l'exposant
    for (dwIndex=0;dwIndex<16;dwIndex++)
    {
        f32TempA                    = (float)dwIndex;
        f32TempA                    = (float)pow ( 10.0, -f32TempA*XD4ADPCM_EXPONENT_STEP*0.05F );
        XD4ADPCM_fExponent[dwIndex] = f32TempA;
    }

    //Génération de l'offset
    for (dwIndex=0;dwIndex<7;dwIndex++)
    {
        f32TempA                        = (float)dwIndex;
        f32TempA                        = (float)pow ( 10.0, -f32TempA*XD4ADPCM_OFFSET_SET*0.05F );
        XD4ADPCM_fOffset[dwIndex]       = f32TempA;
        XD4ADPCM_fOffset[dwIndex+8]     = -f32TempA;
    }
    XD4ADPCM_fOffset[dwIndex]       = 0;
    XD4ADPCM_fOffset[dwIndex+8]     = -0;


    

    //Génération de la mantisse
    for (dwIndex=0;dwIndex<32;dwIndex++)
    {
        f32TempA                        = (float)dwIndex;
        //f32TempA                        = (f32TempA-43);//pow ( f32TempA, 2.0 );
        //f32TempA                        = (float)pow ( 10, f32TempA * 0.05 );
        f32TempA                        = f32TempA/31;
        XD4ADPCM_fMantissa[dwIndex]     = f32TempA;
        XD4ADPCM_fMantissa[dwIndex+32]  = -f32TempA;
    }
}


FLOAT32     SAM_XD4ADPCM_DecodeValue ( DWORD dwXD4Data, DWORD dwValueIndex, void * pCodecValue )
{
    float *pfInternalData;

    pfInternalData = (float *)pCodecValue;

    //Lecture de l'exposant initial
    if (dwValueIndex==0)
    {
        pfInternalData[0] = XD4ADPCM_fExponent[(dwXD4Data&0x0F)];
        pfInternalData[1] = XD4ADPCM_fOffset[(dwXD4Data&0xF0)>>4];
        pfInternalData[2] = 0;

    }

    //Application de la nouvelle mantisse
    pfInternalData[2] += XD4ADPCM_fMantissa[(dwXD4Data>>XD4ADPCM_dwShiftRight[dwValueIndex])&0x3F];
    // *= XD4ADPCM_fMantissa[(dwXD4Data>>XD4ADPCM_dwShiftRight[dwValueIndex])&0x7F];

    return (pfInternalData[0]*pfInternalData[2])+pfInternalData[1];
}

