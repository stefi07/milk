
#include "CommandLineInterface/CLIcore.h"


static char *inimname;

static uint32_t    *size0;
static long      fpi_size0 = -1;

static uint32_t    *size1;
static long      fpi_size1 = -1;

static uint32_t    *size2;
static long      fpi_size2 = -1;



static CLICMDARGDEF farg[] =
{
    {
        CLIARG_IMG,
        ".imname",
        "input image",
        "im1",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &inimname,
        NULL
    },
    {
        CLIARG_UINT32,
        ".size0",
        "axis 0 size",
        "128",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &size0,
        &fpi_size0
    },
    {
        CLIARG_UINT32,
        ".size1",
        "axis 1 size",
        "128",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &size1,
        &fpi_size1
    },
    {
        CLIARG_UINT32,
        ".size2",
        "axis 2 size",
        "128",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &size2,
        &fpi_size2
    }
};


static CLICMDDATA CLIcmddata =
{
    "set3Daxes",
    "set 3D image axes size",
    CLICMD_FIELDS_DEFAULTS
};



// detailed help
static errno_t help_function()
{
    return RETURN_SUCCESS;
}





errno_t image_set_3Daxes(
    IMGID    inimg,
    uint32_t imsize0,
    uint32_t imsize1,
    uint32_t imsize2
)
{
    DEBUG_TRACE_FSTART();

    printf("SIZES: %u %u %u\n", imsize0, imsize1, imsize2);

    long nelem = inimg.md->nelement;

    // if size=0, adopt input size




    uint32_t imsize0c = imsize0;
    if(imsize0 == 0)
    {
        imsize0c =  inimg.md->size[0];
    }


    uint32_t imsize1c = imsize1;
    if(imsize1 == 0)
    {
        if (inimg.md->naxis < 2)
        {
            // if 1D image, promote to 3D with size1 = 1
            imsize1c = 1;
        }
        else
        {
            imsize1c =  inimg.md->size[1];
        }
    }

    uint32_t imsize2c = imsize2;
    if(imsize2 == 0)
    {
        if (inimg.md->naxis < 3)
        {
            // if 1D or 2D image, promote to 3D with size1 = 1
            imsize2c = 1;
        }
        else
        {
            imsize2c =  inimg.md->size[2];
        }
    }



    long nelemout = imsize0c;
    nelemout *= imsize1c;
    nelemout *= imsize2c;


    if(nelemout == nelem)
    {
        inimg.md->naxis = 3;
        inimg.md->size[0] = imsize0c;
        inimg.md->size[1] = imsize1c;
        inimg.md->size[2] = imsize2c;
    }
    else
    {
        printf("total number of element (%ld) does not match input (%ld) - invalid sizes\n", nelemout, nelem);
    }


    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}




static errno_t compute_function()
{
    DEBUG_TRACE_FSTART();

    IMGID inimg = mkIMGID_from_name(inimname);
    resolveIMGID(&inimg, ERRMODE_ABORT);

    INSERT_STD_PROCINFO_COMPUTEFUNC_START
    {
        image_set_3Daxes(inimg, *size0, *size1, *size2);
        processinfo_update_output_stream(processinfo, inimg.ID);

    }
    INSERT_STD_PROCINFO_COMPUTEFUNC_END

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}





INSERT_STD_FPSCLIfunctions



// Register function in CLI
errno_t
CLIADDCMD_COREMOD_arith__imset_3Daxes()
{
    INSERT_STD_CLIREGISTERFUNC

    return RETURN_SUCCESS;
}



