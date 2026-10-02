#include "../../../ps2/veronica/prog/ps2_sg_sycfg.h"

int SoundMode;

// 100% matching! 
Sint32 syCfgExit( Void  )
{

    return 0; /* fell off the end on the EE */
}

// 100% matching! 
Sint32 syCfgGetSoundMode( Sint32* pnMode )
{ 
    *pnMode = SoundMode; 
    
    return 0;
}

// 100% matching! 
Sint32 syCfgInit( Void* )
{

    return 0; /* fell off the end on the EE */
}

// 100% matching! 
Sint32 syCfgSetSoundMode( Sint32 nMode )
{ 
    SoundMode = nMode; 
    return 0; /* fell off the end on the EE */
}
