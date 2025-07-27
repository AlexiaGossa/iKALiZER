#include "sam_header.h"
#include "sam_data.h"


    char szAvecAccent[]="ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜáíóúñÑÌÁÂÀãÃĞÊËÈiÍÎÏÌÓÔÒõÕÚÛÙıİ"; //ß ou B en allemand représente 2 's' soit 'ss'
    char szSansAccent[]="CueaaaaceeeiiiaaeaaooouuyOUaiounNIAAAaADEEEiIIIIOOOoOUUUyY";
    //char szSansAccent[]="CUEAAAACEEEIIIAAEAAOOOUUYOUAIOUNNIAAAAADEEEIIIIIOOOOOUUUYY";

    void SAM_StringAccentKill ( char * psz )
    {
	    int i,j;
        char c, *p;
        for (i=0;psz[i]!=0;i++)
        {
		    c = (unsigned char)psz[i];
            
            //Convertion des accents
            p = strchr ( szAvecAccent, c );
            if (p!=NULL)
            {
               j = (int) ( p - szAvecAccent );
               c = (unsigned char)szSansAccent[j];
		    }       
            psz[i] = (char)c;         
        }
    }
