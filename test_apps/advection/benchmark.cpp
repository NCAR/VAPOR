#include <chrono>
#include <iostream>
#include <vector>
#include <random>

#ifdef OMP_POSSIBLE
    #include <omp.h>
#endif

#include <vapor/Advection.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/OptionParser.h>
#include <vapor/VaporField.h>

using namespace Wasp;
using namespace VAPoR;

void GenerateSeeds(std::vector<flow::Particle> &seeds, const std::vector<double> &extents, const long numOfSeeds, const double initTime)
{
    const unsigned int                    randSeed = 32;
    std::mt19937                          gen(randSeed);    // Standard mersenne_twister_engine
    std::uniform_real_distribution<float> distX(extents.at(0), extents.at(1));
    std::uniform_real_distribution<float> distY(extents.at(2), extents.at(3));
    std::uniform_real_distribution<float> distZ(extents.at(4), extents.at(5));
    seeds.resize(numOfSeeds);
    for (long i = 0; i < numOfSeeds; i++) {
        seeds[i].location.x = distX(gen);
        seeds[i].location.y = distY(gen);
        seeds[i].location.z = distZ(gen);
        seeds[i].time = initTime;
    }
}

int PrintStreams(flow::Advection &advector)
{
    size_t numStreams = advector.GetNumberOfStreams();
    for (size_t index = 0; index < numStreams; index++) {
        auto &stream = advector.GetStreamAt(index);
        {
            auto &start = stream.front();
            auto &end = stream.back();
            std::cout << "[" << index << "] Size : " << stream.size() << " "
                      << "{(" << start.location.x << ", " << start.location.y << ", " << start.location.z << ") : "
                      << "(" << end.location.x << ", " << end.location.y << ", " << end.location.z << ")}" << std::endl;
        }
    }
    return 0;
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
    int                 seeds;
    int                 steps;
    double              length;
    std::vector<double> minu;
    std::vector<double> maxu;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"datapath", 1, "", "Path to the parent directory of the dataset"},
                                         {"fieldname", 1, "",
                                          "Colon delimited 3-element vector "
                                          "specifying field names for X, Y, Z components (u:v:w)"},
                                         {"seeds", 1, "100", "Number of random seeds for the benchmark"},
                                         {"steps", 1, "100", "Maximum number of advection steps for particles"},
                                         {"length", 1, "", "Prescribed step length for an advection step"},
                                         {"minu", 1, "",
                                          "Colon delimited 3-element vector "
                                          "specifying rake min extents in user coordinates (X0:Y0:Z0)"},
                                         {"maxu", 1, "",
                                          "Colon delimited 3-element vector "
                                          "specifying rake max extents in user coordinates (X1:Y1:Z1)"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"datapath", Wasp::CvtToCPPStr, &opt.datapath, sizeof(opt.datapath)},
                                        {"fieldname", Wasp::CvtToStrVec, &opt.fieldname, sizeof(opt.fieldname)},
                                        {"seeds", Wasp::CvtToInt, &opt.seeds, sizeof(opt.seeds)},
                                        {"steps", Wasp::CvtToInt, &opt.steps, sizeof(opt.steps)},
                                        {"length", Wasp::CvtToDouble, &opt.length, sizeof(opt.length)},
                                        {"minu", Wasp::CvtToDoubleVec, &opt.minu, sizeof(opt.minu)},
                                        {"maxu", Wasp::CvtToDoubleVec, &opt.maxu, sizeof(opt.maxu)},
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

    const std::string fieldx = opt.fieldname.at(0);
    const std::string fieldy = opt.fieldname.at(1);
    const std::string fieldz = opt.fieldname.at(2);

    const long numSeeds = opt.seeds;
    const long steps = opt.steps;
    float      length = opt.length;
#ifdef OMP_POSSIBLE
    std::cout << "Available # of threads : " << omp_get_max_threads() << std::endl;
#endif
    std::cout << "Advection w/ : "
              << "\nData : " << datapath << "\nField : " << fieldx << "| " << fieldy << " | " << fieldz << "\nSeeds : " << numSeeds << "\nSteps : " << steps << "\nLength : " << length << std::endl;

    int res;

    const std::string filetype = "cf";
    const size_t      cache = 2000;
    const size_t      threads = 0;

    std::vector<std::string> files;
    files.push_back(datapath);
    std::vector<std::string> fileopts;
    VAPoR::DataMgr           datamgr(filetype, cache, threads);
    res = datamgr.Initialize(files, fileopts);
    if (res < 0) {
        std::cerr << "Failed to intialize CF DataMGR" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<double> timeCoords;
    datamgr.GetTimeCoordinates(timeCoords);
    double              initTime = timeCoords.at(0);
    std::vector<double> extents;
    if (opt.minu.size() == 0 || opt.maxu.size() == 0) {
        // use extents of entire data
        vector<double> mind, maxd;
        vector<double> overlapmin(3, std::numeric_limits<double>::min());
        vector<double> overlapmax(3, std::numeric_limits<double>::max());
        res = datamgr.GetVariableExtents(0, fieldx, 0, 0, mind, maxd);
        AdjustOverlap(overlapmin, overlapmax, mind, maxd);
        res = datamgr.GetVariableExtents(0, fieldy, 0, 0, mind, maxd);
        AdjustOverlap(overlapmin, overlapmax, mind, maxd);
        res = datamgr.GetVariableExtents(0, fieldz, 0, 0, mind, maxd);
        AdjustOverlap(overlapmin, overlapmax, mind, maxd);
        extents.push_back(mind.at(0));
        extents.push_back(maxd.at(0));
        extents.push_back(mind.at(1));
        extents.push_back(maxd.at(1));
        extents.push_back(mind.at(2));
        extents.push_back(maxd.at(2));
    } else {
        extents.push_back(opt.minu.at(0));
        extents.push_back(opt.maxu.at(0));
        extents.push_back(opt.minu.at(1));
        extents.push_back(opt.maxu.at(1));
        extents.push_back(opt.minu.at(2));
        extents.push_back(opt.maxu.at(2));
    }
    // Populate seeds array
    // Random for now, VAPOR lets users configure
    std::vector<flow::Particle> seeds;
    GenerateSeeds(seeds, extents, numSeeds, initTime);

    flow::VaporField velocityField(3);
    velocityField.IsSteady = true;
    velocityField.AssignDataManager(&datamgr);
    velocityField.VelocityNames[0] = fieldx.c_str();
    velocityField.VelocityNames[1] = fieldy.c_str();
    velocityField.VelocityNames[2] = fieldz.c_str();

    ParamsBase::StateSave stateSave;
    VAPoR::FlowParams     params(&datamgr, &stateSave);
    params.SetIsSteady(true);
    params.SetSteadyNumOfSteps(steps);
    params.SetFlowDirection(static_cast<int>(VAPoR::FlowDir::FORWARD));
    params.SetSeedGenMode(static_cast<int>(VAPoR::FlowSeedMode::RANDOM));
    params.SetRefinementLevel(0);
    params.SetCompressionLevel(0);
    velocityField.UpdateParams(&params);
    params.GetBox()->SetExtents(opt.minu, opt.maxu);
    res = velocityField.CalcDeltaTFromCurrentTimeStep(length);

    // Advect particles in the field
    flow::Advection advection;
    advection.UseSeedParticles(seeds);

    auto start = chrono::steady_clock::now();

    int advect = flow::ADVECT_HAPPENED;
    for (size_t step = advection.GetMaxNumOfPart() - 1; step < steps && advect == flow::ADVECT_HAPPENED; step++) {
        advect = advection.AdvectOneStep(&velocityField, length, flow::Advection::ADVECTION_METHOD::RK4);
    }

    // advect = advection.AdvectSteps(&velocityField, length, steps, flow::Advection::ADVECTION_METHOD::RK4);

    auto         end = chrono::steady_clock::now();
    const double nanotosec = 1e-9;
    auto         elapsed = chrono::duration_cast<chrono::nanoseconds>(end - start).count() * nanotosec;
    cout << "Elapsed time : " << elapsed << " sec." << endl;

    // PrintStreams(advection);

    return 0;
}
