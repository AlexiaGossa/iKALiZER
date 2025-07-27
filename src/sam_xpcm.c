/*

    XD4e

    Un bloc de 32 bits
    - Contenant un exposant et 4 mantisses


    
    Les possibilit�s

    Exposant    Mantisses
    4 bits      4 x 7 bits
    8 bits      4 x 6 bits
    12 bits     4 x 5 bits


    Le choix 4bits + 4x7bits semble interressant :

    L'exposant est une valeur en dB, il indique une valeur absolue
    Avec une �chelle de 16 valeurs, on assigne 3 dB pour chaque valeur.
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

    Pour obtenir la valeur r�elle, il faut utiliser un calcul assez complexe :

    ValeurReelle = Exposant * (1+MantisseValeur)/64 * MantisseSigne

    Ainsi, pour un bloc de 4 points, on dispose par point d'une profondeur de -36dB par rapport au total
    

    Th�orie :
    On doit s'attendre � un capacit� d'att�nuation du bruit assez importante puisque l'on travaille
    sur des blocs de 4 �chantillons.
    Certe le rapport S/B d'un bloc n'est pas exceptionnel puisque l'on frise avec 36dB, mais sur un nombre de
    blocs plus important, le rapport S/B doit atteindre dans le meilleur des cas, un peu plus de 80dB.
    Ainsi, on reste assez proche d'un encodage 16 bits.

    En modifiant l�g�rement la courbe de valeur mantisse, on doit arriver � passer de 36 � 45.
    Cela porterait la rapport S/B maximal � 90 dB


    Pour un d�codage acc�l�r�, il est n�cessaire de cr�er des LUT.
    On doit utiliser 16 LUT de 128 valeurs soit un total de 2048 valeurs soit 8Ko.
    Par rapport au XD4, on �vite un calcul sur les flottants car la bonne valeur est directement extraite de la LUT.











    XD4

    Am�lioration
    - Pour accroitre la qualit� de l'encodage, il est possible d'ajouter un bruit blanc � -60dB

    Avantages du tra�tement XD4
    - 36 dB de rapport S/B minimal
    - 91 dB de rapport S/B maximal
    - Vitesse de d�codage � peine plus longue que le XPCM8

    Inconv�nient
    - Blocs de 4 �chantillons... plus complexe � g�rer pour le st�r�o
    - LUT de 2048 et 128 entr�es soit un total de 8704 octets



    4 �chantillons sur 4 octets / 32 bits

    3 �chantillons sur 7 bits
    1 �chantillon sur 11 bits


    Echantillon BASE - 11 bits

        Codage l�ger XPCM
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




    D�codage rapide de l'�chantillon N

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

    Nouvelles id�es :
    
        Pour chaque son lu, on mesure le niveau moyen absolu.
        Cette valeur est stock�e dans les donn�es du son.
        
        Lors de la lecture, on va utiliser les propri�t�s de masquage des diff�rents �l�ments pour d�terminer le meilleur filtre
        permettant de maximiser la fluidit� et la qualit� (mode auto-quality).
        
        Si la dur�e du mixage d�passe une certaine dur�e, on va dans un premier temps r�duire l'ordre FIR des filtres des sons faibles.
        Cette op�ration est effectu�e sur tous les sons n�cessaires tant que le temps de calcul est sup�rieur � la consigne.
        
        Les ordres FIR disponibles sont : 16, 32, 64 et 128. On consid�re qu'une r�duction globale � 64 n'est pas pr�judiciable suivant
        l'ambience g�n�rale.
        
        
        L'autre solution...
        Sur-�chantillonner syst�matiquement � 48KHz !
        
        Un son avec un SR=11025 sur 8 bits n�cessite 88200 bits par seconde.
        Actuellement tous les sons sont convertis en 32 bits donc ici � 352800 bits par seconde.

        Si on doit r�-�chantillonner au chargement � 48KHz, on passe � 1536000 bits/s.        
        Avec l'encodage � DMSP, on arrive � 4 bits par �chantillons. Donc un d�bit de 192000 bits par seconde.
        Le double avantage : Le son est d�j� r�-�chantillonng� - on gagne du temps !
        On peut effectuer un pre-process sur le son pour par exemple re-cr�er les aigus via une FFT et du bruit "doux" mutt� par le son lui-m�me
        et son enveloppe.
        
        MAUVAISE IDEE
        
        Par contre le fait de stocker les donn�es 32 bits en 8 bits est une bonne id�e. Pour cela on peut utiliser A-Law car il est assez performant.
        Pour encore gagner en qualit�...
        
        Il est not� que la diff�rence entre un signal 48000/16/2 et 48000/10/2 n'est pas forc�ment flagrante lors d'un jeu (d'autant plus que les
        effets sont souvent cod�s en ADPCM ou en 8bits...).
        Donc l'id�al serait :
        1 - Normaliser le niveau de l'effet et stocker le gain pour l'utiliser en inverse lors de la lecture
        2 - Effectuer un companding -60...0dB vers -42...0dB
        3 - Convertion directe 8 bits (en supprimant les 8 bits de poids faible) le dithering sur 0.1 bits n'apporte pas grand chose...
        
        Avec ce codage, la premi�re harmonique sur un signal de 20Hz se trouve � 60Hz(rang3) � -76dB,        
        h(rang=3) = -76dB
        h(rang=5) = -77dB
        h(rang=7) = -73dB
        etc... avec -70dB au pire...
        
        Avec un signal de 1KHz
        H(3) = -68dB
        H(5) = -66dB
        ... avec -60dB au pire
        
        
        
        
        Apr�s des tests, le XPCM est dot� des capacit�s suivantes :
        - Dynamique de 60dB
        - Rapport S/B allant de 48 � 60dB
        - Distorsion oscillant de -48 � -70dB
        - Quantification �quivalente � du 10bits
                
                    
                   
        
        
    Avec un ratio de 0.70, on respecte le mod�le 42/60dB
    
    
    Ratio = 0.65
    
    Total Bruit+Distorsion = -40dB
    Niveau minimal = -65dB
    Quantification equivalente = 11 bits
    

    Ratio = 0.70
    
    Total Bruit+Distorsion = 
    Niveau minimal = 
    Quantification equivalente = 
    
    
    Avec un ratio de 0.65, on passe au mod�le 42/65dB qui apporte un meilleur rapport S/B global        



    XPCM
        Ratio = 0.65
        Bruit ajout� � modulation pilot�e par le signal entrant -72 � -54
        

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
    Amplitude n�cessaire du signal entrant :


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