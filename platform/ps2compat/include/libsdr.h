/* libsdr: EE-side remote control of the IOP sound library, served by the port's software mixer. */
#pragma once
#include "eetypes.h"
#include "sdrcmd.h"
#include "sdmacro.h"

int sceSdRemoteInit(void);
int sceSdRemote(int arg, int command, ...);
int sceSdrChangeThreadPriority(int, int);
