/**
 * @file    image_crop2D.c
 * @brief   crop 2D function
 *
 *
 */
#include "CommandLineInterface/CLIcore.h"


static char *cropinsname;
static long fpi_cropinsname;

static char *outsname;
static long fpi_outsname;


static uint32_t *cropxstart;
static long fpi_cropxstart;

static uint32_t *cropxsize;
static long fpi_cropxsize;


static uint32_t *cropystart;
static long fpi_cropystart;

static uint32_t *cropysize;
static long fpi_cropysize;







static CLICMDARGDEF farg[] =
{
    {
        CLIARG_IMG,
        ".insname",
        "input stream name",
        "inim",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &cropinsname,
        &fpi_cropinsname
    },
    {
        CLIARG_STR,
        ".outsname",
        "output stream name",
        "outim",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &outsname,
        &fpi_outsname
    },
    {
        CLIARG_UINT32,
        ".cropxstart",
        "crop x coord start",
        "30",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &cropxstart,
        &fpi_cropxstart
    },
    {
        CLIARG_UINT32,
        ".cropxsize",
        "crop x coord size",
        "32",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &cropxsize,
        &fpi_cropxsize
    },
    {
        CLIARG_UINT32,
        ".cropystart",
        "crop y coord start",
        "20",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &cropystart,
        &fpi_cropystart
    },
    {
        CLIARG_UINT32,
        ".cropysize",
        "crop y coord size",
        "32",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &cropysize,
        &fpi_cropysize
    }
};



// Optional custom configuration setup.
// Runs once at conf startup
//
static errno_t customCONFsetup()
{
    if(data.fpsptr != NULL)
    {
        data.fpsptr->parray[fpi_cropinsname].fpflag |=
            FPFLAG_STREAM_RUN_REQUIRED | FPFLAG_CHECKSTREAM;
    }

    return RETURN_SUCCESS;
}

// Optional custom configuration checks.
// Runs at every configuration check loop iteration
//
static errno_t customCONFcheck()
{

    if(data.fpsptr != NULL)
    {
    }

    return RETURN_SUCCESS;
}

static CLICMDDATA CLIcmddata =
{
    "crop2D", "crop 2D image", CLICMD_FIELDS_DEFAULTS
};

// detailed help
static errno_t help_function()
{
    return RETURN_SUCCESS;
}







static errno_t compute_function()
{
    DEBUG_TRACE_FSTART();

    // CONNECT TO INPUT STREAM
    IMGID imgin = mkIMGID_from_name(cropinsname);
    resolveIMGID(&imgin, ERRMODE_ABORT);

    // CONNNECT TO OR CREATE OUTPUT STREAM
    IMGID imgout = stream_connect_create_2D(outsname, *cropxsize, *cropysize, imgin.md->datatype);


    INSERT_STD_PROCINFO_COMPUTEFUNC_INIT;



    INSERT_STD_PROCINFO_COMPUTEFUNC_LOOPSTART
    {

        for(uint32_t jj = 0; jj < *cropysize; jj++)
        {
            uint64_t indjj = jj + (*cropystart);
            indjj *=  imgin.md->size[0];


            switch (imgin.md->datatype)
            {
            case _DATATYPE_FLOAT:
                memcpy( &imgout.im->array.F[ jj * (*cropxsize)],
                        &imgin.im->array.F[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_FLOAT);
                break;

            case _DATATYPE_DOUBLE:
                memcpy( &imgout.im->array.D[ jj * (*cropxsize)],
                        &imgin.im->array.D[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_DOUBLE);
                break;

            case _DATATYPE_UINT8:
                memcpy( &imgout.im->array.UI8[ jj * (*cropxsize)],
                        &imgin.im->array.UI8[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_UINT8);
                break;

            case _DATATYPE_UINT16:
                memcpy( &imgout.im->array.UI16[ jj * (*cropxsize)],
                        &imgin.im->array.UI16[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_UINT16);
                break;

            case _DATATYPE_UINT32:
                memcpy( &imgout.im->array.UI32[ jj * (*cropxsize)],
                        &imgin.im->array.UI32[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_UINT32);
                break;

            case _DATATYPE_UINT64:
                memcpy( &imgout.im->array.UI64[ jj * (*cropxsize)],
                        &imgin.im->array.UI64[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_UINT64);
                break;

            case _DATATYPE_INT8:
                memcpy( &imgout.im->array.SI8[ jj * (*cropxsize)],
                        &imgin.im->array.SI8[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_INT8);
                break;

            case _DATATYPE_INT16:
                memcpy( &imgout.im->array.SI16[ jj * (*cropxsize)],
                        &imgin.im->array.SI16[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_INT16);
                break;

            case _DATATYPE_INT32:
                memcpy( &imgout.im->array.SI32[ jj * (*cropxsize)],
                        &imgin.im->array.SI32[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_INT32);
                break;

            case _DATATYPE_INT64:
                memcpy( &imgout.im->array.SI64[ jj * (*cropxsize)],
                        &imgin.im->array.SI64[ indjj + (*cropxstart) ],
                        *cropxsize * SIZEOF_DATATYPE_INT64);
                break;

            }

        }
        processinfo_update_output_stream(processinfo, imgout.ID);

    }
    INSERT_STD_PROCINFO_COMPUTEFUNC_END

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}





INSERT_STD_FPSCLIfunctions




// Register function in CLI
errno_t
CLIADDCMD_COREMODE_arith__crop2D()
{

    CLIcmddata.FPS_customCONFsetup = customCONFsetup;
    CLIcmddata.FPS_customCONFcheck = customCONFcheck;
    INSERT_STD_CLIREGISTERFUNC

    return RETURN_SUCCESS;
}
