#pragma once
/* SPU2 parameter/transfer constants (libsd encoding). */
#define SD_CORE_0 0
#define SD_CORE_1 1
#define SD_P_VOLL  (0x00 << 8)
#define SD_P_VOLR  (0x01 << 8)
#define SD_P_PITCH (0x02 << 8)
#define SD_P_MVOLL ((0x09 << 8) + (0x01 << 7))
#define SD_P_MVOLR ((0x0a << 8) + (0x01 << 7))
#define SD_P_EVOLL ((0x0b << 8) + (0x01 << 7))
#define SD_P_EVOLR ((0x0c << 8) + (0x01 << 7))
#define SD_P_AVOLL ((0x0d << 8) + (0x01 << 7))
#define SD_P_AVOLR ((0x0e << 8) + (0x01 << 7))
#define SD_P_BVOLL ((0x0f << 8) + (0x01 << 7))
#define SD_P_BVOLR ((0x10 << 8) + (0x01 << 7))

#define SD_TRANS_MODE_WRITE      0
#define SD_TRANS_MODE_READ       1
#define SD_TRANS_MODE_STOP       2
#define SD_TRANS_MODE_WRITE_FROM 3
#define SD_BLOCK_ONESHOT (0 << 4)
#define SD_BLOCK_LOOP    (1 << 4)
#define SD_TRANS_BY_DMA  (0 << 3)
#define SD_TRANS_BY_IO   (1 << 3)

#define SD_INIT_COLD 0
#define SD_INIT_HOT  1
