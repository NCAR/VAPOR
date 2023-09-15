#ifndef COPYREGIONWIDGET_H
#define COPYREGIONWIDGET_H

#include <vapor/Box.h>
#include "VSection.h"
#include <QPushButton>
#include <QComboBox>

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class CopyRegionWidget : public VSectionGroup {
    Q_OBJECT

public:
    CopyRegionWidget(QWidget *parent = 0);

    ~CopyRegionWidget();

    QString name() const { return "CopyRegionWidget"; }
    QString includeFile() const { return "CopyRegionWidget.h"; }
    QString group() const { return tr("A widget for copying one renderer's region to another"); }
    QString toolTip() const
    {
        return tr("A widget for copying one renderer's "
                  "region to another");
    }
    QString whatsThis() const
    {
        return tr("This widget contains all widgets "
                  "necessary for copying renderer regions");
    }
    bool         isContainer() const { return true; }
    virtual void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

signals:
    void valueChanged();

protected slots:
    virtual void copyRegion();

protected:
    QPushButton *copyButton;
    QComboBox *  copyCombo;

    void updateCopyCombo();

    // Configures a box to have equal minimum and maximum
    // extents along an axis, if the box is planar.  If not,
    // the function returns.
    void _configurePlanarBox(const VAPoR::Box *myBox, std::vector<double> *myMin, std::vector<double> *myMax) const;

    VAPoR::ParamsMgr *   _paramsMgr;
    VAPoR::RenderParams *_rParams;

    std::vector<std::string>           _dataSetNames;
    std::map<std::string, std::string> _visNames;
    std::map<std::string, std::string> _renTypeNames;
};

#endif    // COPYREGIONWIDGET_H
