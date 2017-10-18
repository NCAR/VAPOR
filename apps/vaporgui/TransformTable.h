#ifndef TRANSFORMTABLE_H
#define TRANSFORMTABLE_H

#include <QObject>
#include "vapor/MyBase.h"
#include "vapor/ControlExecutive.h"
#include "ui_TransformTableGUI.h"

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
        tMap[""] = transform;
        Update(tMap);
    }

protected slots:
    void scaleChanged(int row, int col);
    void rotationChanged(int row, int col);
    void translationChanged(int row, int col);

private:
    void updateTransformTable(QTableWidget *table, string target, vector<double> values, int row);

    void updateScales(const std::map<string, VAPoR::Transform *> &transforms);
    void updateTranslations(const std::map<string, VAPoR::Transform *> &transforms);
    void updateRotations(const std::map<string, VAPoR::Transform *> &transforms);

    void setScales(string dataset, vector<double> s);
    void setTranslations(string dataset, vector<double> t);
    void setRotations(string dataset, vector<double> r);
};

#endif    // TRANSFORMTABLE_H
