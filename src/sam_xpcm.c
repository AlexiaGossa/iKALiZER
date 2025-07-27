/*

    XD4e

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
    On doit s'attendre à un capacité d'atténuation du bruit assez importante puisque l'on travaille
    sur des blocs de 4 échantillons.
    Certe le rapport S/B d'un bloc n'est pas exceptionnel puisque l'on frise avec 36dB, mais sur un nombre de
    blocs plus important, le rapport S/B doit atteindre dans le meilleur des cas, un peu plus de 80dB.
    Ainsi, on reste assez proche d'un encodage 16 bits.

    En modifiant légèrement la courbe de valeur mantisse, on doit arriver à passer de 36 à 45.
    Cela porterait la rapport S/B maximal à 90 dB


    Pour un décodage accéléré, il est nécessaire de créer des LUT.
    On doit utiliser 16 LUT de 128 valeurs soit un total de 2048 valeurs soit 8Ko.
    Par rapport au XD4, on évite un calcul sur les flottants car la bonne valeur est directement extraite de la LUT.











    XD4

    Amélioration
    - Pour accroitre la qualité de l'encodage, il est possible d'ajouter un bruit blanc à -60dB

    Avantages du traîtement XD4
    - 36 dB de rapport S/B minimal
    - 91 dB de rapport S/B maximal
    - Vitesse de décodage à peine plus longue que le XPCM8

    Inconvénient
    - Blocs de 4 échantillons... plus complexe à gérer pour le stéréo
    - LUT de 2048 et 128 entrées soit un total de 8704 octets



    4 échantillons sur 4 octets / 32 bits

    3 échantillons sur 7 bits
    1 échantillon sur 11 bits


    Echantillon BASE - 11 bits

        Codage léger XPCM
        Dynamique       85 dB
        Rapport S/B     60 dB
        Niveau maxi     0 dB FS

    Echantillon Delta - 7 bits

        Codage permettant un signal minimal de -85 dB

        bDelta = 0...127
        if (bDelta>=64) fDelta = (bDelta-63)
        else            fDelta = (bDelta-64)

        fSigne = fDelta;
        fDelta = fabs(fDelta);

        fDelta = ( fDelta - 64 ) * 1.44
        fDelta = pow ( 10, fDelta*0.05 );

        fValue = fBase + fDelta


        bDelta = 0...127 => -64...-1 et 1...64
        +/- 64 = 0 dB FS
        +/-  1 = -91 dB FS




    Décodage rapide de l'échantillon N

        A = data32
        B = A

        A = A & 0x000007FF

        fOut = fXD4Base[A];

        if (N!=0)
        {
            B = B>>(4+7*N);
            fOut += fXD4Delta[B];
        }




        




    XPCM - eXponential PCM (like a/muLaw)

    Nouvelles idées :
    
        Pour chaque son lu, on mesure le niveau moyen absolu.
        Cette valeur est stockée dans les données du son.
        
        Lors de la lecture, on va utiliser les propriétés de masquage des différents éléments pour déterminer le meilleur filtre
        permettant de maximiser la fluidité et la qualité (mode auto-quality).
        
        Si la durée du mixage dépasse une certaine durée, on va dans un premier temps réduire l'ordre FIR des filtres des sons faibles.
        Cette opération est effectuée sur tous les sons nécessaires tant que le temps de calcul est supérieur à la consigne.
        
        Les ordres FIR disponibles sont : 16, 32, 64 et 128. On considère qu'une réduction globale à 64 n'est pas préjudiciable suivant
        l'ambience générale.
        
        
        L'autre solution...
        Sur-échantillonner systématiquement à 48KHz !
        
        Un son avec un SR=11025 sur 8 bits nécessite 88200 bits par seconde.
        Actuellement tous les sons sont convertis en 32 bits donc ici à 352800 bits par seconde.

        Si on doit ré-échantillonner au chargement à 48KHz, on passe à 1536000 bits/s.        
        Avec l'encodage à DMSP, on arrive à 4 bits par échantillons. Donc un débit de 192000 bits par seconde.
        Le double avantage : Le son est déjà ré-échantillonngé - on gagne du temps !
        On peut effectuer un pre-process sur le son pour par exemple re-créer les aigus via une FFT et du bruit "doux" mutté par le son lui-même
        et son enveloppe.
        
        MAUVAISE IDEE
        
        Par contre le fait de stocker les données 32 bits en 8 bits est une bonne idée. Pour cela on peut utiliser A-Law car il est assez performant.
        Pour encore gagner en qualité...
        
        Il est noté que la différence entre un signal 48000/16/2 et 48000/10/2 n'est pas forcément flagrante lors d'un jeu (d'autant plus que les
        effets sont souvent codés en ADPCM ou en 8bits...).
        Donc l'idéal serait :
        1 - Normaliser le niveau de l'effet et stocker le gain pour l'utiliser en inverse lors de la lecture
        2 - Effectuer un companding -60...0dB vers -42...0dB
        3 - Convertion directe 8 bits (en supprimant les 8 bits de poids faible) le dithering sur 0.1 bits n'apporte pas grand chose...
        
        Avec ce codage, la première harmonique sur un signal de 20Hz se trouve à 60Hz(rang3) à -76dB,        
        h(rang=3) = -76dB
        h(rang=5) = -77dB
        h(rang=7) = -73dB
        etc... avec -70dB au pire...
        
        Avec un signal de 1KHz
        H(3) = -68dB
        H(5) = -66dB
        ... avec -60dB au pire
        
        
        
        
        Après des tests, le XPCM est doté des capacités suivantes :
        - Dynamique de 60dB
        - Rapport S/B allant de 48 à 60dB
        - Distorsion oscillant de -48 à -70dB
        - Quantification équivalente à du 10bits
                
                    
                   
        
        
    Avec un ratio de 0.70, on respecte le modèle 42/60dB
    
    
    Ratio = 0.65
    
    Total Bruit+Distorsion = -40dB
    Niveau minimal = -65dB
    Quantification equivalente = 11 bits
    

    Ratio = 0.70
    
    Total Bruit+Distorsion = 
    Niveau minimal = 
    Quantification equivalente = 
    
    
    Avec un ratio de 0.65, on passe au modèle 42/65dB qui apporte un meilleur rapport S/B global        



    XPCM
        Ratio = 0.65
        Bruit ajouté à modulation pilotée par le signal entrant -72 à -54
        

*/


#include "sam_header.h"

#define     XPCM_RATIO  (0.65F)

FLOAT32     f32XPCM_LUT_Decode[256];
FLOAT32     f32XPCM_NoiseMul;
FLOAT32     f32XPCM_Noise;
FLOAT32     f32XPCM_NoiseLast;


void        SAM_XPCM_Init        ( void );
BYTE        SAM_XPCM_EncodeValue ( FLOAT32 f32AbsoluteValue );
FLOAT32     SAM_XPCM_DecodeValue ( BYTE bXPCMValue );


DWORD       dwXPCM_Noise;


FLOAT32     SAM_XPCM_log10 ( FLOAT32 f32Value );


/*
    Amplitude nécessaire du signal entrant :


*/
BYTE        SAM_XPCM_EncodeValue ( FLOAT32 f32AbsoluteValue )
{
    /*
        Base algo
        
            
    
    */
    FLOAT32 f32Abs;
    FLOAT32 fValue;
    FLOAT32 fNoiseLevel;
    BYTE    bValue;

    //Mesure le niveau du signal entrant
    f32Abs = (f32AbsoluteValue>=0)?(f32AbsoluteValue):(-f32AbsoluteValue);
    if (f32Abs>=0.0001) //-80 to 0dB
    {
        fValue = SAM_XPCM_log10 ( f32Abs ) * 20; //-80...-65...0
        fValue *= 0.5;
        if (fValue<-10) fValue = -10;
        fValue = 10+fValue;

        //fValue = (80+fValue)*0.125; //0...10
        //fValue *= 0.1;

        fNoiseLevel = 0.000001F * (1+fValue);
    }
    else fNoiseLevel = 0.000001F;



    //Ajout de bruit blanc avec accentuant de la HF
    dwXPCM_Noise        = ((dwXPCM_Noise * 1664525)+1013904223);
    f32XPCM_NoiseLast   = ((float)(dwXPCM_Noise&0xFFFF)) * fNoiseLevel; //0.0000015;//0.0000035;
    f32XPCM_Noise       += (f32XPCM_NoiseLast-f32XPCM_Noise)*0.9F;
    f32AbsoluteValue    += f32XPCM_NoiseLast-f32XPCM_Noise;
    
    f32Abs = (f32AbsoluteValue>=0)?(f32AbsoluteValue):(-f32AbsoluteValue);
    if (f32Abs>=0.00001) //-80 to 0dB
    {
        fValue = SAM_XPCM_log10 ( f32Abs ) * 20; //-80...-65...0
        if (f32Abs>0) f32Abs = 0;
        fValue *= XPCM_RATIO;  //-52...-42...0
        fValue += 42.0F;
        fValue *= 3.035F;

        if (fValue<0) fValue = 0;
        if (fValue>127) fValue = 127;
        
        bValue = (BYTE)fValue;
        
        if (f32AbsoluteValue<0) bValue += 128;
    }
    else bValue = 0;
    
    return bValue;
}




void        SAM_XPCM_Init ( void )
{
    long i;
    FLOAT32 fValue;

    f32XPCM_NoiseMul = 1.0F / (float)pow ( 2, 27 ); //30

    for (i=0;i<128;i++)
    {
        fValue = (FLOAT32)i;
        fValue /= 3.035F;
        fValue -= 42.0F;
        fValue /= XPCM_RATIO;
        fValue /= 20.0F;
        fValue = (FLOAT32)pow ( 10, fValue );
        
        f32XPCM_LUT_Decode[i] = fValue;
        f32XPCM_LUT_Decode[i+128] = -fValue;
    }
}

FLOAT32     SAM_XPCM_DecodeValue ( BYTE bXPCMValue )
{
    return f32XPCM_LUT_Decode[bXPCMValue];
}


FLOAT32     SAM_XPCM_log10 ( FLOAT32 f32Value )
{
    FLOAT32 f32Return;
    __asm {
        fld     dword ptr f32Value
        fldlg2
        fxch    st(1)
        fyl2x
        fstp    f32Return
        fstp    st
        mov     eax, f32Return
    }
    return f32Return;
}