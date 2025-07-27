#include "sam_header.h"
#include "sam_data.h"

#define INTERPOLATION_APPROX_COUNT  1024

long        SAM_INTERPOLATOR_Init ( void )
{
    float fInterpolatorFIR[16 * INTERPOLATION_APPROX_COUNT];
    float fRatio;
    long i,j;
    
    fRatio = 0.5F/((float)INTERPOLATION_APPROX_COUNT);

    //Génération d'un FIR avec une fenêtre de Lanczos modifiée pour A=1,7    
    SAM_FIR_Design (
        INTERPOLATION_APPROX_COUNT * 16,
        fInterpolatorFIR,
        fRatio,
        sam_FIR_LANCZOS,
        1.7F );
        
    //Génération de la table de polynomes FIR (on les prépares à l'envers)
    for (i=0;i<INTERPOLATION_APPROX_COUNT;i++)
    {
        for (j=0;j<16;j++)
        {
            samData_f32InterpolationData_1024_16[(i*16) + (15-j)] =
                fInterpolatorFIR[(j*INTERPOLATION_APPROX_COUNT) + i] * INTERPOLATION_APPROX_COUNT;
        }
    }

    return 0;
}