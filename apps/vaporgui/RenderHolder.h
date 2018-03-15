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
}    // namespace VAPoR

class NewRendererDialog : public QDialog, public Ui_NewRendererDialog {
    Q_OBJECT

public:
    NewRendererDialog(QWidget *parent, std::vector<string> rendererNames, std::vector<string> descriptions, std::vector<string> iconPaths, std::vector<string> smallIconPaths);

    std::string GetSelectedRenderer() { return _selectedRenderer; }
    void        mouseDoubleClickEvent(QMouseEvent *event)
    {
        _msgBox = new QMessageBox();
        _msgBox->setWindowTitle("Hello");
        _msgBox->setText("You Double Clicked Mouse Button");
        _msgBox->show();
    };

private slots:
    void barbChecked(bool state);
    void contourChecked(bool state);
    void imageChecked(bool state);
    void twoDDataChecked(bool state);
    void buttonChecked();

private:
    void createButtons();
    void setUpImage(std::string imageName, QLabel *label);
    void uncheckAllButtons();
    void initializeImages();
    void initializeDataSources();

    static const std::string barbDescription;
    static const std::string contourDescription;
    static const std::string imageDescription;
    static const std::string twoDDataDescription;

    std::vector<string> _rendererNames;
    std::vector<string> _descriptions;
    std::vector<string> _iconPaths;
    std::vector<string> _smallIconPaths;

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
                 const std::vector<string> &iconPaths, const std::vector<string> &smallIconPaths);

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

    GUIStateParams *getStateParams() const
    {
        assert(_controlExec != NULL);
        return ((GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    }

    void updateDupCombo();
    void makeRendererTableHeaders(vector<string> &table);
    void initializeNewRendererDialog(vector<string> datasetNames);

    // Convert name to a unique name (among renderer names)
    std::string uniqueName(std::string name);

private slots:
    void showNewRendererDialog();
    void deleteRenderer();
    void itemTextChange(QTableWidgetItem *);
    void copyInstanceTo(int);
    void activeRendererChanged(int row, int col);
    void tableValueChanged(int row, int col);

signals:
    void newRendererSignal(string vizName, string renderClass, string renderInst);
    void activeChanged(string vizName, string renderClass, string renderInst);

private:
    VAPoR::ControlExec *_controlExec;
    NewRendererDialog * _newRendererDialog;

    VaporTable *        _vaporTable;
    int                 _currentRow;
    std::vector<string> _widgetNames;

    void getRow(int row, string &renderInst, string &renderClass, string &dataSetName) const;

    void                 makeConnections();
    void                 initializeSplitter();
    string               getActiveRendererClass();
    string               getActiveRendererInst();
    void                 highlightActiveRow(int row);
    void                 changeRendererName(int row, int col);
    VAPoR::RenderParams *getRenderParamsFromCell(int row, int col);

#endif    // DOXYGEN_SKIP_THIS
};

#endif    // RENDERHOLDER_H
