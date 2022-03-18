#include <QTreeWidget>
#include <QHeaderView>
#include <sstream>

#include "ErrorReporter.h"
#include "PMetadataSection.h"
#include "PSection.h"

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/DC.h>

// clang-format off

namespace {
    std::vector<std::string> xtypeLookup = { "INVALID", "FLOAT", "DOUBLE", "UINT8", "INT8", "INT32", "INT64", "TEXT" };
    std::vector<std::string> axisLookup  = { "X", "Y", "Z", "T"};
}

POpenVariableMetadataSection::POpenVariableMetadataSection()
: PSection("Metadata", { new POpenVariableMetadataTree() } ) {}

PFullVariableMetadataSection::PFullVariableMetadataSection()
: PSection("Metadata", { new POpenVariableMetadataTree() } ) {}

PCoordinateVariableMetadataSection::PCoordinateVariableMetadataSection()
: PSection("Metadata", { new POpenVariableMetadataTree() } ) {}

PVariableMetadataTree::PVariableMetadataTree()
: PWidget("", _tree = new QTreeWidget)
{
    _tree->setHeaderHidden(true);
    _tree->header()->setStretchLastSection(false);
    _tree->setColumnCount(2);
    _tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void PVariableMetadataTree::updateGUI() const
{
    VAPoR::RenderParams* rp = dynamic_cast<VAPoR::RenderParams*>(getParams());
    if (rp == nullptr) {
        MSG_ERR("Invalid RenderParams instance for acquiring variable metadata.");
        return;
    }
    VAPoR::DataMgr* dm = getDataMgr();
    if (rp == nullptr) {
        MSG_ERR("Invalid DataMgr instance for acquiring variable metadata.");
        return;
    }

    std::vector<std::string> vars;
    _gatherVariables(vars, rp, dm);
    
    std::vector<std::string> auxs = rp->GetAuxVariableNames();
    vars.insert(vars.end(), auxs.begin(), auxs.end());

    QStringList qvars;
    for (auto var : vars)
        qvars << QString::fromStdString(var);
    qvars.removeDuplicates();
    qvars.removeAll(QString(""));

    size_t ts = rp->GetCurrentTimestep();
    int rLevel = rp->GetRefinementLevel();
    int cLevel = rp->GetCompressionLevel();

    _tree->clear();
    for (auto qvar : qvars) {
        std::vector<double> range;
        dm->GetDataRange(ts, qvar.toStdString(), rLevel, cLevel, range);
        QTreeWidgetItem* varItem = new QTreeWidgetItem(_tree, {qvar});
        new QTreeWidgetItem(varItem, {"Min:", QString::number(range[0])});
        new QTreeWidgetItem(varItem, {"Max:", QString::number(range[1])});

        std::vector<size_t> dims;
        dm->GetDimLensAtLevel(qvar.toStdString(), rLevel, dims, ts);
        QString qDims = QString::number(dims[0]) + ":" + QString::number(dims[1]);
        if (dims.size()==3) qDims += ":" + QString::number(dims[2]);
        new QTreeWidgetItem(varItem, {"Dims (XYZ):", qDims});

        VAPoR::DC::DataVar dataVar;
        dm->GetDataVarInfo(qvar.toStdString(), dataVar);
        new QTreeWidgetItem(varItem, {"Units:", QString::fromStdString(dataVar.GetUnits())});
        _tree->insertTopLevelItem(0,varItem);

        _generateCoordVarInfo(varItem, dataVar);
        _generateAttributeInfo(varItem, dataVar);
    }
}

void PVariableMetadataTree::_generateCoordVarInfo(QTreeWidgetItem* parent, const VAPoR::DC::DataVar dataVar) const {
    VAPoR::DataMgr* dm = getDataMgr();
    if (dm == nullptr) {
        MSG_ERR("Invalid DataMgr instance for acquiring coordinate variable metadata.");
        return;
    }

    VAPoR::DC::Mesh mesh;
    QTreeWidgetItem* coords = new QTreeWidgetItem(parent, {"Coordinates"});

    dm->GetMesh(dataVar.GetMeshName(), mesh);
    std::vector<std::string> coordVars = mesh.GetCoordVars();
    QStringList qCoordVars;
    for (auto coordVar : coordVars)
        qCoordVars << QString::fromStdString(coordVar);
    qCoordVars.removeDuplicates();
    qCoordVars.removeAll(QString(""));
    for (auto qCoordVar : qCoordVars) {
        VAPoR::DC::CoordVar coordVar;
        if (!dm->GetCoordVarInfo(qCoordVar.toStdString(), coordVar)) continue;
        QTreeWidgetItem* coord = new QTreeWidgetItem(coords, {qCoordVar});
        std::vector<std::string> dimNames = coordVar.GetDimNames();
        QString qDimNames;
        for (auto dimName : dimNames)
            qDimNames += QString::fromStdString(dimName) + " ";
        new QTreeWidgetItem(coord, {"Spatial dims:", qDimNames});
        new QTreeWidgetItem(coord, {"Time dim:", QString::fromStdString(coordVar.GetTimeDimName())});
        new QTreeWidgetItem(coord, {"Axis:", QString::fromStdString(axisLookup[coordVar.GetAxis()])});
        new QTreeWidgetItem(coord, {"Data type:", QString::fromStdString(xtypeLookup[coordVar.GetXType()+1])});
        new QTreeWidgetItem(coord, {"Units:", QString::fromStdString(coordVar.GetUnits())});
    }
}

void PVariableMetadataTree::_generateAttributeInfo(QTreeWidgetItem* parent, const VAPoR::DC::BaseVar baseVar) const {
    QTreeWidgetItem* attrs = new QTreeWidgetItem(parent, {"Attributes"});
    std::map<std::string, VAPoR::DC::Attribute> attributes = baseVar.GetAttributes();
    std::map<std::string, VAPoR::DC::Attribute>::iterator it;
    std::stringstream s;
    for(auto const& attribute : attributes) {
        s.str(std::string());
        VAPoR::DC::XType xType = attribute.second.GetXType();
        if ( xType == VAPoR::DC::XType::INVALID ) continue;
        else if ( xType == VAPoR::DC::XType::TEXT ) {
            std::string values;
            attribute.second.GetValues(values);
            s.str(values);
        }
        else if (xType == VAPoR::DC::XType::FLOAT ||
                 xType == VAPoR::DC::XType::DOUBLE ) {
            std::vector<double> values;
            attribute.second.GetValues(values);
            copy(values.begin(), values.end(), ostream_iterator<int>(s," "));
        }
        else {
            std::vector<long> values;
            attribute.second.GetValues(values);
            copy(values.begin(), values.end(), ostream_iterator<int>(s," "));
        }
            
        new QTreeWidgetItem(attrs, {QString::fromStdString(attribute.first), QString::fromStdString(s.str())});
    }
}

POpenVariableMetadataTree::POpenVariableMetadataTree() : PVariableMetadataTree() {}

void POpenVariableMetadataTree::_gatherVariables(std::vector<std::string> &vars, VAPoR::RenderParams* rp, VAPoR::DataMgr* dm) const {
    vars.push_back( rp->GetVariableName() );
    vars.push_back( rp->GetXFieldVariableName() );
    vars.push_back( rp->GetYFieldVariableName() );
    vars.push_back( rp->GetZFieldVariableName() );
    vars.push_back( rp->GetHeightVariableName() );
    vars.push_back( rp->GetColorMapVariableName() );
}

PFullVariableMetadataTree::PFullVariableMetadataTree() : PVariableMetadataTree() {}

void PFullVariableMetadataTree::_gatherVariables(std::vector<std::string> &vars, VAPoR::RenderParams* rp, VAPoR::DataMgr* dm) const {
    vars.clear();
    std::vector<std::string> v = dm->GetDataVarNames();
    vars.insert(vars.end(),v.begin(),v.end());
}

PCoordinateVariableMetadataTree::PCoordinateVariableMetadataTree() : PVariableMetadataTree() {}

void PCoordinateVariableMetadataTree::_gatherVariables(std::vector<std::string> &vars, VAPoR::RenderParams* rp, VAPoR::DataMgr* dm) const {
    vars.clear();
    std::vector<std::string> v = dm->GetCoordVarNames();
    vars.insert(vars.end(),v.begin(),v.end());
}

// clang-format on
// clang-format on
