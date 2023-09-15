#ifndef RENDERHOLDER_H
#define RENDERHOLDER_H

#include <qobject.h>
#include <qstackedwidget.h>
#include <qpushbutton.h>
#include <qtableview.h>
#include <vapor/MyBase.h>
#include <QMessageBox>
#include <vapor/GUIStateParams.h>
#include "ui_NewRendererDialog.h"
#include "VaporTable.h"
#include <vapor/ControlExecutive.h>

QT_USE_NAMESPACE

class RendererList;
namespace VAPoR {
class ControlExec;
class ParamsMgr;
class DataStatus;
class RenderParams;
}    // namespace VAPoR

class NewRendererDialog : public QDialog, public Ui_NewRendererDialog {
    Q_OBJECT

public:
    NewRendererDialog(QWidget *parent, std::vector<string> rendererNames, std::vector<string> descriptions, std::vector<string> iconPaths, std::vector<string> smallIconPaths,
                      std::vector<bool> dim2DSupport, std::vector<bool> dim3DSupport, vector<bool> particleSupport);

    std::string GetSelectedRenderer() { return _selectedRenderer; }
    void        InitializeDataSources(VAPoR::DataStatus *dataStatus);

private slots:
    void _buttonChecked();
    void _buttonDoubleClicked();
    void _showRelevantRenderers();

private:
    void         _createButtons();
    void         _setUpImage(std::string imageName, QLabel *label);
    void         _uncheckAllButtons();
    void         _selectFirstValidRenderer();
    QPushButton *_createButton(QIcon icon, QString name, int index);

    std::vector<string>        _rendererNames;
    std::vector<string>        _descriptions;
    std::vector<string>        _iconPaths;
    std::vector<string>        _smallIconPaths;
    std::vector<bool>          _dim2DSupport;
    std::vector<bool>          _dim3DSupport;
    std::vector<bool>          _particleSupport;
    std::vector<QPushButton *> _buttons;

    VAPoR::DataStatus *_dataStatus;

    std::string  _selectedRenderer;
    QMessageBox *_msgBox;
};

class CBWidget : public QWidget, public QTableWidgetItem {
public:
    CBWidget(QWidget *parent, QString type);
};

//! \class RenderHolder
//! \ingroup Public_GUI
//! \brief A class that manages the display of Renderer parameters
//! \author Alan Norton
//! \version 3.0
//! \date April 2015

//! This is class manages a QTableWidget that indicates the currently
//! available Renderers, and a
//! QStackedWidget that displays the various parameters associated
//! with the selected renderer.
//!
class RenderHolder {
public:
    static void _showIntelDriverWarning(VAPoR::ControlExec *_controlExec, const string &rendererType);
    static void _newRendererDialogAccepted(VAPoR::ControlExec *_controlExec, NewRendererDialog *_newRendererDialog);
};

#endif    // RENDERHOLDER_H
