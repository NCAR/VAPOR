#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include <vapor/Advection.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/OptionParser.h>
#include <vapor/VaporField.h>

#include "GridMetaData.h"
#include "FTLEHelper.h"

using namespace Wasp;
using namespace VAPoR;

void AppendDataPathtoFiles(const std::string &datapath, std::vector<std::string> &files)
{
    for (size_t index = 0; index < files.size(); index++) {
        std::string filepath = datapath + "/" + files.at(index);
        files[index] = filepath;
    }
}

void GenerateSeeds(std::vector<flow::Particle> &seeds, const std::vector<double> &overlapmin, const std::vector<double> &overlapmax, const std::vector<long> &dimensions, const double initTime)
{
    /* Create arrays that contain X, Y, and Z coordinates */
    float start[3], step[3];
    for (int i = 0; i < dimensions.size(); i++)    // for each of the X, Y, Z dimensions
    {
        if (dimensions[i] == 1)    // one seed in this dimension
        {
            start[i] = overlapmin[i] + 0.5f * (overlapmax[i] - overlapmin[i]);
            step[i] = 0.0f;
        } else    // more than one seed in this dimension
        {
            start[i] = overlapmin[i];
            step[i] = (overlapmax[i] - overlapmin[i]) / float(dimensions[i] - 1);
        }
    }
    if (dimensions.size() == 2 || dimensions.at(2) == 1)    // put default Z values
    {
        start[2] = 0.0f;
        step[2] = 0.0f;
    }

    /* Populate the list of seeds */
    float     timeVal = initTime;    // Default time value
    glm::vec3 loc;
    seeds.clear();
    long seedsZ;
    if (dimensions.size() == 2 || dimensions.at(2) == 1)
        seedsZ = 1;
    else
        seedsZ = dimensions[2];
    // Reserve enough space at the beginning for performance considerations
    seeds.reserve(seedsZ * dimensions[1] * dimensions[0]);
    for (long k = 0; k < seedsZ; k++)
        for (long j = 0; j < dimensions[1]; j++)
            for (long i = 0; i < dimensions[0]; i++) {
                loc.x = start[0] + float(i) * step[0];
                loc.y = start[1] + float(j) * step[1];
                loc.z = start[2] + float(k) * step[2];
                seeds.emplace_back(loc, timeVal);
            }
}

void AdjustOverlap(std::vector<double> &overlapmin, std::vector<double> &overlapmax, const std::vector<double> &mind, const std::vector<double> &maxd)
{
    overlapmin[0] = std::max(overlapmin[0], mind[0]);
    overlapmin[1] = std::max(overlapmin[1], mind[1]);
    overlapmin[2] = std::max(overlapmin[2], mind[2]);

    overlapmax[0] = std::min(overlapmax[0], maxd[0]);
    overlapmax[1] = std::min(overlapmax[1], maxd[1]);
    overlapmax[2] = std::min(overlapmax[2], maxd[2]);
}

struct {
    string              datapath;
    std::vector<string> fieldname;
    double              length;
    double              duration;
    std::vector<int>    dimensions;
    string              output;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"datapath", 1, "", "Path to the parent directory of the dataset"},
                                         {"fieldname", 1, "",
                                          "Colon delimited 3-element vector "
                                          "specifying field names for X, Y, Z components (u:v:w)"},
                                         {"length", 1, "", "Prescribed step length for an advection step"},
                                         {"duration", 1, "", "The duration over which the FTLE needs to be calculated"},
                                         {"dimensions", 1, "",
                                          "Colon delimited 3-element vector "
                                          "specifying Grid dimensions for FTLE calculation (X:Y:Z)"},
                                         {"output", 1, "FTLEfield.dat", "Name for the output FTLE field"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"datapath", Wasp::CvtToCPPStr, &opt.datapath, sizeof(opt.datapath)},
                                        {"fieldname", Wasp::CvtToStrVec, &opt.fieldname, sizeof(opt.fieldname)},
                                        {"length", Wasp::CvtToDouble, &opt.length, sizeof(opt.length)},
                                        {"duration", Wasp::CvtToDouble, &opt.duration, sizeof(opt.duration)},
                                        {"dimensions", Wasp::CvtToIntVec, &opt.dimensions, sizeof(opt.dimensions)},
                                        {"output", Wasp::CvtToCPPStr, &opt.output, sizeof(opt.output)},
                                        {NULL}};

const char *ProgName;

int main(int argc, char **argv)
{
    OptionParser op;
    ProgName = FileUtils::LegacyBasename(argv[0]);
    MyBase::SetErrMsgFilePtr(stderr);
    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }
    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    const std::string datapath = opt.datapath;
    // Field is not really used
    const std::string fieldx = opt.fieldname.at(0);
    const std::string fieldy = opt.fieldname.at(1);
    const std::string fieldz = opt.fieldname.at(2);
    float             length = opt.length;
    ;
    const double      duration = opt.duration;
    std::vector<long> dims;
    dims.push_back(opt.dimensions.at(0));
    dims.push_back(opt.dimensions.at(1));
    dims.push_back(opt.dimensions.at(2));
    const std::string output = opt.output;

    std::cout << "Advection for FTLE w/ : "
              << "\nData : " << datapath << "\nField : " << fieldx << " | " << fieldy << " | " << fieldz << "\nLength : " << length << "\nDuration : " << duration << "\nDimensions : " << dims.at(0)
              << " | " << dims.at(1) << " | " << dims.at(2) << std::endl;

    int res;
    // TODO : read field and variable information
    // Let's assume we have read the data and field
    const std::string filetype = "cf";
    const size_t      cache = 2000;
    const size_t      threads = 0;

    std::vector<std::string> files = FileUtils::ListFiles(datapath);
    AppendDataPathtoFiles(datapath, files);
    std::vector<std::string> fileopts;
    VAPoR::DataMgr           datamgr(filetype, cache, threads);
    res = datamgr.Initialize(files, fileopts);
    if (res < 0) {
        std::cerr << "Failed to intialize CF DataMGR" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<double> timeCoords;
    datamgr.GetTimeCoordinates(timeCoords);
    double initTime = timeCoords.at(0);
    std::cout << "Total time length : " << (timeCoords.at(timeCoords.size() - 1) - timeCoords.at(0)) << std::endl;

    // Create particles
    std::vector<flow::Particle> seeds;
    vector<double>              mind, maxd;
    vector<double>              overlapmin(3, std::numeric_limits<double>::min());
    vector<double>              overlapmax(3, std::numeric_limits<double>::max());
    res = datamgr.GetVariableExtents(0, fieldx, 0, 0, mind, maxd);
    AdjustOverlap(overlapmin, overlapmax, mind, maxd);
    res = datamgr.GetVariableExtents(0, fieldy, 0, 0, mind, maxd);
    AdjustOverlap(overlapmin, overlapmax, mind, maxd);
    res = datamgr.GetVariableExtents(0, fieldz, 0, 0, mind, maxd);
    AdjustOverlap(overlapmin, overlapmax, mind, maxd);
    // Populate seeds array
    // FTLE requires a Uniform Grid
    GenerateSeeds(seeds, overlapmin, overlapmax, dims, initTime);
    std::cout << "Will use " << seeds.size() << " seeds." << std::endl;

    flow::VaporField velocityField(3);
    velocityField.IsSteady = true;
    velocityField.AssignDataManager(&datamgr);
    velocityField.VelocityNames[0] = fieldx.c_str();
    velocityField.VelocityNames[1] = fieldy.c_str();
    velocityField.VelocityNames[2] = fieldz.c_str();

    ParamsBase::StateSave stateSave;    // = nullptr;
    VAPoR::FlowParams     params(&datamgr, &stateSave);
    params.SetIsSteady(true);
    params.SetFlowDirection(static_cast<int>(VAPoR::FlowDir::FORWARD));
    params.SetSeedGenMode(static_cast<int>(VAPoR::FlowSeedMode::RANDOM));
    params.SetRandomNumOfSeeds(1000);
    params.SetRefinementLevel(0);
    params.SetCompressionLevel(0);
    params.GetBox()->SetExtents(overlapmin, overlapmax);
    velocityField.UpdateParams(&params);

    res = velocityField.CalcDeltaTFromCurrentTimeStep(length);

    auto start = chrono::steady_clock::now();

    // Advect particles in the field
    flow::Advection advection;
    advection.UseSeedParticles(seeds);

    int advect = flow::ADVECT_HAPPENED;
    advect = advection.AdvectTillTime(&velocityField, initTime, length, (initTime + duration), flow::Advection::ADVECTION_METHOD::RK4);

    // Extract streams from the advection class
    std::vector<flow::Particle> endLocations;
    size_t                      streams = advection.GetNumberOfStreams();
    for (size_t index = 0; index < streams; index++) { endLocations.push_back(advection.GetStreamAt(index).back()); }

    /*We'll get streams as an output over here*/
    /*Future optimization : if only FTLE is required, do not calculate the streams*/
    detail::GridMetaData metaData(dims);
    std::vector<double>  FTLEfield;
    CalculateFTLE(seeds, endLocations, metaData, duration, FTLEfield);

    auto         end = chrono::steady_clock::now();
    const double nanotosec = 1e-9;
    auto         elapsed = chrono::duration_cast<chrono::nanoseconds>(end - start).count() * nanotosec;
    cout << "Elapsed time : " << elapsed << " sec." << endl;

    ofstream fout;
    fout.open(output, ios::binary);
    fout.write(reinterpret_cast<const char *>(&FTLEfield[0]), FTLEfield.size() * sizeof(double));
    fout.close();

    return 0;
}
