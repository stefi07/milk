/**
 * @file    fps_CTRLscreen.c
 * @brief   FPS control TUI
 */

#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <time.h>

#include "CommandLineInterface/CLIcore.h"

#include "COREMOD_tools/COREMOD_tools.h"

#include "CommandLineInterface/timeutils.h"

#include "fpsCTRL_TUI_process_user_key.h"
#include "fps/fps_GetTypeString.h"
#include "fps/fps_disconnect.h"
#include "fps/fps_outlog.h"
#include "fps/fps_process_fpsCMDarray.h"
#include "fps/fps_read_fpsCMD_fifo.h"
#include "fps/fps_scan.h"

#include "TUItools.h"

#include "fpsCTRL_FPSdisplay.h"
#include "print_nodeinfo.h"
#include "level0node_summary.h"

#include "scheduler_display.h"

static short unsigned int wrow, wcol;



inline static void
fpsCTRLscreen_print_DisplayMode_status(
    int fpsCTRL_DisplayMode,
    int NBfps
)
{
    DEBUG_TRACE_FSTART();

    int  stringmaxlen = 500;
    char monstring[stringmaxlen];

    screenprint_setbold();

    if (snprintf(monstring,
                 stringmaxlen,
                 "[%d x %d] [PID %d] FUNCTION PARAMETER MONITOR: PRESS (x) TO "
                 "STOP, (h) FOR HELP [%d FPS]",
                 wrow,
                 wcol,
                 (int) getpid(),
                 NBfps) < 0)
    {
        PRINT_ERROR("snprintf error");
    }
    TUI_print_header(monstring, '-');
    screenprint_unsetbold();
    TUI_newline();

    if (fpsCTRL_DisplayMode == 1)
    {
        screenprint_setreverse();
        TUI_printfw("[h] Help");
        screenprint_unsetreverse();
    }
    else
    {
        TUI_printfw("[h] Help");
    }
    TUI_printfw("   ");

    if (fpsCTRL_DisplayMode == 2)
    {
        screenprint_setreverse();
        TUI_printfw("[F2] FPS CTRL");
        screenprint_unsetreverse();
    }
    else
    {
        TUI_printfw("[F2] FPS CTRL");
    }
    TUI_printfw("   ");

    if (fpsCTRL_DisplayMode == 3)
    {
        screenprint_setreverse();
        TUI_printfw("[F3] Sequencer");
        screenprint_unsetreverse();
    }
    else
    {
        TUI_printfw("[F3] Sequencer");
    }
    TUI_newline();
    DEBUG_TRACE_FEXIT();
}




/**
 * @brief Print help
 *
 */
inline static void fpsCTRLscreen_print_help()
{
    DEBUG_TRACE_FSTART();
    // int attrval = A_BOLD;

    TUI_newline();
    print_help_entry("x", "Exit");

    TUI_newline();
    TUI_printfw("============ SCREENS");
    TUI_newline();
    print_help_entry("h/F2/F3", "Help/Control/Sequencer screen");
    print_help_entry("v/V", "verbose mode on/off");

    TUI_newline();
    TUI_printfw("============ OTHER");
    TUI_newline();
    print_help_entry("s", "rescan");
    print_help_entry("T / CRTL+t",
                     "initialize (T)mux session / kill (t)mux session");
    print_help_entry("CTRL+e", "(E)rase FPS and tmux sessions");
    print_help_entry("O / CTRL+o", "start/stop C(O)NF process");
    print_help_entry("u", "(u)pdate CONF process");
    print_help_entry("R / CTRL+r", "start/stop (R)UN process");
    print_help_entry("l", "list all entries");
    print_help_entry("f", "export fps content to datadir file");
    print_help_entry(">", "export fpsdatadir values to fpsconfdir");
    print_help_entry("<", "import/load values from fpsconfdir to fps");
    print_help_entry("P", "(P)rocess input file \"confscript\"");
    TUI_printfw("        format: setval <paramfulname> <value>");
    TUI_newline();

    DEBUG_TRACE_FEXIT();
}




/** @brief runs fpsCTRL GUI
 *
 * ## Purpose
 *
 * Automatically build simple ASCII GUI from function parameter structure (fps) name mask
 *
 *
 *
 */
errno_t functionparameter_CTRLscreen(
    uint32_t mode,
    char    *fpsnamemask,
    char    *fpsCTRLfifoname
)
{
    DEBUG_TRACE_FSTART();

    FPSCTRL_PROCESS_VARS fpsCTRLvar;

    // keyword tree
    KEYWORD_TREE_NODE *keywnode;


    int       loopOK  = 1;
    long long loopcnt = 0;

    long NBtaskLaunchedcnt = 0;


    // What to run ?
    // disable for testing
    int run_display = 1;
    loopOK          = 1;

    struct timespec tnow;
    clock_gettime(CLOCK_REALTIME, &tnow);
    data.FPS_TIMESTAMP = tnow.tv_sec;
    strcpy(data.FPS_PROCESS_TYPE, "ctrl");

    functionparameter_outlog("FPSCTRL", "START\n");

    DEBUG_TRACEPOINT("function start");

    // initialize fpsCTRLvar
    fpsCTRLvar.exitloop              = 0;
    fpsCTRLvar.mode                  = mode;
    fpsCTRLvar.nodeSelected          = 1;
    fpsCTRLvar.run_display           = run_display;
    fpsCTRLvar.fpsindexSelected      = 0;
    fpsCTRLvar.pindexSelected        = 0;
    fpsCTRLvar.directorynodeSelected = 0;
    fpsCTRLvar.currentlevel          = 0;
    fpsCTRLvar.direction             = 1;
    strcpy(fpsCTRLvar.fpsnamemask, fpsnamemask);
    strcpy(fpsCTRLvar.fpsCTRLfifoname, fpsCTRLfifoname);

    fpsCTRLvar.fpsCTRL_DisplayMode = 2;
    // 1: [h]  help
    // 2: [F2] list of conf and run
    // 3: [F3] fpscmdarray




    // All parameters held in this array
    //
    keywnode = (KEYWORD_TREE_NODE *) malloc(sizeof(KEYWORD_TREE_NODE) *
                                            NB_KEYWNODE_MAX);
    if (keywnode == NULL)
    {
        PRINT_ERROR("malloc error: can't allocate keywnode");
        abort();
    }
    for (int kn = 0; kn < NB_KEYWNODE_MAX; kn++)
    {
        strcpy(keywnode[kn].keywordfull, "");
        for (int ch = 0; ch < MAX_NB_CHILD; ch++)
        {
            keywnode[kn].child[ch] = 0;
        }
    }


    // Set up instruction buffer to sequence commands
    //
    FPSCTRL_TASK_ENTRY *fpsctrltasklist;
    fpsctrltasklist = (FPSCTRL_TASK_ENTRY *) malloc(sizeof(FPSCTRL_TASK_ENTRY) *
                      NB_FPSCTRL_TASK_MAX);
    if (fpsctrltasklist == NULL)
    {
        PRINT_ERROR("malloc error");
        abort();
    }
    for (int cmdindex = 0; cmdindex < NB_FPSCTRL_TASK_MAX; cmdindex++)
    {
        fpsctrltasklist[cmdindex].status = 0;
        fpsctrltasklist[cmdindex].queue  = 0;
    }

    // Set up task queue list
    //
    FPSCTRL_TASK_QUEUE *fpsctrlqueuelist;
    fpsctrlqueuelist = (FPSCTRL_TASK_QUEUE *) malloc(
                           sizeof(FPSCTRL_TASK_QUEUE) * NB_FPSCTRL_TASKQUEUE_MAX);
    if (fpsctrlqueuelist == NULL)
    {
        PRINT_ERROR("malloc error");
        abort();
    }
    for (int queueindex = 0; queueindex < NB_FPSCTRL_TASKQUEUE_MAX;
            queueindex++)
    {
        fpsctrlqueuelist[queueindex].priority = 1; // 0 = not active
    }

    // catch signals (CTRL-C etc)
    //
    set_signal_catch();

    // fifo
    fpsCTRLvar.fpsCTRLfifofd = open(fpsCTRLvar.fpsCTRLfifoname, O_RDWR | O_NONBLOCK);
    long fifocmdcnt = 0;

    for (int level = 0; level < MAXNBLEVELS; level++)
    {
        fpsCTRLvar.GUIlineSelected[level] = 0;
    }


    for (int kindex = 0; kindex < NB_KEYWNODE_MAX; kindex++)
    {
        keywnode[kindex].NBchild = 0;
    }


    {
        long NBpindex = 0;
        functionparameter_scan_fps(fpsCTRLvar.mode,
                                   fpsCTRLvar.fpsnamemask,
                                   data.fpsarray,
                                   keywnode,
                                   &fpsCTRLvar.NBkwn,
                                   &fpsCTRLvar.NBfps,
                                   &NBpindex,
                                   0 // quiet
                                  );



        printf("%d function parameter structure(s) imported, %ld parameters\n",
               fpsCTRLvar.NBfps,
               NBpindex);
        fflush(stdout);
    }



    DEBUG_TRACEPOINT(" ");

    fpsCTRLvar.nodeSelected = 1;

    // default: use ncurses
    TUI_set_screenprintmode(SCREENPRINT_NCURSES);

    if (getenv("MILK_TUIPRINT_STDIO"))
    {
        // use stdio instead of ncurses
        TUI_set_screenprintmode(SCREENPRINT_STDIO);
    }

    if (getenv("MILK_TUIPRINT_NONE"))
    {
        TUI_set_screenprintmode(SCREENPRINT_NONE);
    }

    // INITIALIZE terminal

    if (run_display == 1)
    {
        TUI_init_terminal(&wrow, &wcol);
        DEBUG_TRACEPOINT("returned from TUI init %d %d", wrow, wcol);
    }

    fpsCTRLvar.NBindex = 0;
    char shmdname[STRINGMAXLEN_SHMDIRNAME];
    function_parameter_struct_shmdirname(shmdname);

    if (run_display == 0)
    {
        loopOK = 0;
    }

    // how long between getchar probes
    int getchardt_us_ref = 100000;

    // refresh every 1 sec without input
    int refreshtimeoutus_ref = 1000000;

    int getchardt_us     = getchardt_us_ref;
    int refreshtimeoutus = refreshtimeoutus_ref;

    if (TUI_get_screenprintmode() == SCREENPRINT_NCURSES) // ncurses mode
    {
        refreshtimeoutus_ref = 100000; // 10 Hz
    }

    int refresh_screen = 1; // 1 if screen should be refreshed



    while (loopOK == 1)
    {
        int NBtaskLaunched = 0;

        //long icnt = 0;
        int ch = -1;

        int timeoutuscnt = 0;

        while (refresh_screen == 0) // wait for input
        {
            // put input commands from fifo into the task queue
            int fcnt =
                functionparameter_read_fpsCMD_fifo(fpsCTRLvar.fpsCTRLfifofd,
                                                   fpsctrltasklist,
                                                   fpsctrlqueuelist);

            DEBUG_TRACEPOINT(" ");

            // execute next command in the queue
            int taskflag =
                function_parameter_process_fpsCMDarray(fpsctrltasklist,
                        fpsctrlqueuelist,
                        keywnode,
                        &fpsCTRLvar,
                        data.fpsarray);

            if (taskflag > 0) // task has been performed
            {
                getchardt_us = 1000; // check often
            }
            else
            {
                // gradually slow down
                getchardt_us = (int) (1.01 * getchardt_us);

                if (getchardt_us > getchardt_us_ref)
                {
                    getchardt_us = getchardt_us_ref;
                }
            }
            NBtaskLaunched += taskflag;

            NBtaskLaunchedcnt += NBtaskLaunched;

            fifocmdcnt += fcnt;

            usleep(getchardt_us);

            // ==================
            // = GET USER INPUT =
            // ==================
            ch = get_singlechar_nonblock();

            if (ch == -1)
            {
                refresh_screen = 0;
            }
            else
            {
                refresh_screen = 2;
                getchardt_us = 10000; // check often
            }

            timeoutuscnt += getchardt_us;
            if (timeoutuscnt > refreshtimeoutus)
            {
                refresh_screen = 1;
            }

            DEBUG_TRACEPOINT(" ");
        }

        if (refresh_screen > 0)
        {
            refresh_screen--; // will wait next time we enter the loop
        }

        TUI_clearscreen(&wrow, &wcol);

        DEBUG_TRACEPOINT(" ");

        loopOK = fpsCTRL_TUI_process_user_key(ch,
                                              data.fpsarray,
                                              keywnode,
                                              fpsctrltasklist,
                                              fpsctrlqueuelist,
                                              &fpsCTRLvar);

        DEBUG_TRACEPOINT(" ");

        if (fpsCTRLvar.exitloop == 1)
        {
            loopOK = 0;
        }

        if (fpsCTRLvar.run_display == 1)
        {

            TUI_ncurses_erase();

            fpsCTRLscreen_print_DisplayMode_status(
                fpsCTRLvar.fpsCTRL_DisplayMode,
                fpsCTRLvar.NBfps);

            DEBUG_TRACEPOINT(" ");

            if (fpsCTRLvar.fpsCTRL_DisplayVerbose == 1)
            {
                TUI_printfw(
                    "======== FPSCTRL info  ( screen refresh cnt %7ld  "
                    "scan interval %7ld us)",
                    loopcnt,
                    getchardt_us);
                TUI_newline();
                TUI_printfw(
                    "    INPUT FIFO       :  %s (fd=%d)    fifocmdcnt = "
                    "%ld   NBtaskLaunched = %d -> %d   [NB FPS = %d]",
                    fpsCTRLvar.fpsCTRLfifoname,
                    fpsCTRLvar.fpsCTRLfifofd,
                    fifocmdcnt,
                    NBtaskLaunched,
                    NBtaskLaunchedcnt,
                    fpsCTRLvar.NBfps);
                TUI_newline();

                DEBUG_TRACEPOINT(" ");
                char logfname[STRINGMAXLEN_FULLFILENAME];
                getFPSlogfname(logfname);
                TUI_printfw("    OUTPUT LOG       :  %s", logfname);
                TUI_newline();
            }
            DEBUG_TRACEPOINT(" ");




            if (fpsCTRLvar.fpsCTRL_DisplayMode == 1) // help
            {
                fpsCTRLscreen_print_help();
            }

            if (fpsCTRLvar.fpsCTRL_DisplayMode == 2) // display FPS content
            {
                fpsCTRL_FPSdisplay(keywnode, &fpsCTRLvar);
            }

            DEBUG_TRACEPOINT(" ");

            if (fpsCTRLvar.fpsCTRL_DisplayMode == 3) // Task scheduler status
            {
                fpsCTRL_scheduler_display(fpsctrltasklist,
                                          fpsctrlqueuelist,
                                          wrow,
                                          &fpsCTRLvar.scheduler_wrowstart);
            }

            DEBUG_TRACEPOINT(" ");

            TUI_ncurses_refresh();

            DEBUG_TRACEPOINT(" ");

        } // end run_display

        DEBUG_TRACEPOINT("exit from if( fpsCTRLvar.run_display == 1)");

        fpsCTRLvar.run_display = run_display;

        loopcnt++;

        if ((data.signal_TERM == 1) || (data.signal_INT == 1) ||
                (data.signal_ABRT == 1) || (data.signal_BUS == 1) ||
                (data.signal_SEGV == 1) || (data.signal_HUP == 1) ||
                (data.signal_PIPE == 1))
        {
            printf("Exit condition met\n");
            loopOK = 0;
        }
    }




    if (run_display == 1)
    {
        TUI_exit();
    }


    functionparameter_outlog("FPSCTRL", "STOP");


    DEBUG_TRACEPOINT("Disconnect from FPS entries");
    for (int fpsindex = 0; fpsindex < fpsCTRLvar.NBfps; fpsindex++)
    {
        function_parameter_struct_disconnect(&data.fpsarray[fpsindex]);
    }


    free(keywnode);

    free(fpsctrltasklist);
    free(fpsctrlqueuelist);
    functionparameter_outlog("LOGFILECLOSE", "close log file");

    DEBUG_TRACE_FEXIT();

    return RETURN_SUCCESS;
}
