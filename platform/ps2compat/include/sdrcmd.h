#pragma once
/* Command numbers for sceSdRemote(); values are local to the port. */
enum {
    rSdInit = 0x8000, rSdSetParam, rSdGetParam, rSdSetSwitch, rSdGetSwitch, rSdSetAddr,
    rSdGetAddr, rSdSetCoreAttr, rSdGetCoreAttr, rSdNote2Pitch, rSdPitch2Note, rSdProcBatch,
    rSdProcBatchEx, rSdVoiceTrans, rSdBlockTrans, rSdVoiceTransStatus, rSdBlockTransStatus,
    rSdSetTransCallback, rSdSetIRQCallback, rSdSetEffectAttr, rSdGetEffectAttr, rSdClearEffectWorkArea,
    rSdSetTransIntrHandler, rSdSetSpu2IntrHandler, rSdChangeThreadPriority
};
