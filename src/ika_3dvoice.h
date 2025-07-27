ika_ALIGN_SPEC(32)   typedef struct {

                    VECT3                   v3Position;
                    FLOAT32                 f32AbsoluteLevelDecibels;
                    VECT3                   v3AxisConeOrigin;
                    VECT3                   v3AxisConeAngle;
                    VECT3                   v3AxisConeAttenuationDecibelsPerRadian;
    
} ika_3DVOICE;       


typedef struct {
                    VECT3                   v3Position;
                    VECT3                   v3Axis;
                    
                    //Il manque encore le rendu pour la présence dans l'eau

} ika_3DLISTENER;


typedef struct {


} ika_3DOBJECT;