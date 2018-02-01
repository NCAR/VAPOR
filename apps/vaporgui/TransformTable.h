#ifndef TRANSFORMTABLE_H
#define TRANSFORMTABLE_H

#include <QObject>
#include "vapor/MyBase.h"
#include "vapor/ControlExecutive.h"
#include "ui_TransformTableGUI.h"
#include "VaporTable.h"

QT_USE_NAMESPACE

namespace VAPoR {
class Transform;
}

class RenderEventRouter;

//!
//! \class TransformTable
//! \ingroup Public_GUI
//! \brief A reusable promoted widget for transforming different data sets
//! \author Scott Pearse
//! \version 3.0
//! \date  September 2017

class TransformTable : public QWidget, public Ui_TransformTableGUI {
    Q_OBJECT

public:
    TransformTable(QWidget *parent);

    virtual ~TransformTable(){};

    virtual void Update(const std::map<string, VAPoR::Transform *> &transforms);
    virtual void Update(VAPoR::Transform *transform)
    {
        map<string, VAPoR::Transform *> tMap;
        tMap[" "] = transform;
        Update(tMap);
    }

protected slots:
    void ScaleChanged(int row, int col);
    void TranslationChanged(int row, int col);
    void RotationChanged(int row, int col);
    void OriginChanged(int row, int col);

private:
    VaporTable *_scaleTable;
    VaporTable *_translationTable;
    VaporTable *_rotationTable;
    VaporTable *_originTable;

    std::vector<std::string> _horizontalHeaders;
    std::vector<std::string> _verticalHeaders;

    map<string, VAPoR::Transform *> _transforms;

    void UpdateScales();
    void UpdateTranslations();
    void UpdateRotations();
    void UpdateOrigins();
};

#endif    // TRANSFORMTABLE_H
