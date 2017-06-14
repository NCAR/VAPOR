#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>
#include <string>
#include <iomanip>
#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/Metadata.h>
#include <vapor/MetadataSpherical.h>
#include <vapor/TransferFunctionLite.h>
#ifdef WIN32
#pragma warning(disable : 4996)
#endif
using namespace Wasp;
using namespace VAPoR;

//
// asciitf2vtf:
//    This is one of the commandline applications for VAPOR.
//
// Modified in April, 2011.
//    Added the ability for the program to use NCL colormaps as input.
//	Kendall Southwick
//

struct opt_t {
    char *omap;
    char *cmap;
    char *type;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"cmap", 1, "", "Path to ascii file containing color map"},
    {"omap", 1, "", "Path to ascii file containing opacity map"},
    {"type", 1, "vapor", "Type pf color maps being used, defaults to vapor"},
    {"help", 0, "", "Print this message and exit"},
    {"quiet", 0, "", "Operate quietly"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"cmap", Wasp::CvtToString, &opt.cmap, sizeof(opt.cmap)},
    {"omap", Wasp::CvtToString, &opt.omap, sizeof(opt.omap)},
    {"type", Wasp::CvtToString, &opt.type, sizeof(opt.type)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
    {NULL}};

const char *ProgName;

void Usage(OptionParser &op, const char *msg) {

    if (msg) {
        cerr << ProgName << " : " << msg << endl;
    }
    cerr << "Usage: " << ProgName << " [options] (-cmap cmap.txt | -omap omap.txt) file.tf3" << endl;
    op.PrintOptionHelp(stderr);
}

void ErrMsgCBHandler(const char *msg, int) {
    cerr << ProgName << " : " << msg << endl;
}

//
// This function will process the VAPOR color map files.
//

int ProcessVAPORCMAP(TransferFunctionLite *transFunct) {
    int RetVal = 0;
    vector<float> pvec, hvec, svec, vvec;
    float point, h, s, v;

    ColorMap *cmap = transFunct->getColormap();
    cmap->clear();

    FILE *fp = fopen(opt.cmap, "r");
    if (!fp) {
        MyBase::SetErrMsg("fopen(%s) : %M", opt.cmap);
        exit(1);
    }

    const char *format = "%f %f %f %f";

    while ((RetVal = fscanf(fp, format, &point, &h, &s, &v)) == 4) {
        pvec.push_back(point);
        hvec.push_back(h);
        svec.push_back(s);
        vvec.push_back(v);
    }

    if (vvec.size() <= 0) {
        MyBase::SetErrMsg("Error parsing file %s , no data.", opt.cmap);
        exit(1);
    }

    vector<float> tmpvec = pvec;
    sort(tmpvec.begin(), tmpvec.end());

    // Apparently we need to set the min and max values in both
    // the transfer function and colormap class. Setting the
    // cmap bounds is needed so that addControlPointAt() will
    // correctly normalize the data value.

    transFunct->setMinColorMapValue(tmpvec[0]);
    transFunct->setMaxColorMapValue(tmpvec[tmpvec.size() - 1]);
    cmap->minValue(tmpvec[0]);
    cmap->maxValue(tmpvec[tmpvec.size() - 1]);

    ColorMap::Color color;
    for (int i = 0; i < pvec.size(); i++) {
        color.hue(hvec[i]);
        color.sat(svec[i]);
        color.val(vvec[i]);
        //	cmap->addNormControlPoint(pvec[i],  color);
        cmap->addControlPointAt(pvec[i], color);
    }
    //
    // Ugh. now we need to normalize the data bounds.
    //
    cmap->minValue(0.0);
    cmap->maxValue(1.0);

    if (ferror(fp)) {
        MyBase::SetErrMsg("Error parsing file %s", opt.cmap);
        exit(1);
    }
    fclose(fp);

    return (RetVal);
} // End of ProcessVAPORCMAP.

//
// This function will process the NCL color map files.
//

int ProcessNCLCMAP(TransferFunctionLite *transFunct) {
    int RetVal = 0;
    int numColors = 0;
    int fileSize;
    int rc;
    int last_i = 0;
    float THRESHOLD = 0.10;
    float SLOPE_THRESHOLD = 0.010;
    float last_hsv[] = {0.0, 0.0, 0.0};
    float rgb[] = {0.0, 0.0, 0.0};
    float hsv[] = {0.0, 0.0, 0.0};
    float comp_vals[] = {0.0, 0.0, 0.0};
    vector<float> pvec, hvec, svec, vvec;
    char *readLine;
    bool header_flag = false;
    bool h_slope_flag, slope_flag, interval_flag, delta_flag;

    ColorMap *cmap = transFunct->getColormap();
    cmap->clear();

    ifstream nclFile(opt.cmap, ios::in);
    if (!nclFile) {
        MyBase::SetErrMsg("Unable to open file %s: %M", opt.cmap);
        exit(1);
    }

    //
    // Start parsing the file.
    // There needs to be a line must be of the form
    // "ncolors=##"  where ## is a number.
    // The header ends with a lines of the form
    // # r    b   g
    // There may a different number of spaces between the
    // # and the r.
    //

    nclFile.seekg(0, ios::end);
    fileSize = nclFile.tellg();
    nclFile.seekg(0, ios::beg);
    readLine = (char *)malloc(fileSize * sizeof(char));
    char *indexChar;

    while (!header_flag && !nclFile.eof()) {
        nclFile.getline(readLine, fileSize);
        indexChar = NULL;
        if (readLine[0] == '#') {
            indexChar = strstr(readLine, "r   g   b");
            if (indexChar != NULL) {
                header_flag = true;
            } // end of headers.
        }     // comment line.
        else {
            indexChar = strstr(readLine, "ncolors=");
            if (indexChar != NULL) {
                rc = sscanf(indexChar + 8, "%d", &numColors);
            } // have ncolors.
        }     // non comment line.
    }         // End of while.

    if (nclFile.eof() || numColors == 0 || !header_flag) {
        MyBase::SetErrMsg("Formating error in file %s", opt.cmap);
        exit(1);
    }

    //
    // Since it is hard to have more the ten color control points
    // in VAPOR, we need to trim the input down.
    //

    for (int i = 0; ((i < numColors) && !nclFile.eof()); i++) {
        nclFile.getline(readLine, fileSize);
        if (strlen(readLine) > 0) {
            rc = sscanf(readLine, "%f %f %f", &rgb[0], &rgb[1], &rgb[2]);
            if (rc != 3) {
                MyBase::SetErrMsg("Formating error in file %s", opt.cmap);
                exit(1);
            }
            h_slope_flag = slope_flag = interval_flag = delta_flag = false;
            rgb[0] = rgb[0] / 255.0;
            rgb[1] = rgb[1] / 255.0;
            rgb[2] = rgb[2] / 255.0;
            transFunct->rgbToHsv(rgb, hsv);
            if (i == 0) {
                comp_vals[0] = hsv[0];
                comp_vals[1] = hsv[1];
                comp_vals[2] = hsv[2];
                last_hsv[0] = hsv[0];
                last_hsv[1] = hsv[1];
                last_hsv[2] = hsv[2];
                last_i = -1;
            }

            if ((abs(hsv[0] - comp_vals[0]) >= THRESHOLD) ||
                (abs(hsv[1] - comp_vals[1]) >= THRESHOLD) ||
                (abs(hsv[2] - comp_vals[2]) >= THRESHOLD)) {
                delta_flag = true;
            }
            if ((abs((comp_vals[2] - hsv[2]) / (i - last_i) - (last_hsv[2] - hsv[2])) >= SLOPE_THRESHOLD) ||
                (abs((comp_vals[1] - hsv[1]) / (i - last_i) - (last_hsv[1] - hsv[1])) >= SLOPE_THRESHOLD)) {
                slope_flag = true;
            }
            if ((i - last_i) > (0.07 * numColors)) {
                interval_flag = true;
            }

            if (slope_flag || interval_flag || (i == 0)) {
                comp_vals[0] = hsv[0];
                comp_vals[1] = hsv[1];
                comp_vals[2] = hsv[2];
                last_i = i;
                pvec.push_back(((float)(i)) / ((float)(numColors)));
                hvec.push_back(hsv[0]);
                svec.push_back(hsv[1]);
                vvec.push_back(hsv[2]);
            }
            last_hsv[0] = hsv[0];
            last_hsv[1] = hsv[1];
            last_hsv[2] = hsv[2];
        } // End if strlen.
    }     // End for.

    if (vvec.size() <= 0) {
        MyBase::SetErrMsg("Error parsing file %s , no data.", opt.cmap);
        exit(1);
    }

    vector<float> tmpvec = pvec;
    sort(tmpvec.begin(), tmpvec.end());

    // Apparently we need to set the min and max values in both
    // the transfer function and colormap class. Setting the
    // cmap bounds is needed so that addControlPointAt() will
    // correctly normalize the data value.

    transFunct->setMinColorMapValue(tmpvec[0]);
    transFunct->setMaxColorMapValue(tmpvec[tmpvec.size() - 1]);
    cmap->minValue(tmpvec[0]);
    cmap->maxValue(tmpvec[tmpvec.size() - 1]);

    ColorMap::Color color;
    for (int i = 0; i < pvec.size(); i++) {
        color.hue(hvec[i]);
        color.sat(svec[i]);
        color.val(vvec[i]);
        //	cmap->addNormControlPoint(pvec[i],  color);
        cmap->addControlPointAt(pvec[i], color);
    }

    //
    // Ugh. now we need to normalize the data bounds.
    //
    cmap->minValue(0.0);
    cmap->maxValue(1.0);

    nclFile.close();

    return (RetVal);
} // End of ProcessNCLCMAP.

//
// This functions directs how the color maps is t be processed based on the
// type of color maps, form the -type option.
//

int ProcessCMAP(TransferFunctionLite *transFunct, char *cmapType) {
    int RetVal;

    if (strcmp(cmapType, "vapor") == 0) {
        RetVal = ProcessVAPORCMAP(transFunct);
    } else if (strcmp(cmapType, "VAPOR") == 0) {
        RetVal = ProcessVAPORCMAP(transFunct);
    } else if (strcmp(cmapType, "ncl") == 0) {
        RetVal = ProcessNCLCMAP(transFunct);
    } else if (strcmp(cmapType, "NCL") == 0) {
        RetVal = ProcessNCLCMAP(transFunct);
    } else {
        RetVal = 1;
        MyBase::SetErrMsg("Invalid color map type.");
    }

    return (RetVal);
} // End of ProcessCMAP.

int main(int argc, char **argv) {

    OptionParser op;

    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << OptionParser::GetErrMsg();
        exit(1);
    }

    MyBase::SetErrMsgCB(ErrMsgCBHandler);

    if (opt.help) {
        Usage(op, NULL);
        exit(0);
    }

    if (argc != 2) {
        Usage(op, "Wrong number of arguments");
        exit(1);
    }

    if ((strlen(opt.cmap) == 0) && (strlen(opt.omap) == 0)) {
        Usage(op, "Wrong number of arguments");
        exit(1);
    }

    TransferFunctionLite tf(8);
    if (MyBase::GetErrCode() != 0)
        exit(1);

    tf.setMinColorMapValue(0.0);
    tf.setMaxColorMapValue(1.0);
    tf.setMinOpacMapValue(0.0);
    tf.setMaxOpacMapValue(1.0);

    string vtffile(argv[1]);

    int rc;

    if (strlen(opt.cmap) != 0) {
        ProcessCMAP(&tf, opt.type);
    }

    if (strlen(opt.omap) != 0) {
        float point, o;
        vector<float> pvec, ovec;

        OpacityMap *omap = tf.getOpacityMap(0);
        omap->clear();

        FILE *fp = fopen(opt.omap, "r");
        if (!fp) {
            MyBase::SetErrMsg("fopen(%s) : %M", opt.omap);
            exit(1);
        }

        const char *format = "%f %f";

        while ((rc = fscanf(fp, format, &point, &o)) == 2) {
            pvec.push_back(point);
            ovec.push_back(o);
        }

        vector<float> tmpvec = pvec;
        sort(tmpvec.begin(), tmpvec.end());
        tf.setMinOpacMapValue(tmpvec[0]);
        tf.setMaxOpacMapValue(tmpvec[tmpvec.size() - 1]);
        omap->minValue(tmpvec[0]);
        omap->maxValue(tmpvec[tmpvec.size() - 1]);

        for (int i = 0; i < pvec.size(); i++) {
            //			omap->addNormControlPoint(pvec[i],  ovec[i]);
            omap->addControlPoint(pvec[i], ovec[i]);
        }
        omap->minValue(0.0);
        omap->maxValue(1.0);

        if (ferror(fp)) {
            MyBase::SetErrMsg("Error parsing file %s", opt.omap);
            exit(1);
        }
        fclose(fp);
    } // End of if omap.

    //
    // Write output file.
    //

    ofstream fileout;
    fileout.open(vtffile.c_str());
    if (!fileout) {
        MyBase::SetErrMsg(
            "Can't open file \"%s\" for writing", vtffile.c_str());
        exit(1);
    }
    if (!(tf.saveToFile(fileout)))
        exit(1);

    exit(0);
} // End of Main.
