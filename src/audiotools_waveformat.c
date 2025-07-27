#include "audiotools.h"
//#include <MMREG.H>

    #define WAVEFORMAT_MAKEFOURCC(ch0, ch1, ch2, ch3)                               \
		        ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |                   \
		        ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

//D�sactive l'alignement
    
//#pragma


    //Structure WAVE
    /*
    typedef struct {

        long    RIFF_str;           //RIFF = 0x46464952
        long    W_Lenght;           //Nombre d'octets restant apr�s de 8 premiers octets

        long    WAVE_str;           //WAVE = 0x45564157

        long    fmt_str;            //fmt_ = 0x20746D66
        long    Head_Lenght;        //G�n�ralement 16

        short   Format;             //1 = PCM
        short   Channels;           //1=mono 2=stereo

        long    Frequency;          //En Hz
        long    Byte_sec;           //Octets par seconde : 176400 = 44100/16/2, 


        short   Byte_smp;           //Octets par �chantillon : 44100/16/2 = 4, 44100/8/2 = 2...
        short   Quantification;     //Nombre de bits

        long    data_str;           //data = 0x61746164
        long    Lenght;             //Nombre d'octets restants apr�s ce point
    }WAVEFORMAT; */

    #pragma pack(push, 1)
    typedef struct {
        DWORD           dwFourCC_RIFF;
        DWORD           dwRemainSizeBytesAfterRIFF;
        DWORD           dwFourCC_WAVE;
        DWORD           dwFourCC_fmt;
        DWORD           dwWaveFormatSizeBytes;
        wioWAVEFORMAT   wioWaveFormat;
        DWORD           dwFourCC_data;
        DWORD           dwDataSizeBytes;
    } wioBASICFILEHEADER;
    #pragma pack(pop)


    typedef struct {
        FILE                *pFile;             //Pointeur sur le fichier en acc�s (lecture)
        wioWAVEFORMAT       wioWaveFormat;      //Format des donn�es
        DWORD               dwDataBytes;        //Nombre d'octets de donn�es audio du fichier (le total)
        DWORD               dwRemainDataBytes;  //Nombre d'octets de donn�es restant � lire
        wioBASICFILEHEADER  wioBasicFileHeader; //Ent�te de base
        DWORD               dwDataBytesWritten; //Nombre d'octets de donn�es audio �crites
    } wioFILE;




    /*
     *
     *  R�cup�ration des sp�cifications du fichier ouvert
     *
     */

    long WaveIO_IFileGetSpecs ( void * pWaveIO, wioWAVEFORMAT * pwioWaveFormat, DWORD * pdwTotalSamples )
    {
        wioFILE * pwioFile;

        //V�rification du param�tre
        if (!pWaveIO) return -1;

        //Les donn�es en ouverture
        pwioFile = (wioFILE *)pWaveIO;

        //Le format
        if (pwioWaveFormat) memcpy ( pwioWaveFormat, &(pwioFile->wioWaveFormat), sizeof(wioWAVEFORMAT) );

        //La longueur en �chantillons
        if (pdwTotalSamples) *pdwTotalSamples = pwioFile->dwDataBytes / pwioFile->wioWaveFormat.wBlocAlign;

        return 0;
    }

    long WaveIO_IFileRead ( void * pWaveIO, void * pAudioBuffer, DWORD dwTotalSamplesToRead, DWORD * pdwTotalSamplesRead )
    {
        wioFILE     * pwioFile;
        DWORD       dwSamplesRead;

        //V�rification des param�tres
        if ((!pWaveIO)||(!pAudioBuffer)) return -1;

        //Les donn�es en ouverture
        pwioFile = (wioFILE *)pWaveIO;

        //V�rification du pointeur de fichier
        if (!pwioFile->pFile) return -1;

        //Lecture des donn�es
        dwSamplesRead = fread ( 
            pAudioBuffer, 
            pwioFile->wioWaveFormat.wBlocAlign, 
            dwTotalSamplesToRead, 
            pwioFile->pFile );

        //Renvoi de la quantit� des donn�es lues
        if (pdwTotalSamplesRead) *pdwTotalSamplesRead = dwSamplesRead;

        //Mise � jour dans les infos du fichier...
        pwioFile->dwRemainDataBytes -= dwSamplesRead * pwioFile->wioWaveFormat.wBlocAlign;

        return 0;
    }

    long WaveIO_IFileClose ( void * pWaveIO )
    {
        wioFILE * pwioFile;

        //V�rification du param�tre
        if (!pWaveIO) return -1;

        //Les donn�es en ouverture
        pwioFile = (wioFILE *)pWaveIO;

        //V�rification du pointeur de fichier
        if (!pwioFile->pFile) return -1;

        //Fermeture et lib�ration
        fclose ( pwioFile->pFile );
        AUDIOTOOLS_FREE ( pwioFile );

        return 0;
    }



    long WaveIO_IFileOpen ( void ** pWaveIO, char * pszFileNameToRead )
    {
        wioFILE * pwioFile;
        DWORD dwChunkFourCC;
        DWORD dwChunkBytesSize;
        DWORD dwChunkSeek;
        long lEndRead;

        //V�rification des param�tres
        if ((!pWaveIO)||(!pszFileNameToRead)) return -1;

        //Allocation du tampon
        pwioFile = (wioFILE *) AUDIOTOOLS_MALLOC ( sizeof(wioFILE) );
        if (!pwioFile) return -1;
        memset ( pwioFile, 0, sizeof(wioFILE) );

        //Ouverture du fichier
        pwioFile->pFile = fopen ( pszFileNameToRead, "rb" );
        if (!pwioFile->pFile)
        {
            AUDIOTOOLS_FREE ( pwioFile );
            return -1;
        }

        //Lecture du fichier
        lEndRead = 0;
        do {

            //Lecture de la chaine du Chunk
            if (fread ( &dwChunkFourCC, sizeof(dwChunkFourCC), 1, pwioFile->pFile )!=1)
            {
                lEndRead = 1;
            }
            else
            {
                switch (dwChunkFourCC)
                {
                    //RIFF... (rien de tr�s interressant)
                    case WAVEFORMAT_MAKEFOURCC('R', 'I', 'F', 'F'):
                        dwChunkSeek = 4;
                        break;

                    //WAVE... (Pas terrible !)
                    case WAVEFORMAT_MAKEFOURCC('W', 'A', 'V', 'E'):
                        dwChunkSeek = 0;
                        break;

                    //"fmt "... Ah! Le format des donn�es !
                    case WAVEFORMAT_MAKEFOURCC('f', 'm', 't', ' '):
                        fread ( &dwChunkBytesSize, sizeof(dwChunkBytesSize), 1, pwioFile->pFile );
                        fread ( &pwioFile->wioWaveFormat, sizeof(wioWAVEFORMAT), 1, pwioFile->pFile ); 
                        dwChunkSeek = dwChunkBytesSize - sizeof(wioWAVEFORMAT);
                        break;

                    //"data"... Les donn�es !
                    case WAVEFORMAT_MAKEFOURCC('d', 'a', 't', 'a'):
                        fread ( &dwChunkBytesSize, sizeof(dwChunkBytesSize), 1, pwioFile->pFile );
                        pwioFile->dwDataBytes           = dwChunkBytesSize;
                        pwioFile->dwRemainDataBytes     = dwChunkBytesSize;
                        lEndRead                        = 1;
                        dwChunkSeek                     = 0;
                        break;

                    //Qu'est ce donc ?
                    default:
                        fread ( &dwChunkBytesSize, sizeof(dwChunkBytesSize), 1, pwioFile->pFile );
                        dwChunkSeek = dwChunkBytesSize;
                        break;
                
                }
            }

            //On se d�place aux prochaines donn�es
            if ((dwChunkSeek)&&(!lEndRead)) fseek ( pwioFile->pFile, dwChunkSeek, SEEK_CUR );

        } while(!lEndRead);

        //Les donn�es lisibles ?
        if ( ( (pwioFile->wioWaveFormat.wFormatTag==wioWAVE_FORMAT_PCM) || (pwioFile->wioWaveFormat.wFormatTag==wioWAVE_FORMAT_IEEE_FLOAT) ) &&
             ( pwioFile->dwDataBytes ) && (pwioFile->wioWaveFormat.wBlocAlign) )
        {
            //Oui ! Format standard INT ou FLOAT
            *pWaveIO = (void *)pwioFile;
            return 0;
        }

        //Donn�es illisibles
        AUDIOTOOLS_FREE ( pwioFile );
        return -1;
    }

    long WaveIO_OFileOpen               ( void ** pWaveIO, char * pszFileNameToWrite )
    {
        wioFILE * pwioFile;

        //V�rification des param�tres
        if ((!pWaveIO)||(!pszFileNameToWrite)) return -1;

        //Allocation du tampon
        pwioFile = (wioFILE *) AUDIOTOOLS_MALLOC ( sizeof(wioFILE) );
        if (!pwioFile) return -1;
        memset ( pwioFile, 0, sizeof(wioFILE) );

        //Ouverture du fichier (avec lecture et �criture)
        pwioFile->pFile = fopen ( pszFileNameToWrite, "w+b" );
        if (!pwioFile->pFile)
        {
            AUDIOTOOLS_FREE ( pwioFile );
            return -1;
        }

        //Ecriture des marqueurs principaux pour un WAVE standard
        pwioFile->wioBasicFileHeader.dwFourCC_RIFF          = WAVEFORMAT_MAKEFOURCC('R', 'I', 'F', 'F');
        pwioFile->wioBasicFileHeader.dwFourCC_WAVE          = WAVEFORMAT_MAKEFOURCC('W', 'A', 'V', 'E');
        pwioFile->wioBasicFileHeader.dwFourCC_fmt           = WAVEFORMAT_MAKEFOURCC('f', 'm', 't', ' ');
        pwioFile->wioBasicFileHeader.dwFourCC_data          = WAVEFORMAT_MAKEFOURCC('d', 'a', 't', 'a');
        pwioFile->wioBasicFileHeader.dwWaveFormatSizeBytes  = sizeof(wioWAVEFORMAT);

        //Ecriture de l'ent�te
        if (!fwrite ( &pwioFile->wioBasicFileHeader, sizeof(wioBASICFILEHEADER), 1, pwioFile->pFile ))
        {
            AUDIOTOOLS_FREE ( pwioFile );
            return -1;
        }

        //Fin de l'�criture de la base
        *pWaveIO = (void *)pwioFile;
        return 0;
    }

    /*
     *
     *  Sp�cification du format du fichier
     *
     */
    long WaveIO_OFileSetSpecs           ( void * pWaveIO, wioWAVEFORMAT * pwioWaveFormat )
    {
        wioFILE * pwioFile;

        //V�rification des param�tres
        if ((!pWaveIO)||(!pwioWaveFormat)) return -1;

        //Les donn�es en ouverture
        pwioFile = (wioFILE *)pWaveIO;

        //Ecriture du format
        if ( ((pwioWaveFormat->wFormatTag==wioWAVE_FORMAT_PCM)||(pwioWaveFormat->wFormatTag==wioWAVE_FORMAT_IEEE_FLOAT)) &&
             (pwioWaveFormat->wBlocAlign) )
        {
            memcpy ( &(pwioFile->wioWaveFormat), pwioWaveFormat, sizeof(wioWAVEFORMAT) );
            return 0;
        }

        return -1;
    }

    long WaveIO_OFileWrite              ( void * pWaveIO, void * pAudioBuffer, DWORD dwSamplesToWrite, DWORD * pdwSamplesWritten )
    {
        wioFILE * pwioFile;
        DWORD   dwSamplesWritten;

        //V�rification des param�tres
        if ((!pWaveIO)||(!pAudioBuffer)) return -1;

        //Les donn�es en ouverture
        pwioFile = (wioFILE *)pWaveIO;

        //V�rification du pointeur de fichier
        if (!pwioFile->pFile) return -1;

        //On v�rifie que l'on dispose de la taille d'un �l�ment...
        if (!pwioFile->wioWaveFormat.wBlocAlign) return -1;

        //Ecriture des donn�es
        dwSamplesWritten = fwrite ( 
            pAudioBuffer, 
            pwioFile->wioWaveFormat.wBlocAlign, 
            dwSamplesToWrite, 
            pwioFile->pFile );

        //Mise � jour dans les infos du fichier...
        pwioFile->dwDataBytesWritten += dwSamplesWritten * pwioFile->wioWaveFormat.wBlocAlign;

        //Renvoi de la quantit� des donn�es �crites
        if (pdwSamplesWritten) *pdwSamplesWritten = dwSamplesWritten;

        //Avons-nous pu tout �crire ?
        if (dwSamplesWritten!=dwSamplesToWrite) return -2;

        return 0;
    }

    long WaveIO_OFileClose              ( void * pWaveIO )
    {
        wioFILE * pwioFile;

        //V�rification des param�tres
        if (!pWaveIO) return -1;

        //Les donn�es en ouverture
        pwioFile = (wioFILE *)pWaveIO;

        //V�rification du pointeur de fichier
        if (!pwioFile->pFile) return -1;

        //Mise � jour du header basique
        memcpy ( &(pwioFile->wioBasicFileHeader.wioWaveFormat), &(pwioFile->wioWaveFormat), sizeof(wioWAVEFORMAT) );
        pwioFile->wioBasicFileHeader.dwDataSizeBytes                =   pwioFile->dwDataBytesWritten;
        pwioFile->wioBasicFileHeader.dwRemainSizeBytesAfterRIFF     =   pwioFile->dwDataBytesWritten + sizeof(wioBASICFILEHEADER) - 16;

        //Place le pointeur au d�but du fichier
        fseek ( pwioFile->pFile, 0, SEEK_SET );

        //R�-�criture de l'ent�te
        if (!fwrite ( &pwioFile->wioBasicFileHeader, sizeof(wioBASICFILEHEADER), 1, pwioFile->pFile ))
        {
            fseek ( pwioFile->pFile, 0, SEEK_END );
            return -1;
        }

        fclose ( pwioFile->pFile );
        AUDIOTOOLS_FREE ( pwioFile );
        return 0;
    }

    