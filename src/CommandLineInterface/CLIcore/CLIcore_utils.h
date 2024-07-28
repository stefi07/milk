/**
 * @file    CLIcore_utils.h
 * @brief   Util functions and macros for coding convenience
 *
 */

#ifndef CLICORE_UTILS_H
#define CLICORE_UTILS_H

#ifdef __cplusplus
typedef const char *CONST_WORD;
#else
typedef const char *__restrict CONST_WORD;
#endif

#include <string.h>

#include "CommandLineInterface/CLIcore.h"

#include "COREMOD_memory/COREMOD_memory.h"
#include "CommandLineInterface/IMGID.h"

#define CLIARG_VISIBLE_DEFAULT                                                 \
    CLICMDARG_FLAG_DEFAULT, FPTYPE_AUTO, FPFLAG_DEFAULT_INPUT
#define CLIARG_HIDDEN_DEFAULT                                                  \
    CLICMDARG_FLAG_NOCLI, FPTYPE_AUTO, FPFLAG_DEFAULT_INPUT
#define CLIARG_OUTPUT_DEFAULT                                                  \
    CLICMDARG_FLAG_NOCLI, FPTYPE_AUTO, FPFLAG_DEFAULT_OUTPUT

#define CLICMD_FIELDS_FPSPROC                                                  \
    __FILE__, sizeof(farg) / sizeof(CLICMDARGDEF), farg,                       \
        CLICMDFLAG_FPS | CLICMDFLAG_PROCINFO, NULL, NULL, NULL
#define CLICMD_FIELDS_DEFAULTS                                                 \
    __FILE__, sizeof(farg) / sizeof(CLICMDARGDEF), farg, CLICMDFLAG_FPS, NULL, \
        NULL, NULL
#define CLICMD_FIELDS_NOFPS                                                    \
    __FILE__, sizeof(farg) / sizeof(CLICMDARGDEF), farg, 0, NULL, NULL, NULL

// return codes for function CLI_checkarg_array
#define RETURN_CLICHECKARGARRAY_SUCCESS      0
#define RETURN_CLICHECKARGARRAY_FAILURE      1
#define RETURN_CLICHECKARGARRAY_FUNCPARAMSET 2
#define RETURN_CLICHECKARGARRAY_HELP         3

#define HELPDETAILSSTRINGSTART "------- DETAILS ------"

#define HELPDETAILSSTRINGEND   "-------- END ---------"


typedef struct
{
    char *name;
} LOCVAR_INIMG;

#define FARG_INPUTIM(imkey)                                                    \
    {                                                                          \
        CLIARG_STR, "." #imkey ".name", "input image", #imkey,                 \
            CLIARG_VISIBLE_DEFAULT, (void **) &imkey.name                      \
    }






typedef struct
{
    char     *name;
    uint32_t *xsize;
    uint32_t *ysize;
    uint32_t *datatype;
    uint32_t  *shared;
    uint32_t  *NBkw;
    uint32_t  *CBsize;
} LOCVAR_OUTIMG2D;


#define FARG_OUTIM_NAME(imkey)     \
    {CLIARG_STR,                   \
     "." #imkey ".name",           \
     "output image",               \
     #imkey,                       \
     CLIARG_VISIBLE_DEFAULT,       \
     (void **) &imkey.name,        \
     NULL}

#define FARG_OUTIM_XSIZE(imkey)    \
        {CLIARG_UINT32,            \
         "." #imkey ".xsize",      \
         "x size",                 \
         "256",                    \
         CLIARG_VISIBLE_DEFAULT,   \
         (void **) &imkey.xsize,   \
         NULL}

#define FARG_OUTIM_YSIZE(imkey)    \
        {CLIARG_UINT32,            \
         "." #imkey ".ysize",      \
         "y size",                 \
         "256",                    \
         CLIARG_VISIBLE_DEFAULT,   \
         (void **) &imkey.ysize,   \
         NULL}


#define FARG_OUTIM_SHARED(imkey)   \
        {CLIARG_UINT32,            \
         "." #imkey ".shared",     \
         "shared flag",            \
         "0",                      \
         CLIARG_HIDDEN_DEFAULT,    \
         (void **) &imkey.shared,  \
         NULL}


#define FARG_OUTIM_NBKW(imkey)     \
        {CLIARG_UINT32,            \
         "." #imkey ".NBkw",       \
         "number keywords",        \
         "10",                     \
         CLIARG_HIDDEN_DEFAULT,    \
         (void **) &imkey.NBkw,    \
         NULL}


#define FARG_OUTIM_CBSIZE(imkey)   \
        {CLIARG_UINT32,            \
         "." #imkey ".CBsize",     \
         "circ buffer size",       \
         "0",                      \
         CLIARG_HIDDEN_DEFAULT,    \
         (void **) &imkey.CBsize,  \
         NULL}




/** @brief Template for ouput image argument to CLI function
 *
 */
#define FARG_OUTIM2D(imkey)     \
    FARG_OUTIM_NAME(imkey),     \
    FARG_OUTIM_XSIZE(imkey),    \
    FARG_OUTIM_YSIZE(imkey),    \
    FARG_OUTIM_SHARED(imkey),   \
    FARG_OUTIM_NBKW(imkey),     \
    FARG_OUTIM_CBSIZE(imkey)



// connect to and/or create output 2D image/stream
//
#define FARG_OUTIM2DCREATE(imkey, img, data_type)                              \
        IMGID img = mkIMGID_from_name(imkey.name);                             \
        img.shared = *imkey.shared;                                            \
        img.NBkw   = *imkey.NBkw;                                              \
        img.CBsize = *imkey.CBsize;                                            \
        if(*imkey.shared == 1) {                                               \
          img = stream_connect_create_2D(imkey.name, *imkey.xsize, *imkey.ysize, data_type); \
        } else {                                                               \
          img.naxis = 2;                                                       \
          img.size[0] = *imkey.xsize;                                          \
          img.size[1] = *imkey.ysize;                                          \
          img.datatype = data_type;                                            \
          createimagefromIMGID(&img);                                          \
        }                                                                      \
        imcreateIMGID(&img);






// binding between variables and function args/params
#define STD_FARG_LINKfunction                                                  \
    for (int argi = 0; argi < (int) (sizeof(farg) / sizeof(CLICMDARGDEF));     \
         argi++)                                                               \
    {                                                                          \
        long  fpsi           = -1;                                             \
        void *ptr            = get_farg_ptr(farg[argi].fpstag, &fpsi);         \
        *(farg[argi].valptr) = ptr;                                            \
        if (farg[argi].indexptr != NULL)                                       \
        {                                                                      \
            *(farg[argi].indexptr) = fpsi;                                     \
        }                                                                      \
    }













/** @brief Standard Function call wrapper
 *
 * CLI argument(s) is(are) parsed and checked with CLI_checkarray(), then
 * passed to the compute function call.
 *
 * Custom code may be added for more complex processing of function arguments.
 *
 * If CLI call arguments check out, go ahead with computation.
 * Arguments not contained in CLI call line are extracted from the
 * command argument list
 */
#define INSERT_STD_CLIfunction                                                 \
    static errno_t CLIfunction(void)                                           \
    {                                                                          \
        errno_t retval = CLI_checkarg_array(farg, CLIcmddata.nbarg);           \
        if (retval == RETURN_SUCCESS)                                          \
        {                                                                      \
            STD_FARG_LINKfunction return compute_function();                   \
        }                                                                      \
        if (retval == RETURN_CLICHECKARGARRAY_HELP)                            \
        {                                                                      \
            return RETURN_SUCCESS;                                             \
        }                                                                      \
        if (retval == RETURN_CLICHECKARGARRAY_FUNCPARAMSET)                    \
        {                                                                      \
            return RETURN_SUCCESS;                                             \
        }                                                                      \
        return retval;                                                         \
    }




/** @brief FPS conf function
 * Sets up the FPS and its parameters.\n
 * Optional parameter checking can be included.\n
 *
 * ### ADD PARAMETERS
 *
 * The function function_parameter_add_entry() is called to add
 * each parameter.
 *
 * Macros are provided for convenience, named "FPS_ADDPARAM_...".\n
 * The macros are defined in fps_add_entry.h, and provide a function
 * parameter identifier variable (int) for each parameter added.
 *
 * parameters for FPS_ADDPARAM macros:
 * - key/variable name
 * - tag name
 * - description
 * - default initial value
 *
 * Equivalent code without using macro :
 *      function_parameter_add_entry(&fps, ".delayus", "Delay [us]", FPTYPE_INT64, FPFLAG_DEFAULT_INPUT|FPFLAG_WRITERUN, NULL);
 * ### START CONFLOOP
 *
 * start function parameter conf loop\n
 * macro defined in function_parameter.h
 *
 * Optional code to handle/check parameters is included after this
 * statement
 *
 * ### STOP CONFLOOP
 * stop function parameter conf loop\n
 * macro defined in function_parameter.h
 *
 */

#define INSERT_STD_FPSCONFfunction                                             \
    static errno_t FPSCONFfunction()                                           \
    {                                                                          \
        FPS_SETUP_INIT(data.FPS_name, data.FPS_CMDCODE);                       \
        if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO)               \
        {                                                                      \
            fps.cmdset.flags       = CLIcmddata.cmdsettings->flags;            \
            fps.cmdset.RT_priority = CLIcmddata.cmdsettings->RT_priority;      \
            fps.cmdset.procinfo_loopcntMax =                                   \
                CLIcmddata.cmdsettings->procinfo_loopcntMax;                   \
            fps.cmdset.triggermode = CLIcmddata.cmdsettings->triggermode;      \
            strncpy(fps.cmdset.triggerstreamname,                              \
                   CLIcmddata.cmdsettings->triggerstreamname, STRINGMAXLEN_IMAGE_NAME-1); \
            fps.cmdset.semindexrequested =                                     \
                CLIcmddata.cmdsettings->semindexrequested;                     \
            fps.cmdset.triggerdelay.tv_sec =                                   \
                CLIcmddata.cmdsettings->triggerdelay.tv_sec;                   \
            fps.cmdset.triggerdelay.tv_nsec =                                  \
                CLIcmddata.cmdsettings->triggerdelay.tv_nsec;                  \
            fps.cmdset.triggertimeout.tv_sec =                                 \
                CLIcmddata.cmdsettings->triggertimeout.tv_sec;                 \
            fps.cmdset.triggertimeout.tv_nsec =                                \
                CLIcmddata.cmdsettings->triggertimeout.tv_nsec;                \
            fps_add_processinfo_entries(&fps);                                 \
        }                                                                      \
        data.fpsptr = &fps;                                                    \
        strncpy(data.fpsptr->md->description, CLIcmddata.description, FPS_DESCR_STRMAXLEN-1);\
        CMDargs_to_FPSparams_create(&fps);                                     \
        STD_FARG_LINKfunction if (CLIcmddata.FPS_customCONFsetup != NULL)      \
        {                                                                      \
            CLIcmddata.FPS_customCONFsetup();                                  \
        }                                                                      \
        FPS_CONFLOOP_START                                                     \
        if (CLIcmddata.FPS_customCONFcheck != NULL)                            \
            CLIcmddata.FPS_customCONFcheck();                                  \
        FPS_CONFLOOP_END                                                       \
        data.fpsptr = NULL;                                                    \
        return RETURN_SUCCESS;                                                 \
    }




#define INSERT_STD_PROCINFO_COMPUTEFUNC_INIT                                   \
    int          processloopOK = 1;                                            \
    PROCESSINFO *processinfo   = NULL;                                         \
    /* set default timeout to 2 sec */                                         \
    CLIcmddata.cmdsettings->triggertimeout.tv_sec  = 0;                        \
    CLIcmddata.cmdsettings->triggertimeout.tv_nsec = 1000000;                  \
    if (data.fpsptr != NULL)                                                   \
    { /* If FPS mode, then FPS settings override defaults*/                    \
        /* data.fpsptr->cmset entries are read by fps_connect */               \
        /*CLIcmddata.cmdsettings->flags = data.fpsptr->cmdset.flags;*/         \
        CLIcmddata.cmdsettings->RT_priority = data.fpsptr->cmdset.RT_priority; \
        CLIcmddata.cmdsettings->procinfo_loopcntMax =                          \
            data.fpsptr->cmdset.procinfo_loopcntMax;                           \
        CLIcmddata.cmdsettings->triggermode = data.fpsptr->cmdset.triggermode; \
        strncpy(CLIcmddata.cmdsettings->triggerstreamname,                     \
               data.fpsptr->cmdset.triggerstreamname, STRINGMAXLEN_IMAGE_NAME-1);   \
        CLIcmddata.cmdsettings->semindexrequested =                            \
            data.fpsptr->cmdset.semindexrequested;                             \
        CLIcmddata.cmdsettings->triggerdelay.tv_sec =                          \
            data.fpsptr->cmdset.triggerdelay.tv_sec;                           \
        CLIcmddata.cmdsettings->triggerdelay.tv_nsec =                         \
            data.fpsptr->cmdset.triggerdelay.tv_nsec;                          \
        CLIcmddata.cmdsettings->triggertimeout.tv_sec =                        \
            data.fpsptr->cmdset.triggertimeout.tv_sec;                         \
        CLIcmddata.cmdsettings->triggertimeout.tv_nsec =                       \
            data.fpsptr->cmdset.triggertimeout.tv_nsec;                        \
    }                                                                          \
    if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO)                   \
    {                                                                          \
        char pinfodescr[200];                                                  \
        int  slen =                                                            \
            snprintf(pinfodescr, 200, "function %.10s", CLIcmddata.key);       \
        if (slen < 1)                                                          \
        {                                                                      \
            PRINT_ERROR("snprintf wrote <1 char");                             \
            abort();                                                           \
        }                                                                      \
        if (slen >= 200)                                                       \
        {                                                                      \
            PRINT_ERROR("snprintf string truncation");                         \
            abort();                                                           \
        }                                                                      \
        if (data.fpsptr != NULL)                                               \
        {                                                                      \
            processinfo = processinfo_setup(data.FPS_name,                     \
                                            pinfodescr,                        \
                                            "startup",                         \
                                            __FUNCTION__,                      \
                                            __FILE__,                          \
                                            __LINE__);                         \
            fps_to_processinfo(data.fpsptr, processinfo);                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            processinfo = processinfo_setup(CLIcmddata.key,                    \
                                            pinfodescr,                        \
                                            "startup",                         \
                                            __FUNCTION__,                      \
                                            __FILE__,                          \
                                            __LINE__);                         \
        }                                                                      \
        DEBUG_TRACEPOINT("setting processinfo parameters");                    \
        processinfo->loopcntMax = CLIcmddata.cmdsettings->procinfo_loopcntMax; \
        processinfo->triggerstreamID = -2;                                     \
        processinfo->triggermode     = CLIcmddata.cmdsettings->triggermode;    \
        strncpy(processinfo->triggerstreamname,                                \
               CLIcmddata.cmdsettings->triggerstreamname, STRINGMAXLEN_IMAGE_NAME-1);  \
        processinfo->triggerdelay   = CLIcmddata.cmdsettings->triggerdelay;    \
        processinfo->triggertimeout = CLIcmddata.cmdsettings->triggertimeout;  \
        processinfo->triggerstreamID =                                         \
            image_ID(processinfo->triggerstreamname);                          \
        DEBUG_TRACEPOINT("triggerstreamID = %ld",                              \
                         processinfo->triggerstreamID);                        \
        FUNC_CHECK_RETURN(processinfo_waitoninputstream_init(                  \
            processinfo,                                                       \
            processinfo->triggerstreamID,                                      \
            CLIcmddata.cmdsettings->triggermode,                               \
            CLIcmddata.cmdsettings->semindexrequested));                       \
        DEBUG_TRACEPOINT("setting RT priority to %d",                          \
                         CLIcmddata.cmdsettings->RT_priority);                 \
        processinfo->RT_priority = CLIcmddata.cmdsettings->RT_priority;        \
        processinfo->CPUmask     = CLIcmddata.cmdsettings->CPUmask;            \
        processinfo->MeasureTiming =                                           \
            CLIcmddata.cmdsettings->procinfo_MeasureTiming;                    \
        DEBUG_TRACEPOINT("loopstart");                                         \
        processinfo_loopstart(processinfo);                                    \
    }



#define INSERT_STD_PROCINFO_COMPUTEFUNC_LOOPSTART                              \
    while (processloopOK == 1)                                                 \
    {                                                                          \
        if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO)               \
        {                                                                      \
            DEBUG_TRACEPOINT("loopstep");                                      \
            processloopOK = processinfo_loopstep(processinfo);                 \
            DEBUG_TRACEPOINT("waitoninputstream");                             \
            processinfo_waitoninputstream(processinfo);                        \
            if (processinfo->triggerstatus != PROCESSINFO_TRIGGERSTATUS_RECEIVED)   \
            {                                                                  \
                /* Don't execute loop at all upon semaphore timeout */         \
                continue;                                                      \
            }                                                                  \
            DEBUG_TRACEPOINT("exec_start");                                    \
            processinfo_exec_start(processinfo);                               \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            processloopOK = 0;                                                 \
        }                                                                      \
        int processcompstatus = 1;                                             \
        if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO)               \
        {                                                                      \
            processcompstatus = processinfo_compute_status(processinfo);       \
        }                                                                      \
        if (processcompstatus == 1)                                            \
        {



#define INSERT_STD_PROCINFO_COMPUTEFUNC_START                                  \
    INSERT_STD_PROCINFO_COMPUTEFUNC_INIT                                       \
    INSERT_STD_PROCINFO_COMPUTEFUNC_LOOPSTART



#define INSERT_STD_PROCINFO_COMPUTEFUNC_END                                    \
    }                                                                          \
    if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO) {                 \
     if(processinfo != NULL) {                                                 \
      if(data.fpsptr != NULL) {                                                \
        if(data.fpsptr->cmdset.triggermodeptr != NULL){                        \
          processinfo->triggermode = *data.fpsptr->cmdset.triggermodeptr;}     \
        if(data.fpsptr->cmdset.procinfo_loopcntMax_ptr != NULL){               \
          processinfo->loopcntMax = *data.fpsptr->cmdset.procinfo_loopcntMax_ptr;}  \
        if(data.fpsptr->cmdset.triggerdelayptr != NULL){                       \
          processinfo->triggerdelay = data.fpsptr->cmdset.triggerdelayptr[0];} \
        if(data.fpsptr->cmdset.triggertimeoutptr != NULL){                     \
          processinfo->triggertimeout = data.fpsptr->cmdset.triggertimeoutptr[0];} \
        }}                                                                     \
    if(processinfo != NULL) {                                              \
          processinfo_exec_end(processinfo);}                                  \
    }                                                                          \
    }                                                                          \
    if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO)                   \
    {                                                                          \
        processinfo_cleanExit(processinfo);                                    \
    }



/**
 *
        if(data.fpsptr->cmdset.triggerdelayptr != NULL) {                      \
        processinfo->triggerdelay = data.fpsptr->cmdset.triggerdelayptr[0];}   \
        if(data.fpsptr->cmdset.triggertimeoutptr != NULL) {                    \
        processinfo->triggerdelay = data.fpsptr->cmdset.triggertimeoutptr[0];} \
 *
 */

/** @brief FPS run function
 *
 * The FPS name is taken from data.FPS_name, which has to
 * have been set up by either the stand-alone function, or the
 * CLI.
 *
 * Running FPS_CONNECT macro in FPSCONNECT_RUN mode.
 *
 * ### GET FUNCTION PARAMETER VALUES
 *
 * Parameters are addressed by their tag name\n
 * These parameters are read once, before running the loop.\n
 *
 * FPS_GETPARAM... macros are wrapper to functionparameter_GetParamValue
 * and functionparameter_GetParamPtr functions, all defined in
 * fps_paramvalue.h.
 *
 * Each of the FPS_GETPARAM macro creates a variable with "_" prepended
 * to the first macro argument.
 *
 * Equivalent code without using macros:
 *
 * char _IDin_name[200];
 * strncpy(_IDin_name,  functionparameter_GetParamPtr_STRING(&fps, ".in_name"), FUNCTION_PARAMETER_STRMAXLEN);
 * long _delayus = functionparameter_GetParamValue_INT64(&fps, ".delayus");
 */
#define INSERT_STD_FPSRUNfunction                                              \
    static errno_t FPSRUNfunction()                                            \
    {                                                                          \
        FPS_CONNECT(data.FPS_name, FPSCONNECT_RUN);                            \
        data.fpsptr                        = &fps;                             \
        STD_FARG_LINKfunction errno_t fret = compute_function();               \
        data.fpsptr                        = NULL;                             \
        function_parameter_RUNexit(&fps);                                      \
        return fret;                                                           \
    }

/** @brief FPSCLI function
 *
 * GET ARGUMENTS AND PARAMETERS
 * Try FPS implementation
 *
 * Set data.fpsname, providing default value as first arg, and set data.FPS_CMDCODE value.
 * Default FPS name will be used if CLI process has NOT been named.
 * See code in function_parameter.h for detailed rules.
 */
#define INSERT_STD_FPSCLIfunction                                              \
    static errno_t CLIfunction(void)                                           \
    {                                                                          \
        if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_FPS)                    \
        {                                                                      \
            function_parameter_getFPSargs_from_CLIfunc(CLIcmddata.key);        \
            if (data.FPS_CMDCODE != 0)                                         \
            {                                                                  \
                data.FPS_CONFfunc = FPSCONFfunction;                           \
                data.FPS_RUNfunc  = FPSRUNfunction;                            \
                function_parameter_execFPScmd();                               \
                return RETURN_SUCCESS;                                         \
            }                                                                  \
        }                                                                      \
                                                                               \
        errno_t retval = CLI_checkarg_array(farg, CLIcmddata.nbarg);           \
        if (retval == RETURN_CLICHECKARGARRAY_SUCCESS)                         \
        {                                                                      \
            data.fpsptr = NULL;                                                \
            STD_FARG_LINKfunction return compute_function();                   \
        }                                                                      \
        if (retval == RETURN_CLICHECKARGARRAY_HELP)                            \
        {                                                                      \
            printf(HELPDETAILSSTRINGSTART "\n");                               \
            help_function();                                                   \
            printf(HELPDETAILSSTRINGEND "\n");                                 \
            return RETURN_SUCCESS;                                             \
        }                                                                      \
        if (retval == RETURN_CLICHECKARGARRAY_FUNCPARAMSET)                    \
        {                                                                      \
            return RETURN_SUCCESS;                                             \
        }                                                                      \
                                                                               \
        return retval;                                                         \
    }

#define INSERT_STD_FPSCLIfunctions                                             \
    INSERT_STD_FPSCONFfunction                                                 \
    INSERT_STD_FPSRUNfunction                                                  \
        INSERT_STD_FPSCLIfunction

#define INSERT_STD_CLIREGISTERFUNC                                             \
    {                                                                          \
        if (getenv("MILK_FPSPROCINFO"))                                        \
        {                                                                      \
            CLIcmddata.flags |= CLICMDFLAG_PROCINFO;                           \
        }                                                                      \
        int cmdi               = RegisterCLIcmd(CLIcmddata, CLIfunction);      \
        CLIcmddata.cmdsettings = &data.cmd[cmdi].cmdsettings;                  \
    }

/** make IMGID from name
 *
 * Some settings can be embedded in the image name string for convenience :
 *
 * Examples:
 * "im1" no optional setting, image name = im1
 * "s>im1" : set shared memory flag
 * "k10>im1" : number of keyword = 10
 * "c20>im1" : 20-sized circular buffer
 * "tf64>im1" : datatype is double (64 bit floating point)
*/
static inline IMGID mkIMGID_from_name(CONST_WORD name)
{
    IMGID img = {0};

    // default values for image creation
    img.datatype = _DATATYPE_FLOAT;
    img.naxis    = 2;
    img.size[0]  = 1;
    img.size[1]  = 1;
    img.shared   = 0;
    img.NBkw     = NB_KEYWNODE_MAX;
    img.CBsize   = 0;

    char *pch;
    char *pch1;
    int   nbword = 0;

    char  namestring[200];
    strncpy(namestring, name, 199);

    pch1 = namestring;
    if(strlen(namestring) != 0)
    {
        pch = strtok(namestring, ">");
        while(pch != NULL)
        {
            pch1 = pch;
            //printf("[%2d] %s\n", nbword, pch);

            if(strcmp(pch, "s") == 0)
            {
                printf("    shared memory\n");
                img.shared = 1;
            }

            if(strcmp(pch, "tui8") == 0)
            {
                printf("    data type unsigned 8-bit int\n");
                img.datatype = _DATATYPE_UINT8;
            }
            if(strcmp(pch, "tsi8") == 0)
            {
                printf("    data type signed 8-bit int\n");
                img.datatype = _DATATYPE_INT8;
            }
            if(strcmp(pch, "tui16") == 0)
            {
                printf("    data type unsigned 16-bit int\n");
                img.datatype = _DATATYPE_UINT16;
            }
            if(strcmp(pch, "tsi16") == 0)
            {
                printf("    data type signed 16-bit int\n");
                img.datatype = _DATATYPE_INT16;
            }
            if(strcmp(pch, "tui32") == 0)
            {
                printf("    data type unsigned 32-bit int\n");
                img.datatype = _DATATYPE_UINT32;
            }
            if(strcmp(pch, "tsi32") == 0)
            {
                printf("    data type signed 32-bit int\n");
                img.datatype = _DATATYPE_INT32;
            }
            if(strcmp(pch, "tui64") == 0)
            {
                printf("    data type unsigned 64-bit int\n");
                img.datatype = _DATATYPE_UINT64;
            }
            if(strcmp(pch, "tsi64") == 0)
            {
                printf("    data type signed 64-bit int\n");
                img.datatype = _DATATYPE_INT64;
            }

            if(strcmp(pch, "tf32") == 0)
            {
                printf("    data type double (32)\n");
                img.datatype = _DATATYPE_FLOAT;
            }
            if(strcmp(pch, "tf64") == 0)
            {
                printf("    data type float (64)\n");
                img.datatype = _DATATYPE_DOUBLE;
            }

            /*            if(pch[0] == 'k')
                        {
                            int nbkw;
                            sscanf(pch, "k%d", &nbkw);
                            printf("    %d keywords\n", nbkw);
                            img.NBkw = nbkw;
                        }

                        if(pch[0] == 'c')
                        {
                            int cbsize;
                            sscanf(pch, "c%d", &cbsize);
                            printf("    %d circular buffer size\n", cbsize);
                            img.CBsize = cbsize;
                        }
            */
            pch = strtok(NULL, ">");
            nbword++;
        }
    }

    img.ID        = -1;
    img.createcnt = -1;
    strncpy(img.name, pch1, STRINGMAXLEN_IMAGE_NAME - 1);
    img.im = NULL;
    img.md = NULL;

    return img;
}



/** make blank IMGID from name
 *
 * All fields are uninitialized
 * Can be used for comparison
*/
static inline IMGID makeIMGID_blank()
{
    IMGID img;

    // default values for image creation
    img.datatype = _DATATYPE_UNINITIALIZED;
    img.naxis    = -1;
    img.size[0]  = 0;
    img.size[1]  = 0;
    img.size[2]  = 0;
    img.shared   = -1;
    img.NBkw     = -1;
    img.CBsize   = -1;

    img.ID        = -1;
    img.createcnt = -1;
    strncpy(img.name, "", STRINGMAXLEN_IMAGE_NAME - 1);
    img.im = NULL;
    img.md = NULL;

    return img;
}




static inline IMGID makeIMGID_2D(
    CONST_WORD name,
    uint32_t xsize,
    uint32_t ysize
)
{
    IMGID img   = mkIMGID_from_name(name);
    img.naxis   = 2;
    img.size[0] = xsize;
    img.size[1] = ysize;

    return img;
}

static inline IMGID makeIMGID_3D(
    CONST_WORD name,
    uint32_t xsize,
    uint32_t ysize,
    uint32_t zsize
)
{
    IMGID img   = mkIMGID_from_name(name);
    img.naxis   = 3;
    img.size[0] = xsize;
    img.size[1] = ysize;
    img.size[2] = zsize;

    return img;
}




static inline int copyIMGID(
    IMGID *imgin,
    IMGID *imgout
)
{
    imgout->datatype = imgin->datatype;
    imgout->shared   = imgin->shared;

    imgout->naxis = imgin->naxis;

    imgout->size[0] = imgin->size[0];
    imgout->size[1] = imgin->size[1];
    imgout->size[2] = imgin->size[2];

    imgout->NBkw   = imgin->NBkw;
    imgout->CBsize = imgin->CBsize;

    return RETURN_SUCCESS;
}



static inline imageID createimagefromIMGID(IMGID *img)
{
    create_image_ID(img->name,
                    img->naxis,
                    img->size,
                    img->datatype,
                    img->shared,
                    img->NBkw,
                    img->CBsize,
                    &img->ID);

    img->im        = &data.image[img->ID];
    img->md        = &data.image[img->ID].md[0];
    img->createcnt = data.image[img->ID].createcnt;

    return img->ID;
}




/** Create image according to IMGID entries of existing image
 */
static inline imageID imcreatelikewiseIMGID(
    IMGID *target_img,
    IMGID *source_img
)
{
    if(target_img->ID == -1)
    {
        if(target_img != source_img)
        {
            printf("Creating image %s from %s, shared = %d, kw = %d\n",
                   target_img->name,
                   source_img->name,
                   source_img->shared,
                   source_img->NBkw);
        }
        else
        {
            printf("Creating image %s, shared = %d, kw = %d\n",
                   source_img->name,
                   source_img->shared,
                   source_img->NBkw);
        }

        DEBUG_TRACEPOINT("Creating 2D image");
        create_image_ID(target_img->name,
                        source_img->naxis,
                        source_img->size,
                        source_img->datatype,
                        source_img->shared,
                        source_img->NBkw,
                        source_img->CBsize,
                        &target_img->ID);
        DEBUG_TRACEPOINT(" ");
        target_img->im        = &data.image[target_img->ID];
        target_img->md        = &data.image[target_img->ID].md[0];
        target_img->createcnt = data.image[target_img->ID].createcnt;


        target_img->size[0] = source_img->size[0];
        if(source_img->naxis > 1)
        {
            target_img->size[1] = source_img->size[1];
        }
        if(source_img->naxis > 2)
        {
            target_img->size[2] = source_img->size[2];
        }
    }
    return target_img->ID;
}




/** Create image according to IMGID entries
 *  See cloning creation function imcreatelikewiseIMGID()
 */
static inline imageID imcreateIMGID(IMGID *img)
{
    return imcreatelikewiseIMGID(img, img);
}




static inline errno_t updateIMGIDcreationparams(IMGID *img)
{
    img->datatype = img->md->datatype;
    img->naxis    = img->md->naxis;
    for(int ii = 0; ii < 3; ++ii)
    {
        img->size[ii] = img->md->size[ii];
    }
    img->shared = img->md->shared;
    img->NBkw   = img->md->NBkw;
    img->CBsize = img->md->CBsize;

    return RETURN_SUCCESS;
}

static inline IMGID makesetIMGID(CONST_WORD name, imageID ID)
{
    IMGID img;

    img.ID = ID;
    strncpy(img.name, name, STRINGMAXLEN_IMAGE_NAME - 1);

    img.im        = &data.image[ID];
    img.md        = &data.image[ID].md[0];
    img.createcnt = data.image[ID].createcnt;

    return img;
}




/** @brief Resolve image already in memory
 *
 * ERRMODE values
 * ERRMODE_WARN : print warning
 * ERRMODE_FAIL : error
 * ERRMODE_ABORT : abort
 */
static inline imageID resolveIMGID(
    IMGID *img,
    int ERRMODE
)
{
    // IF:
    // Not resolved before OR create counter mismatch OR not used
    if((img->ID == -1)
            || (img->createcnt != data.image[img->ID].createcnt)
            || (data.image[img->ID].used != 1))
    {
        img->ID = image_ID(img->name);
        if(img->ID > -1)  // Resolve success !
        {
            img->im        = &data.image[img->ID];
            img->md        = &data.image[img->ID].md[0];
            img->createcnt = data.image[img->ID].createcnt;

            // Populate the IMGID from the imageID metadata
            updateIMGIDcreationparams(img);
        }
    }

    // if still unresolved
    //
    if(img->ID == -1)
    {
        if((ERRMODE == ERRMODE_FAIL) || (ERRMODE == ERRMODE_ABORT))
        {
            printf("ERROR: %c[%d;%dm Cannot resolve image %s %c[%d;m\n",
                   (char) 27,
                   1,
                   31,
                   img->name,
                   (char) 27,
                   0);
            abort();
        }
        else if(ERRMODE == ERRMODE_WARN)
        {
            printf("WARNING: %c[%d;%dm Cannot resolve image %s %c[%d;m\n",
                   (char) 27,
                   1,
                   35,
                   img->name,
                   (char) 27,
                   0);
        }
    }

    return img->ID;
}



/**
 * @brief Check if img complies to imgtemplate
 *
 */
static inline uint64_t IMGIDcompare(
    IMGID img,
    IMGID imgtemplate
)
{
    int compErr = 0;

    if(imgtemplate.datatype != _DATATYPE_UNINITIALIZED)
    {
        printf("Checking datatype       ");
        if(imgtemplate.datatype != img.datatype)
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.naxis != -1)
    {
        printf("Checking naxis  %d %d    ", imgtemplate.naxis, img.naxis);
        if(imgtemplate.naxis != img.naxis)
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.size[0] != 0)
    {
        printf("Checking size[0]        ");
        if(imgtemplate.size[0] != img.size[0])
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.size[1] != 0)
    {
        printf("Checking size[1]        ");
        if(imgtemplate.size[1] != img.size[1])
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.size[2] != 0)
    {
        printf("Checking size[2]        ");
        if(imgtemplate.size[2] != img.size[2])
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    printf("Checking NBkw           ");
    if(imgtemplate.NBkw != img.NBkw)
    {
        printf("FAIL\n");
        printf("   %4u  %s\n", imgtemplate.NBkw, imgtemplate.name);
        printf("   %4u  %s\n", img.NBkw, img.name);
        compErr++;
    }
    else
    {
        printf("PASS\n");
    }


    return compErr;
}




/**
 * @brief Check if img complies to imgtemplate
 *
 */
static inline uint64_t IMGIDmdcompare(
    IMGID img,
    IMGID imgtemplate
)
{
    int compErr = 0;

    printf("COMPARING %s %s\n", img.name, imgtemplate.name);

    if(imgtemplate.md->datatype != _DATATYPE_UNINITIALIZED)
    {
        printf("Checking md->datatype       ");
        if(imgtemplate.md->datatype != img.md->datatype)
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.md->naxis != 0)
    {
        printf("Checking md->naxis  %d %d    ", imgtemplate.md->naxis, img.md->naxis);
        if(imgtemplate.md->naxis != img.md->naxis)
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.md->size[0] != 0)
    {
        printf("Checking md->size[0]        ");
        if(imgtemplate.md->size[0] != img.md->size[0])
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.md->size[1] != 0)
    {
        printf("Checking md->size[1]        ");
        if(imgtemplate.md->size[1] != img.md->size[1])
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    if(imgtemplate.md->size[2] != 0)
    {
        printf("Checking md->size[2]        ");
        if(imgtemplate.md->size[2] != img.md->size[2])
        {
            printf("FAIL\n");
            compErr++;
        }
        else
        {
            printf("PASS\n");
        }
    }

    printf("Checking NBkw           ");
    if(imgtemplate.md->NBkw != img.md->NBkw)
    {
        printf("FAIL\n");
        printf("   %4u  %s\n", imgtemplate.md->NBkw, imgtemplate.md->name);
        printf("   %4u  %s\n", img.md->NBkw, img.md->name);
        printf("    template : %u\n", imgtemplate.md->NBkw);
        printf("    dest     : %u\n", img.md->NBkw);
        compErr++;
    }
    else
    {
        printf("PASS\n");
    }

    return compErr;
}





/**
 * @brief Connnect to stream
 *
 * @param imname  stream name
 * @return IMGID
 */
static inline IMGID
stream_connect(
    char *__restrict imname
)
{
    IMGID img = mkIMGID_from_name(imname);
    resolveIMGID(&img, ERRMODE_WARN);

    if(img.ID == -1)
    {
        // try to connect to shared memory if not in local memory already
        read_sharedmem_image(imname);
        resolveIMGID(&img, ERRMODE_WARN);
    }

    return img;
}


static inline IMGID stream_connect_create_2D(
    char *__restrict imname,
    uint32_t xsize,
    uint32_t ysize,
    uint8_t  datatype
)
{
    IMGID img = mkIMGID_from_name(imname);
    resolveIMGID(&img, ERRMODE_WARN);


    if(img.ID == -1)
    {
        // try to connect to shared memory if not in local memory already
        read_sharedmem_image(imname);
        resolveIMGID(&img, ERRMODE_WARN);
    }

    if(img.ID != -1)
    {
        // if in local memory,
        // create blank img for comparison
        IMGID imgc      = makeIMGID_blank();
        imgc.datatype   = datatype;
        imgc.naxis      = 2;
        imgc.size[0]    = xsize;
        imgc.size[1]    = ysize;
        imgc.NBkw       = NB_KEYWNODE_MAX;
        uint64_t imgerr = IMGIDcompare(img, imgc);
        printf("%lu errors\n", imgerr);

        // if doesn't pass test, erase from local memory
        if(imgerr != 0)
        {
            delete_image_ID(imname, DELETE_IMAGE_ERRMODE_WARNING);
            img.ID = -1;
        }
    }

    // if not in local memory, (re)-create
    if(img.ID == -1)
    {
        uint32_t arraytmp[2];

        arraytmp[0] = xsize;
        arraytmp[1] = ysize;

        create_image_ID(imname, 2, arraytmp, datatype, 1, NB_KEYWNODE_MAX, 0, &img.ID);
    }


    if(img.ID != -1)
    {
        imageID ID    = img.ID;
        img.im        = &data.image[ID];
        img.md        = data.image[ID].md;
        img.createcnt = data.image[ID].createcnt;
        updateIMGIDcreationparams(&img);
    }

    return img;
}

/**
 * @brief Connnect to stream or create if doesn't exist
 *
 * If stream exists but has wrong size type, recreate
 *
 * @param imname  stream name
 * @param xsize   x size
 * @param ysize   y size
 * @return IMGID
 */
static inline IMGID
stream_connect_create_2Df32(
    char *__restrict imname,
    uint32_t xsize,
    uint32_t ysize
)
{
    return stream_connect_create_2D(imname, xsize, ysize, _DATATYPE_FLOAT);
}

static inline IMGID stream_connect_create_3D(
    char *__restrict imname,
    uint32_t xsize,
    uint32_t ysize,
    uint32_t zsize,
    uint8_t  datatype
)
{
    IMGID img = mkIMGID_from_name(imname);
    resolveIMGID(&img, ERRMODE_WARN);


    if(img.ID == -1)
    {
        // try to connect to shared memory if not in local memory already
        read_sharedmem_image(imname);
        resolveIMGID(&img, ERRMODE_WARN);
    }

    if(img.ID != -1)
    {
        // if in local memory,
        // create blank img for comparison
        IMGID imgc      = makeIMGID_blank();
        imgc.datatype   = datatype;
        imgc.naxis      = 3;
        imgc.size[0]    = xsize;
        imgc.size[1]    = ysize;
        imgc.size[2]    = zsize;
        imgc.NBkw       = NB_KEYWNODE_MAX;
        uint64_t imgerr = IMGIDcompare(img, imgc);
        printf("%lu errors\n", imgerr);

        // if doesn't pass test, erase from local memory
        if(imgerr != 0)
        {
            delete_image_ID(imname, DELETE_IMAGE_ERRMODE_WARNING);
            img.ID = -1;
        }
    }

    // if not in local memory, (re)-create
    if(img.ID == -1)
    {
        uint32_t arraytmp[3];

        arraytmp[0] = xsize;
        arraytmp[1] = ysize;
        arraytmp[2] = zsize;

        printf("CREATING image size %u %u %u\n", xsize, ysize, zsize);

        create_image_ID(imname, 3, arraytmp, datatype, 1, NB_KEYWNODE_MAX, 0, &img.ID);
    }


    if(img.ID != -1)
    {
        imageID ID    = img.ID;
        img.im        = &data.image[ID];
        img.md        = data.image[ID].md;
        img.createcnt = data.image[ID].createcnt;
        updateIMGIDcreationparams(&img);
    }

    return img;
}

/**
 * @brief Connnect to stream or create if doesn't exist
 *
 * If stream exists but has wrong size type, recreate
 *
 * @param imname  stream name
 * @param xsize   x size
 * @param ysize   y size
 * @param zsize   z size
 * @return IMGID
 */
static inline IMGID stream_connect_create_3Df32(
    char *__restrict imname,
    uint32_t xsize,
    uint32_t ysize,
    uint32_t zsize)
{
    return stream_connect_create_3D(imname, xsize, ysize, zsize, _DATATYPE_FLOAT);
}




#endif
