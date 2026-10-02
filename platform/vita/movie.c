/*
 * Movie playback behind the game's Sofdec-style front end (ps2_sfd_mw.c).
 *
 * This takes the place of ps2_MovieFunc.c, the IPU/libmpeg decoder. Until
 * the SceAvPlayer backend lands, a movie plays for a single server call and
 * then ends: WaitPrePlayMovie needs to see PLAYING once before PlayMovieMain
 * can see PLAYEND, or the caller waits for playback forever.
 */
#include <string.h>
#include "types.h"
#include "ps2_MovieFunc.h"
#include "recvx_platform.h"
#include "iop/iop.h"

int movie_draw;
StrFile infile;
VoBuf voBuf;
u_long128 test_tag[1400] __attribute__((aligned(64)));
RMI_WORK rmi;
MDSIZE_WORK mdSize;
static int served;

void initAll(void)
{
    memset(&rmi, 0, sizeof(rmi));
    rmi.uiContFlag = 1;
    rmi.iMovieState = 2;   /* MWE_PLY_STAT_PLAYING */
    served = 0;
    RECVX_LOG("movie: playback not implemented, skipping (%u bytes)", infile.size);
}

void readMpeg(void)
{
    movie_draw = 1;
    if (rmi.iMovieState == 2 && served++)
        rmi.iMovieState = 3;   /* MWE_PLY_STAT_PLAYEND */
}

void setImageTag(unsigned int *tags, void *image) { (void)tags; (void)image; }
void vbrank_draw(void) {}
void termAll(void) {}
void changeInputVolume(u_int val) { (void)val; }

int sendToIOP(int dst, u_char *src, int size)
{
    iop_lock();
    memcpy(iop_ptr(dst), src, size);
    iop_unlock();
    return size;
}
