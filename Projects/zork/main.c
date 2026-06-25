/*
** Zork I: The Great Underground Empire -- Sega Saturn port
**
** Game (c) 1980 by Infocom, Inc.
** C port and parser (c) 2021 by Donnie Russell II.
** Sega Saturn integration (Jo Engine) built on the keyboard demo.
**
** The engine is initialised here, then control is handed to the original
** Zork GameLoop(). The Saturn I/O layer in utility.cpp drives the video
** (slSynch) and reads the keyboard each frame, so GameLoop() can keep its
** original blocking structure.
*/

#include <jo/jo.h>

/* Defined in the Zork sources (utility.cpp / generated _parser.cpp), which are
** compiled as C for the Saturn, so these have C linkage like everything here. */
void            SetRandomSeed(unsigned int seed);
void            GameLoop(void);

void            jo_main(void)
{
    jo_core_init(JO_COLOR_Black);

    SetRandomSeed(0);   /* 0 => seed from the video-frame counter */
    GameLoop();         /* blocking REPL; renders + polls the keyboard itself */

    /* GameLoop() only returns when the player chooses to exit. */
    for (;;)
        slSynch();
}

/*
** END OF FILE
*/
