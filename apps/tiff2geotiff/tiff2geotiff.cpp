/* tiff2geotiff.c -- based on Sam Leffler's "tiffcp" and "geotifcp" codes */

/*
 *  Original code had this copyright notice:
 *
 * Copyright (c) 1988-1995 Sam Leffler
 * Copyright (c) 1991-1995 Silicon Graphics, Inc.
 *
 * and a lot of legal stuff denying liability for anything.
 * This version modified (by A. Norton) to enable specifying a file of lon-lat extents
 * and timesteps that can be inserted to georeference and date the
 * separate directories in the output geotiff file.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <stdint.h>
#include "vapor/VAssert.h"

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

/* GeoTIFF overrides */

#ifdef WIN32
    #include "geotiff/geotiff.h"
    #include "geotiff/geo_normalize.h"
    #include "geotiff/geo_tiffp.h"
    #include "geotiff/geo_keyp.h"
    #include "geotiff/xtiffio.h"
    #include "geotiff/cpl_serv.h"
    #include "proj_api.h"
#else
    #include "geotiff.h"
    #include "geo_normalize.h"
    #include "geo_tiffp.h"
    #include "geo_keyp.h"
    #include "xtiffio.h"
    #include "cpl_serv.h"
    #include "proj_api.h"
#endif

#ifdef WIN32
    #include <float.h>
    #pragma warning(disable : 4996)
#endif

#define TIFFOpen  XTIFFOpen
#define TIFFClose XTIFFClose

#if defined(VMS)
    #define unlink delete
#endif

#define streq(a, b)     (strcmp(a, b) == 0)
#define strneq(a, b, n) (strncmp(a, b, n) == 0)

#define TRUE  1
#define FALSE 0

int getopt(int nargc, char **nargv, const char *ostr);

static int    outtiled = -1;
static uint32_t tilewidth;
static uint32_t tilelength;
static int    convert_8_to_4 = 0;

static uint16_t      config;
static uint16_t      compression;
static uint16_t      predictor;
static uint16_t      fillorder;
static uint32_t      rowsperstrip;
static uint32_t      g3opts;
static int         ignore = FALSE; /* if true, ignore read errors */
static uint32_t      defg3opts = (uint32_t)-1;
static int         quality = 75; /* JPEG quality */
static int         jpegcolormode = JPEGCOLORMODE_RGB;
static uint16_t      defcompression = (uint16_t)-1;
static uint16_t      defpredictor = (uint16_t)-1;
static const char *geofile = (char *)0;
static const char *timeLonLatName = (char *)0;
static const char *timeName = (char *)0;
static FILE *      timeLonLatFile = (FILE *)0;
static FILE *      timeFile = (FILE *)0;
static float       lonLatExts[4] = {999.f, 999.f, 999.f, 999.f};
static uint32_t      currentImageWidth;
static uint32_t      currentImageHeight;

static const char *proj4_string = (char *)0;
static const char *p4string;
static const char *worldfile = (char *)0;
static int         dirnum = 0;
static void        ApplyWorldFile(const char *worldfile, TIFF *out);
static int         tiffcp(TIFF *, TIFF *);
static int         processCompressOptions(const char *);
static void        usage(void);
static int         applyCorners(float cors[4], float relpos[4], TIFF *out);
extern int         GTIFSetFromProj4_WRF(GTIF *gtif, const char *proj4);

int main(int argc, char *argv[])
{
    uint16_t             defconfig = (uint16_t)-1;
    uint16_t             deffillorder = 0;
    uint32_t             deftilewidth = (uint32_t)-1;
    uint32_t             deftilelength = (uint32_t)-1;
    uint32_t             defrowsperstrip = (uint32_t)-1;
    uint32_t             diroff = 0;
    TIFF *             in;
    TIFF *             out;
    const char *       mode = "w";
    int                c;
    extern int         optind;
    extern const char *myoptarg;

    while ((c = getopt(argc, argv, "c:f:l:m:M:n:o:p:r:w:e:g:4:aistd")) != -1) switch (c) {
        case 'a': /* append to output */ mode = "a"; break;
        case 'd': /* down cast 8bit to 4bit */ convert_8_to_4 = 1; break;
        case 'c': /* compression scheme */
            if (!processCompressOptions(myoptarg)) usage();
            break;
        case 'e': worldfile = myoptarg; break;
        case 'f': /* fill order */
            if (streq(myoptarg, "lsb2msb"))
                deffillorder = FILLORDER_LSB2MSB;
            else if (streq(myoptarg, "msb2lsb"))
                deffillorder = FILLORDER_MSB2LSB;
            else
                usage();
            break;
        case 'i': /* ignore errors */ ignore = TRUE; break;
        case 'g': /* GeoTIFF metadata file */ geofile = myoptarg; break;
        case 'm': /*multiple times and latlon extents file */
            timeLonLatName = myoptarg;
            timeLonLatFile = fopen(timeLonLatName, "r");
            if (!timeLonLatFile) {
                fprintf(stderr, "Failure to open %s\n", timeLonLatName);
                exit(-1);
            }
            break;
        case 'M': /*multiple timestamps file */
            timeName = myoptarg;
            timeFile = fopen(timeName, "r");
            if (!timeFile) {
                fprintf(stderr, "Failure to open %s\n", timeName);
                exit(-1);
            }
            break;
        case 'n': /* single latlong extents, requires option 4 */
        {
            int retval = sscanf(myoptarg, "%f %f %f %f", lonLatExts, lonLatExts + 1, lonLatExts + 2, lonLatExts + 3);
            if (retval != 4) {
                fprintf(stderr, "Four lon/lat extent values required\n");
                exit(-1);
            }
        } break;

        case '4': proj4_string = myoptarg; break;
        case 'l': /* tile length */
            outtiled = TRUE;
            deftilelength = atoi(myoptarg);
            break;
        case 'o': /* initial directory offset */ diroff = strtoul(myoptarg, NULL, 0); break;
        case 'p': /* planar configuration */
            if (streq(myoptarg, "separate"))
                defconfig = PLANARCONFIG_SEPARATE;
            else if (streq(myoptarg, "contig"))
                defconfig = PLANARCONFIG_CONTIG;
            else
                usage();
            break;
        case 'r': /* rows/strip */ defrowsperstrip = atoi(myoptarg); break;
        case 's': /* generate stripped output */ outtiled = FALSE; break;
        case 't': /* generate tiled output */ outtiled = TRUE; break;
        case 'w': /* tile width */
            outtiled = TRUE;
            deftilewidth = atoi(myoptarg);
            break;
        case '?':
            usage();
            /*NOTREACHED*/
        }
    if (argc - optind < 2) usage();
    out = TIFFOpen(argv[argc - 1], mode);
    if (out == NULL) return (-2);
    for (; optind < argc - 1; optind++) {
        in = TIFFOpen(argv[optind], "r");
        if (in == NULL) return (-3);
        if (diroff != 0 && !TIFFSetSubDirectory(in, diroff)) {
            TIFFError(TIFFFileName(in), "Error, setting subdirectory at %#x", diroff);
            (void)TIFFClose(out);
            return (1);
        }
        do {
            config = defconfig;
            compression = defcompression;
            predictor = defpredictor;
            fillorder = deffillorder;
            rowsperstrip = defrowsperstrip;
            tilewidth = deftilewidth;
            tilelength = deftilelength;
            g3opts = defg3opts;
            if (!tiffcp(in, out) || !TIFFWriteDirectory(out)) {
                (void)TIFFClose(out);
                return (1);
            }
        } while (TIFFReadDirectory(in));
        (void)TIFFClose(in);
    }
    (void)TIFFClose(out);
    return (0);
}

static void ApplyWorldFile(const char *worldfilename, TIFF *out)

{
    FILE * tfw;
    double pixsize[3], xoff, yoff, tiepoint[6], x_rot, y_rot;

    /*
     * Read the world file.  Note we currently ignore rotational coefficients!
     */
    tfw = fopen(worldfilename, "rt");
    if (tfw == NULL) {
        perror(worldfilename);
        return;
    }

    int rt;
    rt = std::fscanf(tfw, "%lf", pixsize + 0);
    VAssert(rt > 0 && rt != EOF);
    rt = std::fscanf(tfw, "%lf", &y_rot);
    VAssert(rt > 0 && rt != EOF);
    rt = std::fscanf(tfw, "%lf", &x_rot);
    VAssert(rt > 0 && rt != EOF);
    rt = std::fscanf(tfw, "%lf", pixsize + 1);
    VAssert(rt > 0 && rt != EOF);
    rt = std::fscanf(tfw, "%lf", &xoff);
    VAssert(rt > 0 && rt != EOF);
    rt = std::fscanf(tfw, "%lf", &yoff);
    VAssert(rt > 0 && rt != EOF);

    fclose(tfw);

    /*
     * Write out pixel scale, and tiepoint information.
     */
    if (x_rot == 0.0 && y_rot == 0.0) {
        pixsize[1] = ABS(pixsize[1]);
        pixsize[2] = 0.0;
        TIFFSetField(out, GTIFF_PIXELSCALE, 3, pixsize);

        tiepoint[0] = 0.5;
        tiepoint[1] = 0.5;
        tiepoint[2] = 0.0;
        tiepoint[3] = xoff;
        tiepoint[4] = yoff;
        tiepoint[5] = 0.0;
        TIFFSetField(out, GTIFF_TIEPOINTS, 6, tiepoint);
    } else {
        double adfMatrix[16];

        memset(adfMatrix, 0, sizeof(double) * 16);

        adfMatrix[0] = pixsize[0];
        adfMatrix[1] = x_rot;
        adfMatrix[3] = xoff - (pixsize[0] + x_rot) * 0.5;
        adfMatrix[4] = y_rot;
        adfMatrix[5] = pixsize[1];
        adfMatrix[7] = yoff - (pixsize[1] + y_rot) * 0.5;
        adfMatrix[15] = 1.0;

        TIFFSetField(out, TIFFTAG_GEOTRANSMATRIX, 16, adfMatrix);
    }
}

static void InstallGeoTIFF(TIFF *out)
{
    GTIF *gtif = (GTIF *)0; /* GeoKey-level descriptor */
    FILE *fd;

    gtif = GTIFNew(out);
    if (!gtif) {
        printf("failed in GTIFNew\n");
        return;
    }

    if (geofile) {
        /* Install keys and tags */
        fd = fopen(geofile, "r");
        if (fd == NULL) {
            perror(geofile);
            exit(-1);
        }
        if (!GTIFImport(gtif, 0, fd)) {
            fprintf(stderr, "Failure in GTIFImport\n");
            exit(-1);
        }
        fclose(fd);
    } else if (proj4_string) {
        // Make sure ellps is in string:
        int pos;
        for (pos = 0; pos < (int)strlen(proj4_string) - 6; pos++) {
            if (strncmp(proj4_string + pos, "+ellps", 6) == 0) {
                pos = -1;
                break;
            }
        }
        p4string = proj4_string;
        if (pos >= 0) {
            char *newString = new char[strlen(proj4_string) + 15];
            strcpy(newString, proj4_string);
            strcat(newString, " +ellps=sphere");

            p4string = newString;
        }

        if (!GTIFSetFromProj4_WRF(gtif, p4string)) {
            fprintf(stderr, "Failure in GTIFSetFromProj4_WRF\n");
            exit(-1);
        }
        if (timeLonLatFile) {
            // get next timestamp and latlon extents from timeLonLatFile
            float lonlat[4];
            float relPos[4];
            char  timestamp[20];
            // double modelPixelScale[3] = {0.,0.,0.};
            // double tiePoint[6] = {0.,0.,0.,0.,0.,0.};

            int rc = fscanf(timeLonLatFile, "%19s %f %f %f %f %f %f %f %f", timestamp, lonlat, lonlat + 1, lonlat + 2, lonlat + 3, relPos, relPos + 1, relPos + 2, relPos + 3);
            dirnum++;
            if (rc != 9) {
                fprintf(stderr, "Failed to read line %d of time-lon-lat file\n", dirnum);
                if (rc == 0) fprintf(stderr, "time-lon-lat file has fewer entries than images in tiff file\n");
                exit(-3);
            } else {    // convert the latlon and time and put into geotiff

                // insert time stamp from file
                TIFFSetField(out, TIFFTAG_DATETIME, timestamp);

                // Use proj4 to calculate the corner coordinates of the
                // image from the lonlat extents
                int rc = applyCorners(lonlat, relPos, out);
                if (rc) exit(rc);
            }
        } else if (lonLatExts[0] != 999.f) {
            // Use proj4 to calculate the corner coordinates of the
            // image from the lonlat extents
            float relpos[4] = {0.f, 0.f, 1.f, 1.f};
            int   rc = applyCorners(lonLatExts, relpos, out);
            if (rc) exit(rc);
        }
    } else if (timeFile) {
        // get next timestamp from timeFile
        char timestamp[20];
        int  rc = fscanf(timeFile, "%19s", timestamp);
        dirnum++;
        if (rc != 1) {
            fprintf(stderr, "Failed to read line %d of timestamp file\n", dirnum);
            if (rc == 0) fprintf(stderr, "timestamp file has fewer entries than images in tiff file\n");
            exit(-3);
        } else {    // put timestamp into tiff

            // insert time stamp from file
            TIFFSetField(out, TIFFTAG_DATETIME, timestamp);
        }
    }
    GTIFWriteKeys(gtif);
    GTIFFree(gtif);
    return;
}
//
static int applyCorners(float lonlat[4], float relPos[4], TIFF *out)
{
    void * p;
    double modelPixelScale[3] = {0., 0., 0.};
    double tiePoint[6] = {0., 0., 0., 0., 0., 0.};
    p = pj_init_plus(p4string);

    if (!p && !ignore) {
        // Invalid string. Get the error code:
        int *pjerrnum = pj_get_errno_ref();
        fprintf(stderr, "Invalid Proj4 string %s; message:\n %s\n", p4string, pj_strerrno(*pjerrnum));
        return -1;
    }
    if (p) {
        // reproject latlon to specified coord projection.
        // unless it's already a lat/lon projection
        double dbextents[4];
        if (pj_is_latlong(static_cast<projPJ>(p))) {
            for (int j = 0; j < 4; j++) { dbextents[j] = lonlat[j]; }
        } else {
            // Must convert to radians:
            const double DEG2RAD = 3.141592653589793 / 180.;
            const char * latLongProjString = "+proj=latlong +ellps=sphere";
            projPJ       latlon_p = pj_init_plus(latLongProjString);
            // convert to radians...
            for (int j = 0; j < 4; j++) dbextents[j] = lonlat[j] * DEG2RAD;
            // convert the latlons to coordinates in the projection.
            double dummy[1] = {0.0};
            int    rc = pj_transform(latlon_p, static_cast<projPJ>(p), 2, 2, dbextents, dbextents + 1, dummy);
            if (rc && !ignore) {
                int *pjerrnum = pj_get_errno_ref();
                fprintf(stderr, "Error converting lonlat to projection\n %s\n", pj_strerrno(*pjerrnum));
                return (-1);
            }
        }
        // Now the extents in projection space must be scaled, to
        // allow for the corners being in the interior of the page.
        // If R0 and R1 are the relative positions of the plot corners
        // in the page, and X0 and X1 are the x-coords of the plot corners
        // then the page corners are at:
        // LOWER = (X0*R1 - X1*R0)/(R1-R0)
        // UPPER = LOWER + (X1-X0)/(R1-R0)

        // When dealing with x coordinates,
        // R0 and R1 are relPos[0] and [2] , X0 and X1 are dbextents[0] and [2]
        // similarly the y coordinates use the [1] and [3] indices
        double newDBExtents[4];
        newDBExtents[0] = (dbextents[0] * relPos[2] - dbextents[2] * relPos[0]) / (relPos[2] - relPos[0]);
        newDBExtents[2] = newDBExtents[0] + (dbextents[2] - dbextents[0]) / (relPos[2] - relPos[0]);
        newDBExtents[1] = (dbextents[1] * relPos[3] - dbextents[3] * relPos[1]) / (relPos[3] - relPos[1]);
        newDBExtents[3] = newDBExtents[1] + (dbextents[3] - dbextents[1]) / (relPos[3] - relPos[1]);
        // calculate scale and model tie point
        modelPixelScale[0] = (newDBExtents[2] - newDBExtents[0]) / ((double)currentImageWidth - 1.);
        modelPixelScale[1] = (newDBExtents[3] - newDBExtents[1]) / ((double)currentImageHeight - 1.);

        tiePoint[3] = newDBExtents[0];
        // Following is just dbextents[1] + dbextents[3]-dbextents[1] = dbextents[3].
        // tiePoint[4] = dbextents[1] + ((double)currentImageHeight -1.)*modelPixelScale[1];
        tiePoint[4] = newDBExtents[3];
        TIFFSetField(out, GTIFF_TIEPOINTS, 6, tiePoint);
        TIFFSetField(out, GTIFF_PIXELSCALE, 3, modelPixelScale);
    }
    return 0;
}

static void CopyGeoTIFF(TIFF *in, TIFF *out)
{
    GTIF *  gtif = (GTIF *)0; /* GeoKey-level descriptor */
    double *d_list = NULL;
    int16_t   d_list_count;

    /* read definition from source file. */
    gtif = GTIFNew(in);
    if (!gtif) return;

    if (TIFFGetField(in, GTIFF_TIEPOINTS, &d_list_count, &d_list)) TIFFSetField(out, GTIFF_TIEPOINTS, d_list_count, d_list);
    if (TIFFGetField(in, GTIFF_PIXELSCALE, &d_list_count, &d_list)) TIFFSetField(out, GTIFF_PIXELSCALE, d_list_count, d_list);
    if (TIFFGetField(in, GTIFF_TRANSMATRIX, &d_list_count, &d_list)) TIFFSetField(out, GTIFF_TRANSMATRIX, d_list_count, d_list);

    /* Here we violate the GTIF abstraction to retarget on another file.
       We should just have a function for copying tags from one GTIF object
       to another. */
    gtif->gt_tif = out;
    gtif->gt_flags |= FLAG_FILE_MODIFIED;

    /* Install keys and tags */
    GTIFWriteKeys(gtif);
    GTIFFree(gtif);
    return;
}

static void processG3Options(const char *cp)
{
    if ((cp = strchr(cp, ':')) != NULL) {
        if (defg3opts == (uint32_t)-1) defg3opts = 0;
        do {
            cp++;
            if (strneq(cp, "1d", 2))
                defg3opts &= ~GROUP3OPT_2DENCODING;
            else if (strneq(cp, "2d", 2))
                defg3opts |= GROUP3OPT_2DENCODING;
            else if (strneq(cp, "fill", 4))
                defg3opts |= GROUP3OPT_FILLBITS;
            else
                usage();
        } while ((cp = strchr(cp, ':')) != NULL);
    }
}

static int processCompressOptions(const char *opt)
{
    if (streq(opt, "none"))
        defcompression = COMPRESSION_NONE;
    else if (streq(opt, "packbits"))
        defcompression = COMPRESSION_PACKBITS;
    else if (strneq(opt, "jpeg", 4)) {
        const char *cp = strchr(opt, ':');
        if (cp && isdigit(cp[1])) quality = atoi(cp + 1);
        if (cp && strchr(cp, 'r')) jpegcolormode = JPEGCOLORMODE_RAW;
        defcompression = COMPRESSION_JPEG;
    } else if (strneq(opt, "g3", 2)) {
        processG3Options(opt);
        defcompression = COMPRESSION_CCITTFAX3;
    } else if (streq(opt, "g4"))
        defcompression = COMPRESSION_CCITTFAX4;
    else if (strneq(opt, "lzw", 3)) {
        const char *cp = strchr(opt, ':');
        if (cp) defpredictor = atoi(cp + 1);
        defcompression = COMPRESSION_LZW;
    } else if (strneq(opt, "zip", 3)) {
        const char *cp = strchr(opt, ':');
        if (cp) defpredictor = atoi(cp + 1);
        defcompression = COMPRESSION_DEFLATE;
    } else
        return (0);
    return (1);
}

const char *stuff[] = {"usage: tiff2geotiff [options] input... output",
                       "where options are:",
                       " -g file	install GeoTIFF metadata from <file>",
                       " -4 proj4_str	install GeoTIFF metadata from proj4 string",
                       " -e file	install positioning info from ESRI Worldfile <file>",
                       " -a		append to output instead of overwriting",
                       " -m file	Specify filename with multiple timestamps and image placement info:",
                       "			Each line of file has date/timestamp, and 8 floats;",
                       "			first four are lon/lat corners of plot area,",
                       "			second four are relative positions of plot corners in page.",
                       "			This option requires option -4",
                       " -M file   Specify filename with multiple timestamps, w/o georeferencing:",
                       "			Each line of file has date/timestamp only",
                       "			Option -4 must not be specified.",
                       " -n llx lly urx ury",
                       "			Install longitude/latitude extents;",
                       "			Four lon and lat values must in quotes in the order:",
                       "			lower-left longitude, lower-left latitude,",
                       "			upper-right longitute, upper-right latitude",
                       "			This option requires option -4",
                       "			Option '-m' overrides this option",
                       " -o offset	set initial directory offset",
                       " -p contig	pack samples contiguously (e.g. RGBRGB...)",
                       " -p separate	store samples separately (e.g. RRR...GGG...BBB...)",
                       " -s		write output in strips",
                       " -t		write output in tiles",
                       " -i		ignore read errors",
                       " -d		truncate 8 bitspersample to 4bitspersample",
                       "",
                       " -r #		make each strip have no more than # rows",
                       " -w #		set output tile width (pixels)",
                       " -l #		set output tile length (pixels)",
                       "",
                       " -f lsb2msb	force lsb-to-msb FillOrder for output",
                       " -f msb2lsb	force msb-to-lsb FillOrder for output",
                       "",
                       " -c lzw[:opts]	compress output with Lempel-Ziv & Welch encoding",
                       " -c zip[:opts]	compress output with deflate encoding",
                       " -c jpeg[:opts]compress output with JPEG encoding",
                       " -c packbits	compress output with packbits encoding",
                       " -c g3[:opts]	compress output with CCITT Group 3 encoding",
                       " -c g4		compress output with CCITT Group 4 encoding",
                       " -c none	use no compression algorithm on output",
                       "",
                       "Group 3 options:",
                       " 1d		use default CCITT Group 3 1D-encoding",
                       " 2d		use optional CCITT Group 3 2D-encoding",
                       " fill		byte-align EOL codes",
                       "For example, -c g3:2d:fill to get G3-2D-encoded data with byte-aligned EOLs",
                       "",
                       "JPEG options:",
                       " #		set compression quality level (0-100, default 75)",
                       " r		output color image as RGB rather than YCbCr",
                       "For example, -c jpeg:r:50 to get JPEG-encoded RGB data with 50% comp. quality",
                       "",
                       "LZW and deflate options:",
                       " #		set predictor value",
                       "For example, -c lzw:2 to get LZW-encoded data with horizontal differencing",
                       NULL};

static void usage(void)
{
    char buf[BUFSIZ];
    int  i;

    setbuf(stderr, buf);
    for (i = 0; stuff[i] != NULL; i++) fprintf(stderr, "%s\n", stuff[i]);
    exit(-1);
}

static void CheckAndCorrectColormap(TIFF *tif, int n, uint16_t *r, uint16_t *g, uint16_t *b)
{
    int i;

    for (i = 0; i < n; i++)
        if (r[i] >= 256 || g[i] >= 256 || b[i] >= 256) return;
    TIFFWarning(TIFFFileName(tif), "Scaling 8-bit colormap");
#define CVT(x) (((x) * ((1L << 16) - 1)) / 255)
    for (i = 0; i < n; i++) {
        r[i] = CVT(r[i]);
        g[i] = CVT(g[i]);
        b[i] = CVT(b[i]);
    }
#undef CVT
}

#define CopyField(tag, v) \
    if (TIFFGetField(in, tag, &v)) TIFFSetField(out, tag, v)
#define CopyField2(tag, v1, v2) \
    if (TIFFGetField(in, tag, &v1, &v2)) TIFFSetField(out, tag, v1, v2)
#define CopyField3(tag, v1, v2, v3) \
    if (TIFFGetField(in, tag, &v1, &v2, &v3)) TIFFSetField(out, tag, v1, v2, v3)
#define CopyField4(tag, v1, v2, v3, v4) \
    if (TIFFGetField(in, tag, &v1, &v2, &v3, &v4)) TIFFSetField(out, tag, v1, v2, v3, v4)

static struct cpTag {
    uint16_t       tag;
    uint16_t       count;
    TIFFDataType type;
} tags[] = {
    {TIFFTAG_SUBFILETYPE, 1, TIFF_LONG},
    {TIFFTAG_THRESHHOLDING, 1, TIFF_SHORT},
    {TIFFTAG_DOCUMENTNAME, 1, TIFF_ASCII},
    {TIFFTAG_IMAGEDESCRIPTION, 1, TIFF_ASCII},
    {TIFFTAG_MAKE, 1, TIFF_ASCII},
    {TIFFTAG_MODEL, 1, TIFF_ASCII},
    {TIFFTAG_ORIENTATION, 1, TIFF_SHORT},
    {TIFFTAG_MINSAMPLEVALUE, 1, TIFF_SHORT},
    {TIFFTAG_MAXSAMPLEVALUE, 1, TIFF_SHORT},
    {TIFFTAG_XRESOLUTION, 1, TIFF_RATIONAL},
    {TIFFTAG_YRESOLUTION, 1, TIFF_RATIONAL},
    {TIFFTAG_PAGENAME, 1, TIFF_ASCII},
    {TIFFTAG_XPOSITION, 1, TIFF_RATIONAL},
    {TIFFTAG_YPOSITION, 1, TIFF_RATIONAL},
    {TIFFTAG_GROUP4OPTIONS, 1, TIFF_LONG},
    {TIFFTAG_RESOLUTIONUNIT, 1, TIFF_SHORT},
    {TIFFTAG_PAGENUMBER, 2, TIFF_SHORT},
    {TIFFTAG_SOFTWARE, 1, TIFF_ASCII},
    {TIFFTAG_DATETIME, 1, TIFF_ASCII},
    {TIFFTAG_ARTIST, 1, TIFF_ASCII},
    {TIFFTAG_HOSTCOMPUTER, 1, TIFF_ASCII},
    {TIFFTAG_WHITEPOINT, 1, TIFF_RATIONAL},
    {TIFFTAG_PRIMARYCHROMATICITIES, (uint16_t)-1, TIFF_RATIONAL},
    {TIFFTAG_HALFTONEHINTS, 2, TIFF_SHORT},
    {TIFFTAG_BADFAXLINES, 1, TIFF_LONG},
    {TIFFTAG_CLEANFAXDATA, 1, TIFF_SHORT},
    {TIFFTAG_CONSECUTIVEBADFAXLINES, 1, TIFF_LONG},
    {TIFFTAG_INKSET, 1, TIFF_SHORT},
    {TIFFTAG_INKNAMES, 1, TIFF_ASCII},
    {TIFFTAG_DOTRANGE, 2, TIFF_SHORT},
    {TIFFTAG_TARGETPRINTER, 1, TIFF_ASCII},
    {TIFFTAG_SAMPLEFORMAT, 1, TIFF_SHORT},
    {TIFFTAG_YCBCRCOEFFICIENTS, (uint16_t)-1, TIFF_RATIONAL},
    {TIFFTAG_YCBCRSUBSAMPLING, 2, TIFF_SHORT},
    {TIFFTAG_YCBCRPOSITIONING, 1, TIFF_SHORT},
    {TIFFTAG_REFERENCEBLACKWHITE, (uint16_t)-1, TIFF_RATIONAL},
    {TIFFTAG_EXTRASAMPLES, (uint16_t)-1, TIFF_SHORT},
    {TIFFTAG_SMINSAMPLEVALUE, 1, TIFF_DOUBLE},
    {TIFFTAG_SMAXSAMPLEVALUE, 1, TIFF_DOUBLE},
};
#define NTAGS (sizeof(tags) / sizeof(tags[0]))

static void cpOtherTags(TIFF *in, TIFF *out)
{
    struct cpTag *p = tags;
    for (int i = 0; i < NTAGS; i++, p++) {
        switch (p->type) {
        case TIFF_SHORT:
            if (p->count == 1) {
                uint16_t shortv;
                CopyField(p->tag, shortv);
            } else if (p->count == 2) {
                uint16_t shortv1, shortv2;
                CopyField2(p->tag, shortv1, shortv2);
            } else if (p->count == (uint16_t)-1) {
                uint16_t  shortv1;
                uint16_t *shortav;
                CopyField2(p->tag, shortv1, shortav);
            }
            break;
        case TIFF_LONG: {
            uint32_t longv;
            CopyField(p->tag, longv);
        } break;
        case TIFF_RATIONAL:
            if (p->count == 1) {
                float floatv;
                // CopyField(tag, v) replaced by following
                // Workaround a tiff lib bug:  TIFFGetField gets a very small float
                if (TIFFGetField(in, p->tag, &floatv) && floatv >= .00001) TIFFSetField(out, p->tag, floatv);
            } else if (p->count == (uint16_t)-1) {
                float *floatav;
                CopyField(p->tag, floatav);
            }
            break;
        case TIFF_ASCII: {
            char *stringv;
            CopyField(p->tag, stringv);
        } break;
        case TIFF_DOUBLE:
            if (p->count == 1) {
                double doublev;
                CopyField(p->tag, doublev);
            } else if (p->count == (uint16_t)-1) {
                double *doubleav;
                CopyField(p->tag, doubleav);
            }
            break;
        default: break;
        }
    }
}

typedef int (*copyFunc)(TIFF *in, TIFF *out, uint32_t l, uint32_t w, uint16_t samplesperpixel);
static copyFunc pickCopyFunc(TIFF *, TIFF *, uint16_t, uint16_t);

static int tiffcp(TIFF *in, TIFF *out)
{
    uint16_t   samplesperpixel, shortv;
    uint16_t   bitspersample = 0;
    copyFunc cf;
    uint32_t   w, l;

    CopyField(TIFFTAG_IMAGEWIDTH, w);
    CopyField(TIFFTAG_IMAGELENGTH, l);

    currentImageWidth = w;
    currentImageHeight = l;
    if (convert_8_to_4) {
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 4);
    } else {
        CopyField(TIFFTAG_BITSPERSAMPLE, bitspersample);
    }
    if (compression != (uint16_t)-1)
        TIFFSetField(out, TIFFTAG_COMPRESSION, compression);
    else
        CopyField(TIFFTAG_COMPRESSION, compression);
    if (compression == COMPRESSION_JPEG && jpegcolormode == JPEGCOLORMODE_RGB)
        TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_YCBCR);
    else
        CopyField(TIFFTAG_PHOTOMETRIC, shortv);
    if (fillorder != 0)
        TIFFSetField(out, TIFFTAG_FILLORDER, fillorder);
    else
        CopyField(TIFFTAG_FILLORDER, shortv);
    CopyField(TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
    /*
     * Choose tiles/strip for the output image according to
     * the command line arguments (-tiles, -strips) and the
     * structure of the input image.
     */
    if (outtiled == -1) outtiled = TIFFIsTiled(in);
    if (outtiled) {
        /*
         * Setup output file's tile width&height.  If either
         * is not specified, use either the value from the
         * input image or, if nothing is defined, use the
         * library default.
         */
        if (tilewidth == (uint32_t)-1) TIFFGetField(in, TIFFTAG_TILEWIDTH, &tilewidth);
        if (tilelength == (uint32_t)-1) TIFFGetField(in, TIFFTAG_TILELENGTH, &tilelength);
        TIFFDefaultTileSize(out, &tilewidth, &tilelength);
        TIFFSetField(out, TIFFTAG_TILEWIDTH, tilewidth);
        TIFFSetField(out, TIFFTAG_TILELENGTH, tilelength);
    } else {
        /*
         * RowsPerStrip is left unspecified: use either the
         * value from the input image or, if nothing is defined,
         * use the library default.
         */
        if (rowsperstrip == (uint32_t)-1) TIFFGetField(in, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
        rowsperstrip = TIFFDefaultStripSize(out, rowsperstrip);
        TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
    }
    if (config != (uint16_t)-1)
        TIFFSetField(out, TIFFTAG_PLANARCONFIG, config);
    else
        CopyField(TIFFTAG_PLANARCONFIG, config);
    if (g3opts != (uint32_t)-1)
        TIFFSetField(out, TIFFTAG_GROUP3OPTIONS, g3opts);
    else
        CopyField(TIFFTAG_GROUP3OPTIONS, g3opts);
    if (samplesperpixel <= 4) {
        uint16_t *tr, *tg, *tb, *ta;
        CopyField4(TIFFTAG_TRANSFERFUNCTION, tr, tg, tb, ta);
    }
    {
        uint16_t *red, *green, *blue;
        if (TIFFGetField(in, TIFFTAG_COLORMAP, &red, &green, &blue)) {
            CheckAndCorrectColormap(in, 1 << bitspersample, red, green, blue);
            TIFFSetField(out, TIFFTAG_COLORMAP, red, green, blue);
        }
    }
    /* SMinSampleValue & SMaxSampleValue */
    switch (compression) {
    case COMPRESSION_JPEG:
        TIFFSetField(out, TIFFTAG_JPEGQUALITY, quality);
        TIFFSetField(out, TIFFTAG_JPEGCOLORMODE, jpegcolormode);
        break;
    case COMPRESSION_LZW:
    case COMPRESSION_DEFLATE:
        if (predictor != (uint16_t)-1)
            TIFFSetField(out, TIFFTAG_PREDICTOR, predictor);
        else
            CopyField(TIFFTAG_PREDICTOR, predictor);
        break;
    }
    cpOtherTags(in, out);

    if (geofile || proj4_string || timeFile)
        InstallGeoTIFF(out);
    else
        CopyGeoTIFF(in, out);

    if (worldfile) ApplyWorldFile(worldfile, out);

    cf = pickCopyFunc(in, out, bitspersample, samplesperpixel);
    return (cf ? (*cf)(in, out, l, w, samplesperpixel) : FALSE);
}

/*
 * Copy Functions.
 */
#define DECLAREcpFunc(x) static int x(TIFF *in, TIFF *out, uint32_t imagelength, uint32_t imagewidth, tsample_t spp)

#define DECLAREreadFunc(x) static void x(TIFF *in, unsigned char *buf, uint32_t imagelength, uint32_t imagewidth, tsample_t spp)
typedef void (*readFunc)(TIFF *, unsigned char *, uint32_t, uint32_t, tsample_t);

#define DECLAREwriteFunc(x) static int x(TIFF *out, unsigned char *buf, uint32_t imagelength, uint32_t imagewidth, tsample_t spp)
typedef int (*writeFunc)(TIFF *, unsigned char *, uint32_t, uint32_t, tsample_t);

/*
 * Contig -> contig by scanline for rows/strip change.
 */
DECLAREcpFunc(cpContig2ContigByRow)
{
    unsigned char *buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(in));
    uint32_t         row;

    (void)imagewidth;
    (void)spp;
    for (row = 0; row < imagelength; row++) {
        if (TIFFReadScanline(in, buf, row, 0) < 0 && !ignore) goto done;
        if (TIFFWriteScanline(out, buf, row, 0) < 0) goto bad;
    }
done:
    _TIFFfree(buf);
    return (TRUE);
bad:
    _TIFFfree(buf);
    return (FALSE);
}

/*
 * Contig -> contig by scanline for rows/strip change.
 */
DECLAREcpFunc(cpContig2ContigByRow_8_to_4)
{
    unsigned char *buf_in = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(in));
    unsigned char *buf_out = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
    uint32_t         row;

    printf("Downsample\n");

    (void)imagewidth;
    (void)spp;
    for (row = 0; row < imagelength; row++) {
        int i_in, i_out_byte;

        if (TIFFReadScanline(in, buf_in, row, 0) < 0 && !ignore) goto done;

        for (i_in = 0, i_out_byte = 0; i_in < (int)imagewidth; i_in += 2, i_out_byte++) { buf_out[i_out_byte] = (buf_in[i_in] & 0xf) * 16 + (buf_in[i_in + 1] & 0xf); }

        if (TIFFWriteScanline(out, buf_out, row, 0) < 0) goto bad;
    }
done:
    _TIFFfree(buf_in);
    _TIFFfree(buf_out);
    return (TRUE);
bad:
    _TIFFfree(buf_in);
    _TIFFfree(buf_out);
    return (FALSE);
}

/*
 * Strip -> strip for change in encoding.
 */
DECLAREcpFunc(cpDecodedStrips)
{
    tsize_t        stripsize = TIFFStripSize(in);
    unsigned char *buf = (unsigned char *)_TIFFmalloc(stripsize);

    (void)imagewidth;
    (void)spp;
    if (buf) {
        tstrip_t s, ns = TIFFNumberOfStrips(in);
        uint32_t   row = 0;
        for (s = 0; s < ns; s++) {
            tsize_t cc = (row + rowsperstrip > imagelength) ? TIFFVStripSize(in, imagelength - row) : stripsize;
            if (TIFFReadEncodedStrip(in, s, buf, cc) < 0 && !ignore) break;
            if (TIFFWriteEncodedStrip(out, s, buf, cc) < 0) {
                _TIFFfree(buf);
                return (FALSE);
            }
            row += rowsperstrip;
        }
        _TIFFfree(buf);
        return (TRUE);
    }
    return (FALSE);
}

/*
 * Separate -> separate by row for rows/strip change.
 */
DECLAREcpFunc(cpSeparate2SeparateByRow)
{
    unsigned char *buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(in));
    uint32_t         row;
    tsample_t      s;

    (void)imagewidth;
    for (s = 0; s < spp; s++) {
        for (row = 0; row < imagelength; row++) {
            if (TIFFReadScanline(in, buf, row, s) < 0 && !ignore) goto done;
            if (TIFFWriteScanline(out, buf, row, s) < 0) goto bad;
        }
    }
done:
    _TIFFfree(buf);
    return (TRUE);
bad:
    _TIFFfree(buf);
    return (FALSE);
}

/*
 * Contig -> separate by row.
 */
DECLAREcpFunc(cpContig2SeparateByRow)
{
    unsigned char *inbuf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(in));
    unsigned char *outbuf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
    unsigned char *inp, *outp;
    uint32_t         n;
    uint32_t         row;
    tsample_t      s;

    /* unpack channels */
    for (s = 0; s < spp; s++) {
        for (row = 0; row < imagelength; row++) {
            if (TIFFReadScanline(in, inbuf, row, 0) < 0 && !ignore) goto done;
            inp = inbuf + s;
            outp = outbuf;
            for (n = imagewidth; n-- > 0;) {
                *outp++ = *inp;
                inp += spp;
            }
            if (TIFFWriteScanline(out, outbuf, row, s) < 0) goto bad;
        }
    }
done:
    if (inbuf) _TIFFfree(inbuf);
    if (outbuf) _TIFFfree(outbuf);
    return (TRUE);
bad:
    if (inbuf) _TIFFfree(inbuf);
    if (outbuf) _TIFFfree(outbuf);
    return (FALSE);
}

/*
 * Separate -> contig by row.
 */
DECLAREcpFunc(cpSeparate2ContigByRow)
{
    unsigned char *inbuf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(in));
    unsigned char *outbuf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
    unsigned char *inp, *outp;
    uint32_t         n;
    uint32_t         row;
    tsample_t      s;

    for (row = 0; row < imagelength; row++) {
        /* merge channels */
        for (s = 0; s < spp; s++) {
            if (TIFFReadScanline(in, inbuf, row, s) < 0 && !ignore) goto done;
            inp = inbuf;
            outp = outbuf + s;
            for (n = imagewidth; n-- > 0;) {
                *outp = *inp++;
                outp += spp;
            }
        }
        if (TIFFWriteScanline(out, outbuf, row, 0) < 0) goto bad;
    }
done:
    if (inbuf) _TIFFfree(inbuf);
    if (outbuf) _TIFFfree(outbuf);
    return (TRUE);
bad:
    if (inbuf) _TIFFfree(inbuf);
    if (outbuf) _TIFFfree(outbuf);
    return (FALSE);
}

static void cpStripToTile(unsigned char *out, unsigned char *in, uint32_t rows, uint32_t cols, int outskew, int inskew)
{
    while (rows-- > 0) {
        uint32_t j = cols;
        while (j-- > 0) *out++ = *in++;
        out += outskew;
        in += inskew;
    }
}

static void cpContigBufToSeparateBuf(unsigned char *out, unsigned char *in, uint32_t rows, uint32_t cols, int outskew, int inskew, tsample_t spp)
{
    while (rows-- > 0) {
        uint32_t j = cols;
        while (j-- > 0) *out++ = *in, in += spp;
        out += outskew;
        in += inskew;
    }
}

static void cpSeparateBufToContigBuf(unsigned char *out, unsigned char *in, uint32_t rows, uint32_t cols, int outskew, int inskew, tsample_t spp)
{
    while (rows-- > 0) {
        uint32_t j = cols;
        while (j-- > 0) *out = *in++, out += spp;
        out += outskew;
        in += inskew;
    }
}

static int cpImage(TIFF *in, TIFF *out, readFunc fin, writeFunc fout, uint32_t imagelength, uint32_t imagewidth, tsample_t spp)
{
    int            status = FALSE;
    unsigned char *buf = (unsigned char *)_TIFFmalloc(TIFFRasterScanlineSize(in) * imagelength);
    if (buf) {
        (*fin)(in, buf, imagelength, imagewidth, spp);
        status = (fout)(out, buf, imagelength, imagewidth, spp);
        _TIFFfree(buf);
    }
    return (status);
}

DECLAREreadFunc(readContigStripsIntoBuffer)
{
    tsize_t        scanlinesize = TIFFScanlineSize(in);
    unsigned char *bufp = buf;
    uint32_t         row;

    (void)imagewidth;
    (void)spp;
    for (row = 0; row < imagelength; row++) {
        if (TIFFReadScanline(in, bufp, row, 0) < 0 && !ignore) break;
        bufp += scanlinesize;
    }
}

DECLAREreadFunc(readSeparateStripsIntoBuffer)
{
    tsize_t        scanlinesize = TIFFScanlineSize(in);
    unsigned char *scanline = (unsigned char *)_TIFFmalloc(scanlinesize);

    (void)imagewidth;
    if (scanline) {
        unsigned char *bufp = buf;
        uint32_t         row;
        tsample_t      s;

        for (row = 0; row < imagelength; row++) {
            /* merge channels */
            for (s = 0; s < spp; s++) {
                unsigned char *sp = scanline;
                unsigned char *bp = bufp + s;
                tsize_t        n = scanlinesize;

                if (TIFFReadScanline(in, sp, row, s) < 0 && !ignore) goto done;
                while (n-- > 0) *bp = *bufp++, bp += spp;
            }
            bufp += scanlinesize;
        }
    done:
        _TIFFfree(scanline);
    }
}

DECLAREreadFunc(readContigTilesIntoBuffer)
{
    unsigned char *tilebuf = (unsigned char *)_TIFFmalloc(TIFFTileSize(in));
    uint32_t         imagew = TIFFScanlineSize(in);
    uint32_t         tilew = TIFFTileRowSize(in);
    int            iskew = imagew - tilew;
    unsigned char *bufp = buf;
    uint32_t         tw, tl;
    uint32_t         row;

    (void)spp;
    if (tilebuf == 0) return;
    (void)TIFFGetField(in, TIFFTAG_TILEWIDTH, &tw);
    (void)TIFFGetField(in, TIFFTAG_TILELENGTH, &tl);
    for (row = 0; row < imagelength; row += tl) {
        uint32_t nrow = (row + tl > imagelength) ? imagelength - row : tl;
        uint32_t colb = 0;
        uint32_t col;

        for (col = 0; col < imagewidth; col += tw) {
            if (TIFFReadTile(in, tilebuf, col, row, 0, 0) < 0 && !ignore) goto done;
            if (colb + tilew > imagew) {
                uint32_t width = imagew - colb;
                uint32_t oskew = tilew - width;
                cpStripToTile(bufp + colb, tilebuf, nrow, width, oskew + iskew, oskew);
            } else
                cpStripToTile(bufp + colb, tilebuf, nrow, tilew, iskew, 0);
            colb += tilew;
        }
        bufp += imagew * nrow;
    }
done:
    _TIFFfree(tilebuf);
}

DECLAREreadFunc(readSeparateTilesIntoBuffer)
{
    uint32_t         imagew = TIFFScanlineSize(in);
    uint32_t         tilew = TIFFTileRowSize(in);
    int            iskew = imagew - tilew;
    unsigned char *tilebuf = (unsigned char *)_TIFFmalloc(TIFFTileSize(in));
    unsigned char *bufp = buf;
    uint32_t         tw, tl;
    uint32_t         row;

    if (tilebuf == 0) return;
    (void)TIFFGetField(in, TIFFTAG_TILEWIDTH, &tw);
    (void)TIFFGetField(in, TIFFTAG_TILELENGTH, &tl);
    for (row = 0; row < imagelength; row += tl) {
        uint32_t nrow = (row + tl > imagelength) ? imagelength - row : tl;
        uint32_t colb = 0;
        uint32_t col;

        for (col = 0; col < imagewidth; col += tw) {
            tsample_t s;

            for (s = 0; s < spp; s++) {
                if (TIFFReadTile(in, tilebuf, col, row, 0, s) < 0 && !ignore) goto done;
                /*
                 * Tile is clipped horizontally.  Calculate
                 * visible portion and skewing factors.
                 */
                if (colb + tilew > imagew) {
                    uint32_t width = imagew - colb;
                    int    oskew = tilew - width;
                    cpSeparateBufToContigBuf(bufp + colb + s, tilebuf, nrow, width, oskew + iskew, oskew, spp);
                } else
                    cpSeparateBufToContigBuf(bufp + colb + s, tilebuf, nrow, tw, iskew, 0, spp);
            }
            colb += tilew;
        }
        bufp += imagew * nrow;
    }
done:
    _TIFFfree(tilebuf);
}

DECLAREwriteFunc(writeBufferToContigStrips)
{
    tsize_t scanline = TIFFScanlineSize(out);
    uint32_t  row;

    (void)imagewidth;
    (void)spp;
    for (row = 0; row < imagelength; row++) {
        if (TIFFWriteScanline(out, buf, row, 0) < 0) return (FALSE);
        buf += scanline;
    }
    return (TRUE);
}

DECLAREwriteFunc(writeBufferToSeparateStrips)
{
    unsigned char *obuf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
    tsample_t      s;

    if (obuf == NULL) return (0);
    for (s = 0; s < spp; s++) {
        uint32_t row;
        for (row = 0; row < imagelength; row++) {
            unsigned char *inp = buf + s;
            unsigned char *outp = obuf;
            uint32_t         n = imagewidth;

            while (n-- > 0) *outp++ = *inp, inp += spp;
            if (TIFFWriteScanline(out, obuf, row, s) < 0) {
                _TIFFfree(obuf);
                return (FALSE);
            }
        }
    }
    _TIFFfree(obuf);
    return (TRUE);
}

DECLAREwriteFunc(writeBufferToContigTiles)
{
    uint32_t         imagew = TIFFScanlineSize(out);
    uint32_t         tilew = TIFFTileRowSize(out);
    int            iskew = imagew - tilew;
    unsigned char *obuf = (unsigned char *)_TIFFmalloc(TIFFTileSize(out));
    unsigned char *bufp = buf;
    uint32_t         tl, tw;
    uint32_t         row;

    (void)spp;
    if (obuf == NULL) return (FALSE);
    (void)TIFFGetField(out, TIFFTAG_TILELENGTH, &tl);
    (void)TIFFGetField(out, TIFFTAG_TILEWIDTH, &tw);
    for (row = 0; row < imagelength; row += tilelength) {
        uint32_t nrow = (row + tl > imagelength) ? imagelength - row : tl;
        uint32_t colb = 0;
        uint32_t col;

        for (col = 0; col < imagewidth; col += tw) {
            /*
             * Tile is clipped horizontally.  Calculate
             * visible portion and skewing factors.
             */
            if (colb + tilew > imagew) {
                uint32_t width = imagew - colb;
                int    oskew = tilew - width;
                cpStripToTile(obuf, bufp + colb, nrow, width, oskew, oskew + iskew);
            } else
                cpStripToTile(obuf, bufp + colb, nrow, tilew, 0, iskew);
            if (TIFFWriteTile(out, obuf, col, row, 0, 0) < 0) {
                _TIFFfree(obuf);
                return (FALSE);
            }
            colb += tilew;
        }
        bufp += nrow * imagew;
    }
    _TIFFfree(obuf);
    return (TRUE);
}

DECLAREwriteFunc(writeBufferToSeparateTiles)
{
    uint32_t         imagew = TIFFScanlineSize(out);
    tsize_t        tilew = TIFFTileRowSize(out);
    int            iskew = imagew - tilew;
    unsigned char *obuf = (unsigned char *)_TIFFmalloc(TIFFTileSize(out));
    unsigned char *bufp = buf;
    uint32_t         tl, tw;
    uint32_t         row;

    if (obuf == NULL) return (FALSE);
    (void)TIFFGetField(out, TIFFTAG_TILELENGTH, &tl);
    (void)TIFFGetField(out, TIFFTAG_TILEWIDTH, &tw);
    for (row = 0; row < imagelength; row += tl) {
        uint32_t nrow = (row + tl > imagelength) ? imagelength - row : tl;
        uint32_t colb = 0;
        uint32_t col;

        for (col = 0; col < imagewidth; col += tw) {
            tsample_t s;
            for (s = 0; s < spp; s++) {
                /*
                 * Tile is clipped horizontally.  Calculate
                 * visible portion and skewing factors.
                 */
                if (colb + tilew > imagew) {
                    uint32_t width = imagew - colb;
                    int    oskew = tilew - width;

                    cpContigBufToSeparateBuf(obuf, bufp + colb + s, nrow, width, oskew / spp, oskew + imagew, spp);
                } else
                    cpContigBufToSeparateBuf(obuf, bufp + colb + s, nrow, tilewidth, 0, iskew, spp);
                if (TIFFWriteTile(out, obuf, col, row, 0, s) < 0) {
                    _TIFFfree(obuf);
                    return (FALSE);
                }
            }
            colb += tilew;
        }
        bufp += nrow * imagew;
    }
    _TIFFfree(obuf);
    return (TRUE);
}

/*
 * Contig strips -> contig tiles.
 */
DECLAREcpFunc(cpContigStrips2ContigTiles) { return cpImage(in, out, readContigStripsIntoBuffer, writeBufferToContigTiles, imagelength, imagewidth, spp); }

/*
 * Contig strips -> separate tiles.
 */
DECLAREcpFunc(cpContigStrips2SeparateTiles) { return cpImage(in, out, readContigStripsIntoBuffer, writeBufferToSeparateTiles, imagelength, imagewidth, spp); }

/*
 * Separate strips -> contig tiles.
 */
DECLAREcpFunc(cpSeparateStrips2ContigTiles) { return cpImage(in, out, readSeparateStripsIntoBuffer, writeBufferToContigTiles, imagelength, imagewidth, spp); }

/*
 * Separate strips -> separate tiles.
 */
DECLAREcpFunc(cpSeparateStrips2SeparateTiles) { return cpImage(in, out, readSeparateStripsIntoBuffer, writeBufferToSeparateTiles, imagelength, imagewidth, spp); }

/*
 * Contig strips -> contig tiles.
 */
DECLAREcpFunc(cpContigTiles2ContigTiles) { return cpImage(in, out, readContigTilesIntoBuffer, writeBufferToContigTiles, imagelength, imagewidth, spp); }

/*
 * Contig tiles -> separate tiles.
 */
DECLAREcpFunc(cpContigTiles2SeparateTiles) { return cpImage(in, out, readContigTilesIntoBuffer, writeBufferToSeparateTiles, imagelength, imagewidth, spp); }

/*
 * Separate tiles -> contig tiles.
 */
DECLAREcpFunc(cpSeparateTiles2ContigTiles) { return cpImage(in, out, readSeparateTilesIntoBuffer, writeBufferToContigTiles, imagelength, imagewidth, spp); }

/*
 * Separate tiles -> separate tiles (tile dimension change).
 */
DECLAREcpFunc(cpSeparateTiles2SeparateTiles) { return cpImage(in, out, readSeparateTilesIntoBuffer, writeBufferToSeparateTiles, imagelength, imagewidth, spp); }

/*
 * Contig tiles -> contig tiles (tile dimension change).
 */
DECLAREcpFunc(cpContigTiles2ContigStrips) { return cpImage(in, out, readContigTilesIntoBuffer, writeBufferToContigStrips, imagelength, imagewidth, spp); }

/*
 * Contig tiles -> separate strips.
 */
DECLAREcpFunc(cpContigTiles2SeparateStrips) { return cpImage(in, out, readContigTilesIntoBuffer, writeBufferToSeparateStrips, imagelength, imagewidth, spp); }

/*
 * Separate tiles -> contig strips.
 */
DECLAREcpFunc(cpSeparateTiles2ContigStrips) { return cpImage(in, out, readSeparateTilesIntoBuffer, writeBufferToContigStrips, imagelength, imagewidth, spp); }

/*
 * Separate tiles -> separate strips.
 */
DECLAREcpFunc(cpSeparateTiles2SeparateStrips) { return cpImage(in, out, readSeparateTilesIntoBuffer, writeBufferToSeparateStrips, imagelength, imagewidth, spp); }

/*
 * Select the appropriate copy function to use.
 */
static copyFunc pickCopyFunc(TIFF *in, TIFF *out, uint16_t bitspersample, uint16_t samplesperpixel)
{
    uint16_t shortv;
    uint32_t w, l, tw, tl;
    int    bychunk;

    (void)TIFFGetField(in, TIFFTAG_PLANARCONFIG, &shortv);
    if (shortv != config && bitspersample != 8 && samplesperpixel > 1) {
        fprintf(stderr, "%s: Can not handle different planar configuration w/ bits/sample != 8\n", TIFFFileName(in));
        return (NULL);
    }
    TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(in, TIFFTAG_IMAGELENGTH, &l);
    if (TIFFIsTiled(out)) {
        if (!TIFFGetField(in, TIFFTAG_TILEWIDTH, &tw)) tw = w;
        if (!TIFFGetField(in, TIFFTAG_TILELENGTH, &tl)) tl = l;
        bychunk = (tw == tilewidth && tl == tilelength);
    } else if (TIFFIsTiled(in)) {
        TIFFGetField(in, TIFFTAG_TILEWIDTH, &tw);
        TIFFGetField(in, TIFFTAG_TILELENGTH, &tl);
        bychunk = (tw == w && tl == rowsperstrip);
    } else {
        uint32_t irps = (uint32_t)-1L;
        TIFFGetField(in, TIFFTAG_ROWSPERSTRIP, &irps);
        bychunk = (rowsperstrip == irps);
    }
#define T                   1
#define F                   0
#define pack(a, b, c, d, e) ((long)(((a) << 11) | ((b) << 3) | ((c) << 2) | ((d) << 1) | (e)))
    switch (pack(shortv, config, TIFFIsTiled(in), TIFFIsTiled(out), bychunk)) {
        /* Strips -> Tiles */
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, F, T, F):
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, F, T, T): return cpContigStrips2ContigTiles;
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, F, T, F):
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, F, T, T): return cpContigStrips2SeparateTiles;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, F, T, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, F, T, T): return cpSeparateStrips2ContigTiles;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F, T, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F, T, T):
        return cpSeparateStrips2SeparateTiles;
        /* Tiles -> Tiles */
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, T, T, F):
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, T, T, T): return cpContigTiles2ContigTiles;
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, T, T, F):
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, T, T, T): return cpContigTiles2SeparateTiles;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, T, T, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, T, T, T): return cpSeparateTiles2ContigTiles;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T, T, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T, T, T):
        return cpSeparateTiles2SeparateTiles;
        /* Tiles -> Strips */
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, T, F, F):
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, T, F, T): return cpContigTiles2ContigStrips;
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, T, F, F):
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, T, F, T): return cpContigTiles2SeparateStrips;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, T, F, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, T, F, T): return cpSeparateTiles2ContigStrips;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T, F, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T, F, T):
        return cpSeparateTiles2SeparateStrips;
        /* Strips -> Strips */
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, F, F, F):
        if (convert_8_to_4)
            return cpContig2ContigByRow_8_to_4;
        else
            return cpContig2ContigByRow;

    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG, F, F, T):
        if (convert_8_to_4)
            return cpContig2ContigByRow_8_to_4;
        else
            return cpDecodedStrips;
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, F, F, F):
    case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE, F, F, T): return cpContig2SeparateByRow;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, F, F, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG, F, F, T): return cpSeparate2ContigByRow;
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F, F, F):
    case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F, F, T): return cpSeparate2SeparateByRow;
    }
#undef pack
#undef F
#undef T
    fprintf(stderr, "tiffcp: %s: Don't know how to copy/convert image.\n", TIFFFileName(in));
    return (NULL);
}
