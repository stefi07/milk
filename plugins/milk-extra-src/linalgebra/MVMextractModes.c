#ifdef HAVE_CUDA
#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <cusolverDn.h>
#include <device_types.h>
#endif


#include <pthread.h>


// Use MKL if available
// Otherwise use openBLAS
//
#ifdef HAVE_MKL
#include "mkl.h"
#define BLASLIB "IntelMKL"
#else
#ifdef HAVE_OPENBLAS
#include <cblas.h>
#include <lapacke.h>
#define BLASLIB "OpenBLAS"
#endif
#endif



#include "CommandLineInterface/CLIcore.h"
#include "CommandLineInterface/timeutils.h"

#include "MVM_CPU.h"



// Local variables pointers
static int32_t *GPUindex;
long fpi_GPUindex;


static uint32_t *mmax;
long fpi_mmax;

static uint32_t *nmax;
long fpi_nmax;



static char *insname;
long fpi_insname;

static char *inmasksname;
long fpi_inmasksname;

static char *immodes;
long fpi_immodes;

static char *outcoeff;
long fpi_outcoeff;

static int64_t *outinit;
long fpi_outinit;

static uint32_t *axmode;
long fpi_axmode;

static int64_t *PROCESS;
long fpi_PROCESS;

static int64_t *TRACEMODE;
long fpi_TRACEMODE;

static int64_t *MODENORM;
long fpi_MODENORM;

static char *intot_stream;
long fpi_intot_stream;

static char *inrefsname;
long fpi_inrefsname;

static char *outrefsname;
long fpi_outrefsname;

static uint64_t *twait;
long fpi_twait;



static CLICMDARGDEF farg[] =
{
    {
        CLIARG_INT32,
        ".GPUindex",
        "GPU index, 99 for CPU",
        "0",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &GPUindex,
        &fpi_GPUindex
    },
    {
        CLIARG_STREAM,
        ".insname",
        "input stream name",
        "inV",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &insname,
        &fpi_insname
    },
    {
        CLIARG_STREAM,
        ".inmasksname",
        "nput mask stream name",
        "inV",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &inmasksname,
        &fpi_inmasksname
    },
    {
        CLIARG_STREAM,
        ".immodes",
        "modes stream name",
        "mat",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &immodes,
        &fpi_immodes
    },
    {
        CLIARG_STREAM,
        ".outcoeff",
        "output coefficients",
        "outV",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &outcoeff,
        &fpi_outcoeff
    },
    {
        CLIARG_ONOFF,
        ".outinit",
        "output init mode",
        "0",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &outinit,
        &fpi_outinit
    },
    {
        CLIARG_UINT32,
        ".option.axmode",
        "0 for normal mode extraction, 1 for expansion",
        "0",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &axmode,
        &fpi_axmode
    },
    {
        CLIARG_ONOFF,
        ".option.PROCESS",
        "processing flag",
        "0",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &PROCESS,
        &fpi_PROCESS
    },
    {
        CLIARG_ONOFF,
        ".option.TRACEMODE",
        "writing trace",
        "0",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &TRACEMODE,
        &fpi_TRACEMODE
    },
    {
        CLIARG_ONOFF,
        ".option.MODENORM",
        "input modes normalization",
        "0",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &MODENORM,
        &fpi_MODENORM
    },
    {
        CLIARG_STREAM,
        ".option.sname_intot",
        "optional input normalization stream",
        "null",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &intot_stream,
        &fpi_intot_stream
    },
    {
        CLIARG_STREAM,
        ".option.sname_refin",
        "optional input reference to be subtracted stream",
        "null",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &inrefsname,
        &fpi_inrefsname
    },
    {
        CLIARG_STREAM,
        ".option.sname_refout",
        "optional output reference to be subtracted stream",
        "null",
        CLIARG_VISIBLE_DEFAULT,
        (void **) &outrefsname,
        &fpi_outrefsname
    },
    {
        CLIARG_UINT64,
        ".option.twait",
        "insert time wait [us] at each iteration",
        "0",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &twait,
        &fpi_twait
    },
    {
        CLIARG_UINT32,
        ".option.mmax",
        "partial computation: max m index value",
        "100000",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &mmax,
        &fpi_mmax
    },
    {
        CLIARG_UINT32,
        ".option.nmax",
        "partial computation: max n index value",
        "100000",
        CLIARG_HIDDEN_DEFAULT,
        (void **) &nmax,
        &fpi_nmax
    }
};

// Optional custom configuration setup.
// Runs once at conf startup
//
static errno_t customCONFsetup()
{
    if(data.fpsptr != NULL)
    {
        data.fpsptr->parray[fpi_insname].fpflag |=
            FPFLAG_STREAM_RUN_REQUIRED | FPFLAG_CHECKSTREAM;
        data.fpsptr->parray[fpi_immodes].fpflag |=
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
    "MVMmextrmodes", "extract modes by MVM", CLICMD_FIELDS_DEFAULTS
};

// detailed help
static errno_t help_function()
{
    return RETURN_SUCCESS;
}














static errno_t compute_function()
{
    DEBUG_TRACE_FSTART();

    int MODEVALCOMPUTE = 1; // 1 if compute, 0 if import


#ifdef HAVE_CUDA
    cublasHandle_t cublasH = NULL;
    cublasStatus_t cublas_status = CUBLAS_STATUS_SUCCESS;
    cudaError_t cudaStat = cudaSuccess;
    struct cudaDeviceProp deviceProp;
#endif

    float *d_modes = NULL; // linear memory of GPU
    float *d_in = NULL;
    float *d_modeval = NULL;




    // each step is 2x longer average than previous step
    uint32_t NBaveSTEP = 10;

    int initref = 0; // 1 when reference has been processed

    // CONNECT TO INPUT STREAM
    IMGID imgin = mkIMGID_from_name(insname);
    resolveIMGID(&imgin, ERRMODE_ABORT);
    printf("Input stream size : %u %u\n", imgin.md->size[0], imgin.md->size[1]);
    long m = imgin.md->size[0] * imgin.md->size[1];

    // CONNECT TO MASK STREAM
    int use_mask = 0; //flag indicating that the mask is being used
    uint32_t mask_npix = 0; //The number of 1 pixels in the mask
    uint32_t * mask_idx = NULL; //Array holding the indices of the 1 pixels
    float * masked_pix = NULL; //Array to hold the pixel values

    IMGID imgmask = mkIMGID_from_name(inmasksname);
    if(resolveIMGID(&imgmask, ERRMODE_WARN) != -1)
    {
        printf("Mask stream size : %u %u\n", imgmask.md->size[0], imgmask.md->size[1]);
        if(imgmask.md->size[0] == imgin.md->size[0] && imgmask.md->size[1] == imgin.md->size[1])
        {
            use_mask = 1;
        }
    }

    //use_mask = 0; //for testing

    //setup the mask
    //
    if(use_mask)
    {
        for(long n=0; n < imgmask.md->size[0]*imgmask.md->size[1]; ++n)
        {
            if(imgmask.im->array.F[n] == 1)
            {
                ++mask_npix;
            }
        }

        mask_idx = (uint32_t *) malloc( mask_npix * sizeof(long));
        masked_pix = (float *) malloc( mask_npix * sizeof(float));
        long nn = 0;
        for(long n=0; n < imgmask.md->size[0]*imgmask.md->size[1]; ++n)
        {
            if(imgmask.im->array.F[n] == 1)
            {
                mask_idx[nn] = n;
                ++nn;
            }
        }

        printf("Mask has : %u pixels (%f%%)\n", mask_npix, (100.0*mask_npix)/(imgmask.md->size[0]*imgmask.md->size[1]));
    }
    else
    {
        //Just use full image
        mask_npix = imgin.md->size[0]*imgin.md->size[1];
        printf("No mask using : %u pixels (%f%%)\n", mask_npix, (100.0*mask_npix)/(imgin.md->size[0]*imgin.md->size[1]));
    }



    // NORMALIZATION
    // CONNECT TO TOTAL FLUX STREAM
    imageID IDintot;
    IDintot = image_ID(intot_stream);
    int INNORMMODE = 0; // 1 if input normalized

    if(IDintot == -1)
    {
        INNORMMODE = 0;
        create_2Dimage_ID("intot_tmp", 1, 1, &IDintot);
        data.image[IDintot].array.F[0] = 1.0;
    }
    else
    {
        INNORMMODE = 1;
    }



    // CONNECT TO OPTIONAL INPUT REFERENCE STREAM
    imageID IDinref = -1;
    IMGID imginref = mkIMGID_from_name(inrefsname);
    resolveIMGID(&imginref, ERRMODE_WARN);
    if(imginref.ID == -1)
    {
        create_2Dimage_ID("_tmprefin",
                          imgin.md->size[0],
                          imgin.md->size[1],
                          &IDinref);
        for(uint64_t ii = 0; ii < imgin.md->size[0] * imgin.md->size[1]; ii++)
        {
            data.image[IDinref].array.F[ii] = 0.0;
        }
    }
    else
    {
        IDinref = imginref.ID;
    }


    // CONNECT TO OPTIONAL OUTPUT REFERENCE STREAM
    IMGID imgoutref = mkIMGID_from_name(outrefsname);
    resolveIMGID(&imgoutref, ERRMODE_WARN);


    // CONNECT TO MODES STREAM
    IMGID imgmodes = mkIMGID_from_name(immodes);
    resolveIMGID(&imgmodes, ERRMODE_ABORT);



    printf("Modes stream size : %u %u\n",
           imgmodes.md->size[0],
           imgmodes.md->size[1]);


    long n;
    long NBmodes = 1;
    imageID IDmodes = -1;


    if((*axmode) == 0)
    {
        //
        // Extract modes.
        // This is the default geometry, no need to remap
        //
        n = imgmodes.md->size[2];
        IDmodes = imgmodes.ID;
        NBmodes = n;
        printf("NBmodes = %ld\n", NBmodes);
        fflush(stdout);


        // make col-major storage
    }
    else
    {
        //
        // Expand
        // Remap to new matrix tmpmodes
        //

        NBmodes = imgmodes.md->size[0] * imgmodes.md->size[1];
        n = NBmodes;
        printf("NBmodes = %ld\n", NBmodes);
        fflush(stdout);

        printf("creating _tmpmodes  %ld %ld %ld\n",
               (long)imgin.md->size[0],
               (long)imgin.md->size[1],
               NBmodes);
        fflush(stdout);

        create_3Dimage_ID("_tmpmodes",
                          imgin.md->size[0],
                          imgin.md->size[1],
                          NBmodes,
                          &IDmodes);

        for(uint32_t ii = 0; ii < imgin.md->size[0]; ii++)
            for(uint32_t jj = 0; jj < imgin.md->size[1]; jj++)
            {
                for(long kk = 0; kk < NBmodes; kk++)
                {
                    data.image[IDmodes]
                    .array.F[kk * imgin.md->size[0] * imgin.md->size[1] +
                                jj * imgin.md->size[0] + ii] =
                                 imgmodes.im->array
                                 .F[NBmodes * (jj * imgin.md->size[0] + ii) + kk];
                }
            }

        // save_fits("_tmpmodes", "_test_tmpmodes.fits");
    }

    float *normcoeff = (float *)malloc(sizeof(float) * NBmodes);

    if((*MODENORM) == 1)
    {
        // In this mode, the input modes are normalized to unity (vector 2-norm)
        // norm is computed here


        // compute normalization coeffs
        for(long k = 0; k < NBmodes; k++)
        {
            normcoeff[k] = 0.0;
            for(long ii = 0; ii < m; ii++)
            {
                normcoeff[k] += imgmodes.im->array.F[k * m + ii] *
                                imgmodes.im->array.F[k * m + ii];
            }
        }
    }
    else
    {
        // or set them to 1
        for(long k = 0; k < NBmodes; k++)
        {
            normcoeff[k] = 1.0;
        }
    }

    float *modevalarray = (float *)malloc(sizeof(float) * n);
    float *modevalarrayref = (float *)malloc(sizeof(float) * n);

    uint32_t *arraytmp = (uint32_t *)malloc(sizeof(uint32_t) * 2);

    // IDrefout = image_ID(IDrefout_name);
    imageID IDrefout = -1; // TODO handle this
    if(IDrefout == -1)
    {
        if((*axmode) == 0)
        {
            arraytmp[0] = NBmodes;
            arraytmp[1] = 1;
        }
        else
        {
            arraytmp[0] = imgmodes.md->size[0];
            arraytmp[1] = imgmodes.md->size[1];
        }
    }
    else
    {
        arraytmp[0] = data.image[IDrefout].md->size[0];
        arraytmp[1] = data.image[IDrefout].md->size[1];
    }



    // CONNNECT TO OR CREATE OUTPUT STREAM
    IMGID imgout = stream_connect_create_2Df32(outcoeff, arraytmp[0], arraytmp[1]);

    // Local working copy of output
    float *outarray = (float*) malloc(sizeof(float) * arraytmp[0] * arraytmp[1]);



    MODEVALCOMPUTE = 1;

    free(arraytmp);



    INSERT_STD_PROCINFO_COMPUTEFUNC_INIT;



    if(MODEVALCOMPUTE == 1)
    {

        if(((*GPUindex) >= 0) && ((*GPUindex) != 99))
        {

#ifdef HAVE_CUDA
            int deviceCount;
            int devicecntMax = 100;

            cudaGetDeviceCount(&deviceCount);
            printf("%d devices found\n", deviceCount);
            fflush(stdout);

            processinfo_WriteMessage_fmt(processinfo, "CUDA : %d devices", deviceCount);

            if(deviceCount > devicecntMax)
            {
                deviceCount = 0;
            }
            if(deviceCount < 0)
            {
                deviceCount = 0;
            }

            printf("\n");

            for(int k = 0; k < deviceCount; k++)
            {
                cudaGetDeviceProperties(&deviceProp, k);
                printf("Device %d / %d [ %20s ]  has compute capability %d.%d.\n",
                       k + 1,
                       deviceCount,
                       deviceProp.name,
                       deviceProp.major,
                       deviceProp.minor);
                printf(
                    "  Total amount of global memory:                 %.0f MBytes "
                    "(%llu bytes)\n",
                    (float)deviceProp.totalGlobalMem / 1048576.0f,
                    (unsigned long long)deviceProp.totalGlobalMem);
                printf("  (%2d) Multiprocessors\n", deviceProp.multiProcessorCount);
                printf(
                    "  GPU Clock rate:                                %.0f MHz "
                    "(%0.2f GHz)\n",
                    deviceProp.clockRate * 1e-3f,
                    deviceProp.clockRate * 1e-6f);
                printf("\n");
            }

            if((*GPUindex) < deviceCount)
            {
                cudaSetDevice(*GPUindex);
            }
            else
            {
                printf("Invalid Device : %d / %d\n", *GPUindex, deviceCount);
                processinfo_WriteMessage_fmt(processinfo, "Invalid GPU device %d", *GPUindex);
                exit(0);
            }

            printf("Create cublas handle ...");
            fflush(stdout);
            cublas_status = cublasCreate(&cublasH);
            if(cublas_status != CUBLAS_STATUS_SUCCESS)
            {
                printf("CUBLAS initialization failed\n");
                return EXIT_FAILURE;
            }
            printf(" done\n");
            fflush(stdout);

            long matsz;
            float * modesmat;

            if(use_mask)
            {
                //reformat the matrix using the mask
                matsz = mask_npix * NBmodes;
                modesmat = (float *) malloc(sizeof(float)*mask_npix * data.image[IDmodes].md->size[2]);

                uint32_t nrows = data.image[IDmodes].md->size[2];
                uint32_t ncols = data.image[IDmodes].md->size[0]*data.image[IDmodes].md->size[1];

                for(uint32_t rr = 0; rr < nrows; ++rr)
                {
                    for(uint32_t cc = 0; cc < mask_npix; ++cc)
                    {
                        modesmat[rr*mask_npix + cc] = data.image[IDmodes].array.F[rr*ncols + mask_idx[cc]];
                    }
                }
            }
            else
            {
                matsz = m * NBmodes;
                modesmat = data.image[IDmodes].array.F;
            }

            // load modes to GPU
            cudaStat = cudaMalloc((void **)&d_modes, sizeof(float) * matsz);
            if(cudaStat != cudaSuccess)
            {
                printf("cudaMalloc d_modes returned error code %d, line %d\n",
                       cudaStat,
                       __LINE__);
                exit(EXIT_FAILURE);
            }

            cudaStat = cudaMemcpy(d_modes,
                                  modesmat,
                                  sizeof(float) * matsz,
                                  cudaMemcpyHostToDevice);
            // cudaStat = cudaMemcpy(d_modes, imgmodes.im->array.F, sizeof(float) * m * NBmodes, cudaMemcpyHostToDevice);

            if(use_mask)
            {
                free(modesmat);
            }

            if(cudaStat != cudaSuccess)
            {
                printf("cudaMemcpy returned error code %d, line %d\n",
                       cudaStat,
                       __LINE__);
                exit(EXIT_FAILURE);
            }


            // create d_in
            cudaStat = cudaMalloc((void **)&d_in, sizeof(float) * m);
            if(cudaStat != cudaSuccess)
            {
                printf("cudaMalloc d_in returned error code %d, line %d\n",
                       cudaStat,
                       __LINE__);
                exit(EXIT_FAILURE);
            }

            // create d_modeval
            cudaStat = cudaMalloc((void **)&d_modeval, sizeof(float) * NBmodes);
            if(cudaStat != cudaSuccess)
            {
                printf("cudaMalloc d_modeval returned error code %d, line %d\n",
                       cudaStat,
                       __LINE__);
                exit(EXIT_FAILURE);
            }
#else
            processinfo_WriteMessage(processinfo, "NO CUDA - CPU fallback");
            *GPUindex = 99;
#endif
        }
    }

    if((*TRACEMODE) == 1)
    {
        char traceim_name[STRINGMAXLEN_IMGNAME];
        long TRACEsize = 2000;
        imageID IDtrace;

        uint32_t *sizearraytmp = (uint32_t *)malloc(sizeof(uint32_t) * 2);

        {
            int slen = snprintf(traceim_name,
                                STRINGMAXLEN_IMGNAME,
                                "%s_trace",
                                outcoeff);
            if(slen < 1)
            {
                PRINT_ERROR("snprintf wrote <1 char");
                abort(); // can't handle this error any other way
            }
            if(slen >= STRINGMAXLEN_IMGNAME)
            {
                PRINT_ERROR("snprintf string truncation");
                abort(); // can't handle this error any other way
            }
        }

        sizearraytmp[0] = TRACEsize;
        sizearraytmp[1] = NBmodes;
        IDtrace = image_ID(traceim_name);
        int imOK = 1;
        if(IDtrace == -1)
        {
            imOK = 0;
        }
        else
        {
            if((data.image[IDtrace].md[0].size[0] != TRACEsize) ||
                    (data.image[IDtrace].md[0].size[1] != NBmodes))
            {
                imOK = 0;
                delete_image_ID(traceim_name, DELETE_IMAGE_ERRMODE_WARNING);
            }
        }
        if(imOK == 0)
        {
            create_image_ID(traceim_name,
                            2,
                            sizearraytmp,
                            _DATATYPE_FLOAT,
                            1,
                            0,
                            0,
                            &IDtrace);
        }
        free(sizearraytmp);
    }

    if((*PROCESS) == 1)
    {
        char process_ave_name[STRINGMAXLEN_IMGNAME];
        char process_rms_name[STRINGMAXLEN_IMGNAME];
        imageID IDprocave;
        imageID IDprocrms;

        uint32_t *sizearraytmp = (uint32_t *)malloc(sizeof(uint32_t) * 2);

        {
            int slen = snprintf(process_ave_name,
                                STRINGMAXLEN_IMGNAME,
                                "%s_ave",
                                outcoeff);
            if(slen < 1)
            {
                PRINT_ERROR("snprintf wrote <1 char");
                abort(); // can't handle this error any other way
            }
            if(slen >= STRINGMAXLEN_IMGNAME)
            {
                PRINT_ERROR("snprintf string truncation");
                abort(); // can't handle this error any other way
            }
        }

        sizearraytmp[0] = NBmodes;
        sizearraytmp[1] = NBaveSTEP;
        IDprocave       = image_ID(process_ave_name);
        int imOK = 1;
        if(IDprocave == -1)
        {
            imOK = 0;
        }
        else
        {
            if((data.image[IDprocave].md[0].size[0] != NBmodes) ||
                    (data.image[IDprocave].md[0].size[1] != NBaveSTEP))
            {
                imOK = 0;
                delete_image_ID(process_ave_name, DELETE_IMAGE_ERRMODE_WARNING);
            }
        }
        if(imOK == 0)
        {
            create_image_ID(process_ave_name,
                            2,
                            sizearraytmp,
                            _DATATYPE_FLOAT,
                            1,
                            0,
                            0,
                            &IDprocave);
        }
        free(sizearraytmp);

        sizearraytmp = (uint32_t *)malloc(sizeof(uint32_t) * 2);

        {
            int slen = snprintf(process_rms_name,
                                STRINGMAXLEN_IMGNAME,
                                "%s_rms",
                                outcoeff);
            if(slen < 1)
            {
                PRINT_ERROR("snprintf wrote <1 char");
                abort(); // can't handle this error any other way
            }
            if(slen >= STRINGMAXLEN_IMGNAME)
            {
                PRINT_ERROR("snprintf string truncation");
                abort(); // can't handle this error any other way
            }
        }

        sizearraytmp[0] = NBmodes;
        sizearraytmp[1] = NBaveSTEP;
        IDprocrms = image_ID(process_rms_name);
        imOK = 1;
        if(IDprocrms == -1)
        {
            imOK = 0;
        }
        else
        {
            if((data.image[IDprocrms].md->size[0] != NBmodes) ||
                    (data.image[IDprocrms].md->size[1] != NBaveSTEP))
            {
                imOK = 0;
                delete_image_ID(process_rms_name, DELETE_IMAGE_ERRMODE_WARNING);
            }
        }
        if(imOK == 0)
        {
            create_image_ID(process_rms_name,
                            2,
                            sizearraytmp,
                            _DATATYPE_FLOAT,
                            1,
                            0,
                            0,
                            &IDprocrms);
        }
        free(sizearraytmp);
    }

    initref = 0; // 1 when reference has been processed

    // long twait1 = *twait;

    printf("LOOP START   MODEVALCOMPUTE = %d\n", MODEVALCOMPUTE);
    fflush(stdout);

    if(MODEVALCOMPUTE == 0)
    {
        printf("\n");
        printf("This function is NOT computing mode values\n");
        printf("Pre-existing stream %s was detected\n", outcoeff);
        printf("\n");
    }
    else
    {
        char msgstring[STRINGMAXLEN_PROCESSINFO_STATUSMSG];

        {
            int slen = snprintf(msgstring,
                                STRINGMAXLEN_PROCESSINFO_STATUSMSG,
                                "Running on GPU %d",
                                (*GPUindex));
            if(slen < 1)
            {
                PRINT_ERROR("snprintf wrote <1 char");
                abort(); // can't handle this error any other way
            }
            if(slen >= STRINGMAXLEN_PROCESSINFO_STATUSMSG)
            {
                PRINT_ERROR("snprintf string truncation");
                abort(); // can't handle this error any other way
            }
        }
    }

    printf(" m       = %u\n", mask_npix);
    printf(" n       = %ld\n", n);
    printf(" NBmodes = %ld\n", NBmodes);

    float alpha = 1.0;
    float beta = 0.0;
    uint64_t refindex = 0;

#ifdef HAVE_OPENBLAS
    printf("OpenBLASS  YES\n");
#else
    printf("OpenBLASS  NO\n");
#endif

#ifdef HAVE_MKL
    printf("MKL        YES\n");
#else
    printf("MKL        NO\n");
#endif


#ifdef HAVE_CUDA
    printf("CUDA       YES\n");
#else
    printf("CUDA       NO\n");
#endif




    float *ColMajorMatrix = (float *) malloc(sizeof(float) * m * n);
    if(*axmode == 0)
    {
        for(int ii = 0; ii < m; ii++)
        {
            for(int jj = 0; jj < n; jj++)
            {
                ColMajorMatrix[ii * n + jj] = imgmodes.im->array.F[jj * m + ii];
            }
        }
    }
    else
    {
        memcpy(ColMajorMatrix, imgmodes.im->array.F, sizeof(float)*m * n);
    }


    float * imginfloatptr = NULL;


    if( imgin.md->datatype == _DATATYPE_FLOAT )
    {
        imginfloatptr = imgin.im->array.F;
        printf("INPUT type = FLOAT  - no type conversion required\n");
    }
    else
    {
        imginfloatptr = (float*) malloc(sizeof(float) * imgin.md->size[0] * imgin.md->size[1] );
        printf("INPUT not float -> type conversion to float enabled\n");
    }


    printf(">>> START MVM loop\n");

    INSERT_STD_PROCINFO_COMPUTEFUNC_LOOPSTART
    {

        // Are we computing a new reference ?
        // if yes, set initref to 0 (reference is NOT initialized)
        //
        if(refindex != data.image[IDinref].md->cnt0)
        {
            initref = 0;
            refindex = data.image[IDinref].md->cnt0;
        }


        if(((*GPUindex) < 0) || (*GPUindex == 99))
        {
            // using CPU

#ifdef BLASLIB
            struct timespec t0, t1;
            clock_gettime(CLOCK_MILK, &t0);
            processinfo_WriteMessage_fmt(processinfo, "imgout %s ID %d", imgout.md->name,
                                         imgout.ID);
            {
                float beta = 0.0;

                if(imgoutref.ID != -1)
                {
                    beta = 1.0;
                    memcpy(outarray, imgoutref.im->array.F, sizeof(float)*n);
                }



                if( imgin.md->datatype != _DATATYPE_FLOAT )
                {
                    imginfloatptr = imgin.im->array.F;


                    // type conversion (if needed)
                    switch (imgin.md->datatype )
                    {
                    case _DATATYPE_DOUBLE:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.D[ii];
                        }
                        break;

                    case _DATATYPE_UINT8:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.UI8[ii];
                        }
                        break;

                    case _DATATYPE_INT8:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.SI8[ii];
                        }
                        break;

                    case _DATATYPE_UINT16:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.UI16[ii];
                        }
                        break;

                    case _DATATYPE_INT16:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.SI16[ii];
                        }
                        break;

                    case _DATATYPE_UINT32:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.UI32[ii];
                        }
                        break;

                    case _DATATYPE_INT32:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.SI32[ii];
                        }
                        break;

                    case _DATATYPE_UINT64:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.UI64[ii];
                        }
                        break;

                    case _DATATYPE_INT64:
                        for(int ii=0; ii<imgin.md->size[0] * imgin.md->size[1]; ii++)
                        {
                            imginfloatptr[ii] = (float) imgin.im->array.SI64[ii];
                        }
                        break;


                    }
                }



                if(*axmode == 1)
                {
                    cblas_sgemv(CblasColMajor,
                                CblasNoTrans, (int) n, (int) m,
                                1.0, ColMajorMatrix, (int) n,
                                imginfloatptr, 1, beta,
                                outarray, 1);
                }
                else
                {
                    cblas_sgemv(CblasColMajor,
                                CblasNoTrans, (int) n, (int) m,
                                1.0, ColMajorMatrix, (int) n,
                                imginfloatptr, 1, beta,
                                outarray, 1);
                }

                clock_gettime(CLOCK_MILK, &t1);
                struct timespec tdiff;
                tdiff = timespec_diff(t0, t1);
                double t01d  = 1.0 * tdiff.tv_sec + 1.0e-9 * tdiff.tv_nsec;
                processinfo_WriteMessage_fmt(processinfo, "%s %dx%d MVM %.3f us", BLASLIB, n, m,
                                             t01d * 1e6);
            }
#else
            // Run on CPU without lib
            int mmax1 = (*mmax);
            if(mmax1 > m)
            {
                mmax1 = m;
            }

            int nmax1 = (*nmax);
            if(nmax1 > n)
            {
                nmax1 = n;
            }

            for(int jj = 0; jj < n; jj++)
            {
                outarray[jj] = 0.0;
            }


            for(int ii = 0; ii < m; ii++)
            {
                for(int jj = 0; jj < n; jj++)
                {
                    int index = ii * n + jj;
                    outarray[jj] += imgmodes.im->array.F[index] * imginfloatptr[ii];
                }
            }

#endif

            // update output
            data.image[imgout.ID].md->write = 1;
            for(int jj = 0; jj < n; jj++)
            {
                imgout.im->array.F[jj] = outarray[jj] / normcoeff[jj];
            }
//            memcpy(imgout.im->array.F, outarray, sizeof(float)*n);
            processinfo_update_output_stream(processinfo, imgout.ID);
        }
        else
        {
            // running on GPU
#ifdef HAVE_CUDA

            struct timespec t0, t1;
            clock_gettime(CLOCK_MILK, &t0);

            // load in_stream to GPU
            if(initref == 0)
            {
                if(use_mask == 1)
                {
                    for(uint32_t cc = 0; cc < mask_npix; ++cc)
                    {
                        masked_pix[cc] = data.image[IDinref].array.F[mask_idx[cc]];
                    }
                }
                else
                {
                    masked_pix = data.image[IDinref].array.F;
                }
                cudaStat = cudaMemcpy(d_in,
                                      masked_pix,
                                      sizeof(float) * mask_npix,
                                      cudaMemcpyHostToDevice);
            }
            else
            {
                if(use_mask == 1)
                {
                    for(uint32_t cc = 0; cc < mask_npix; ++cc)
                    {
                        masked_pix[cc] = imginfloatptr[mask_idx[cc]];
                    }
                }
                else
                {
                    masked_pix = imginfloatptr;
                }
                cudaStat = cudaMemcpy(d_in,
                                      masked_pix,
                                      sizeof(float) * mask_npix,
                                      cudaMemcpyHostToDevice);
            }

            if(cudaStat != cudaSuccess)
            {
                printf("initref = %d    %ld  %ld\n", initref, IDinref, imgin.ID);
                printf("cudaMemcpy returned error code %d, line %d\n",
                       cudaStat,
                       __LINE__);
                exit(EXIT_FAILURE);
            }

            // compute
            cublas_status = cublasSgemv(cublasH,
                                        CUBLAS_OP_T,
                                        mask_npix,
                                        NBmodes,
                                        &alpha,
                                        d_modes,
                                        mask_npix,
                                        d_in,
                                        1,
                                        &beta,
                                        d_modeval,
                                        1);
            if(cublas_status != CUBLAS_STATUS_SUCCESS)
            {
                printf("cublasSgemv returned error code %d, line(%d)\n",
                       cublas_status,
                       __LINE__);
                fflush(stdout);
                if(cublas_status == CUBLAS_STATUS_NOT_INITIALIZED)
                {
                    printf("   CUBLAS_STATUS_NOT_INITIALIZED\n");
                }
                if(cublas_status == CUBLAS_STATUS_INVALID_VALUE)
                {
                    printf("   CUBLAS_STATUS_INVALID_VALUE\n");
                }
                if(cublas_status == CUBLAS_STATUS_ARCH_MISMATCH)
                {
                    printf("   CUBLAS_STATUS_ARCH_MISMATCH\n");
                }
                if(cublas_status == CUBLAS_STATUS_EXECUTION_FAILED)
                {
                    printf("   CUBLAS_STATUS_EXECUTION_FAILED\n");
                }

                printf("GPU index                           = %d\n", (*GPUindex));

                printf("CUBLAS_OP                           = %d\n", CUBLAS_OP_T);
                printf("alpha                               = %f\n", alpha);
                printf("alpha                               = %f\n", beta);
                printf("m                                   = %d\n", (int)m);
                printf("NBmodes                             = %d\n", (int)NBmodes);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }

            // copy result
            imgout.md->write = 1;

            if(initref == 0)
            {
                // construct reference to be subtracted
                printf("... reference compute\n");
                cudaStat = cudaMemcpy(modevalarrayref,
                                      d_modeval,
                                      sizeof(float) * NBmodes,
                                      cudaMemcpyDeviceToHost);

                IDrefout = image_ID(outrefsname);
                if(IDrefout != -1)
                    for(long k = 0; k < NBmodes; k++)
                    {
                        modevalarrayref[k] -= data.image[IDrefout].array.F[k];
                    }

            }
            else
            {
                cudaStat = cudaMemcpy(modevalarray,
                                      d_modeval,
                                      sizeof(float) * NBmodes,
                                      cudaMemcpyDeviceToHost);


                for(long k = 0; k < NBmodes; k++)
                {
                    imgout.im->array.F[k] =
                        (modevalarray[k] / data.image[IDintot].array.F[0] -
                         modevalarrayref[k]) /
                        normcoeff[k];
                }




                clock_gettime(CLOCK_MILK, &t1);
                struct timespec tdiff;
                tdiff = timespec_diff(t0, t1);
                double t01d  = 1.0 * tdiff.tv_sec + 1.0e-9 * tdiff.tv_nsec;
                processinfo_WriteMessage_fmt(processinfo, "GPU%d %dx%d MVM %.3f us", *GPUindex,
                                             n, m, t01d * 1e6);



                processinfo_update_output_stream(processinfo, imgout.ID);
            }
#endif
        }

        initref = 1;
    }

    INSERT_STD_PROCINFO_COMPUTEFUNC_END

    free(outarray);

    free(ColMajorMatrix);

    free(normcoeff);
    free(modevalarray);
    free(modevalarrayref);


    if( imgin.md->datatype != _DATATYPE_FLOAT )
    {
        free(imginfloatptr);
    }



    if(use_mask)
    {
        free(masked_pix);
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}




INSERT_STD_FPSCLIfunctions




// Register function in CLI
errno_t
CLIADDCMD_linalgebra__MVMextractModes()
{

    CLIcmddata.FPS_customCONFsetup = customCONFsetup;
    CLIcmddata.FPS_customCONFcheck = customCONFcheck;
    INSERT_STD_CLIREGISTERFUNC

    return RETURN_SUCCESS;
}
