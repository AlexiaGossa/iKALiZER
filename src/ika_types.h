#   if ( !defined(_WINDEF_) && !defined(_IKA_TYPES_H_) )
        typedef signed char         INT8;
        typedef signed short        INT16;
        typedef signed long         INT32;
        typedef signed __int64      INT64;
        typedef unsigned char       UINT8;
        typedef unsigned short      UINT16;
        typedef unsigned long       UINT32;
        typedef unsigned __int64    UINT64;

        typedef unsigned char       BYTE;
        typedef unsigned short      WORD;
        typedef unsigned long       DWORD;
        typedef unsigned __int64    QWORD;
        
        typedef float               FLOAT;
        
        typedef unsigned long       BOOL;
#   endif



#   ifndef _IKA_TYPES_H_
#       define _IKA_TYPES_H_

        typedef float               FLOAT32;
        typedef double              FLOAT64;

        typedef FLOAT32             VECT;
        typedef VECT                VECT2[2];
        typedef VECT                VECT3[3];
        typedef VECT                VECT4[4];
#   endif


