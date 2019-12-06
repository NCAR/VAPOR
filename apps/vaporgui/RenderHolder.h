#ifndef RENDERHOLDER_H
#define RENDERHOLDER_H

#include <qobject.h>
#include <qstackedwidget.h>
#include <qpushbutton.h>
#include <qtableview.h>
#include <vapor/MyBase.h>
#include <QMessageBox>
#include "GUIStateParams.h"
#include "ui_LeftPanel.h"
#include "ui_NewRendererDialog.h"
#include "VaporTable.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
class ParamsMgr;
class DataStatus;
}    // namespace VAPoR

/*class VPushButtonWithDoubleClick : public QPushButton {
    Q_OBJECT
    using QPushButton::QPushButton;
    void mouseDoubleClickEvent(QMouseEvent * e) {
        emit doubleClicked();
    }

signals:
    void doubleClicked();
};*/

class NewRendererDialog : public QDialog, public Ui_NewRendererDialog {
    Q_OBJECT

public:
    NewRendererDialog(QWidget *parent, std::vector<string> rendererNames, std::vector<string> descriptions, std::vector<string> iconPaths, std::vector<string> smallIconPaths,
                      std::vector<bool> dim2DSupport, std::vector<bool> dim3DSupport);

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
class RenderHolder : public QWidget, public Ui_LeftPanel {
    Q_OBJECT

public:
    //! Constructor:
    //!
    //! \param[in] widgets vector of Widgets to be added
    //! \param[in] widgetNames vector of widget names
    //! \param[in] descriptions vector of Descriptions of the renderers to
    //! be displayed
    //! \param[in] iconPath Full path to a raster file containing a
    //! large icon, or an empty string if none exists
    //! \param[in] smallIconPath Full path to a raster file containing a
    //! small icon (thumbnail), or an empty string if none exists
    //
    RenderHolder(QWidget *parent, VAPoR::ControlExec *ce, const std::vector<QWidget *> &widgets, const std::vector<string> &widgetNames, const std::vector<string> &descriptions,
                 const std::vector<string> &iconPaths, const std::vector<string> &smallIconPaths, const std::vector<bool> &dim2DSupport, const std::vector<bool> &dim3DSupport);

    virtual ~RenderHolder() {}

    //! Make the tableWidget match the currently displayed RenderParams
    //!
    void Update();

    //! Specify the name of the page to be displayed of the stackedWidget.
    //! \param[in] name name of widget
    //!
    //! \sa RenderHolder()
    //
    void SetCurrentWidget(string name);

#ifndef DOXYGEN_SKIP_THIS
private:
    RenderHolder() {}

    GUIStateParams *_getStateParams() const
    {
        VAssert(_controlExec != NULL);
        return ((GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    }

    void _updateDupCombo();
    void _makeRendererTableHeaders(vector<string> &table);
    void _initializeNewRendererDialog(vector<string> datasetNames);

    // Convert name to a unique name (among renderer names)
    std::string uniqueName(std::string name);

    void _showIntelDriverWarning(const string &rendererType);

private slots:
    void _showNewRendererDialog();
    void _deleteRenderer();
    void _itemTextChange(QTableWidgetItem *);
    void _copyInstanceTo(int);
    void _activeRendererChanged(int row, int col);
    void _tableValueChanged(int row, int col);

signals:
    void newRendererSignal(string vizName, string renderClass, string renderInst);
    void activeChanged(string vizName, string renderClass, string renderInst);

private:
    VAPoR::ControlExec *_controlExec;
    NewRendererDialog * _newRendererDialog;

    VaporTable *        _vaporTable;
    int                 _currentRow;
    std::vector<string> _widgetNames;

    int _getRow(string renderInst) const;

    void _getRowInfo(int row, string &renderInst, string &renderClass, string &dataSetName) const;

    void   _makeConnections();
    void   _initializeSplitter();
    string _getActiveRendererClass();
    string _getActiveRendererInst();
    // void highlightActiveRow(int row);
    void                 _changeRendererName(int row, int col);
    VAPoR::RenderParams *_getRenderParamsFromCell(int row, int col);

#endif    // DOXYGEN_SKIP_THIS
};

#endif    // RENDERHOLDER_H
