#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <qlineedit.h>
#include <qcolordialog.h>
#include <vector>
#include <string>
#include "vapor/HelloParams.h"
#include "vapor/HelloRenderer.h"
#include "HelloEventRouter.h"
#include "helloSubtabs.h"
#include <qscrollarea.h>

using namespace VAPoR;

HelloEventRouter::HelloEventRouter(QWidget *parent, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, HelloParams::GetClassType())
{
    // Create all the sub tabs, put them in their own scroll areas
    QScrollArea *qsapp = new QScrollArea(this);
    QScrollArea *qsvar = new QScrollArea(this);
    QScrollArea *qsgeom = new QScrollArea(this);
    qsgeom->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    _variables = new HelloVariablesSubtab(this);

    // Add the variables tab as a sub-tab of the hello renderer tab
    qsvar->setWidget(_variables);
    addTab(qsvar, "Variables");

    // Create and add the appearance sub-tab.  This is a customized internal class of the HelloEventRouter

    _appearance = new HelloAppearanceGUI(this, this, ce);
    qsapp->setWidget(_appearance);

    addTab(qsapp, "Appearance");

    // Create and add the geometry subtab. This is also an internal class in HelloEventRouter
    _geometry = new HelloLayoutGUI(this, this, ce);
    qsgeom->setWidget(_geometry);
    addTab(qsgeom, "Geometry");
}

// Destructor does nothing
HelloEventRouter::~HelloEventRouter()
{
    delete _variables;
    delete _appearance;
}
/**********************************************************
 * Whenever a new Hellotab is created all the connections between widgets and slots are made
 ************************************************************/
void HelloEventRouter::hookUpTab()
{
    // Appearance tab:
    // The thickness lineEdit (like all lineEdits) sets the textChanged flag when its value changes,
    // and connects to the returnPressed when user presses enter over the widget
    connect(_appearance->thicknessEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setHelloTextChanged(const QString &)));
    connect(_appearance->thicknessEdit, SIGNAL(returnPressed()), this, SLOT(helloReturnPressed()));

    // The colorSelectButton invokes the selectColor slot when pressed.
    connect(_appearance->colorSelectButton, SIGNAL(pressed()), this, SLOT(selectColor()));

    // Geometry tab:
    // This tab has 6 lineEdits.  Each lineEdit is connected to both the setHelloTextChanged slot
    // and the helloReturnPressed slot.
    connect(_geometry->xCoord1Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setHelloTextChanged(const QString &)));
    connect(_geometry->xCoord1Edit, SIGNAL(returnPressed()), this, SLOT(helloReturnPressed()));
    connect(_geometry->xCoord2Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setHelloTextChanged(const QString &)));
    connect(_geometry->xCoord2Edit, SIGNAL(returnPressed()), this, SLOT(helloReturnPressed()));
    connect(_geometry->yCoord1Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setHelloTextChanged(const QString &)));
    connect(_geometry->yCoord1Edit, SIGNAL(returnPressed()), this, SLOT(helloReturnPressed()));
    connect(_geometry->yCoord2Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setHelloTextChanged(const QString &)));
    connect(_geometry->yCoord2Edit, SIGNAL(returnPressed()), this, SLOT(helloReturnPressed()));
    connect(_geometry->zCoord1Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setHelloTextChanged(const QString &)));
    connect(_geometry->zCoord1Edit, SIGNAL(returnPressed()), this, SLOT(helloReturnPressed()));
    connect(_geometry->zCoord2Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setHelloTextChanged(const QString &)));
    connect(_geometry->zCoord2Edit, SIGNAL(returnPressed()), this, SLOT(helloReturnPressed()));

    // Note that the variablesTab manages its own signal and slot connections.
}

void HelloEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Hello Overview", "http://www.vapor.ucar.edu/docs/vapor-gui-help/hello#HelloOverview"));

    help.push_back(make_pair("Renderer control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/renderer-instances"));

    help.push_back(make_pair("Data accuracy control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/refinement-and-lod-control"));

    help.push_back(make_pair("Hello geometry options", "http://www.vapor.ucar.edu/docs/vapor-gui-help/hello#HelloGeometry"));

    help.push_back(make_pair("Hello Appearance settings", "<>"
                                                          "http://www.vapor.ucar.edu/docs/vapor-gui-help/hello#HelloAppearance"));
}

// The updateTab() method is invoked whenever it is necessary to display the current values in the tab.

void HelloEventRouter::_updateTab()
{
    // The variable tab updates itself:
    _variables->Update(_controlExec->GetDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());
    _appearance->updateTab();
    _geometry->updateTab();
}

// Whenever the user presses enter, call confirmText
// This method takes all the values in the gui and sets them into the Params.
void HelloEventRouter::_confirmText()
{
    HelloParams *hParams = (HelloParams *)GetActiveParams();

    // Obtain the point coordinates from the gui,
    vector<double> pt1;
    vector<double> pt2;
    pt1.push_back(_geometry->xCoord1Edit->text().toDouble());
    pt1.push_back(_geometry->yCoord1Edit->text().toDouble());
    pt1.push_back(_geometry->zCoord1Edit->text().toDouble());
    pt2.push_back(_geometry->xCoord2Edit->text().toDouble());
    pt2.push_back(_geometry->yCoord2Edit->text().toDouble());
    pt2.push_back(_geometry->zCoord2Edit->text().toDouble());

    // Set the two point coordinates in the Params.
    hParams->SetPoint1(pt1);
    hParams->SetPoint2(pt2);

    // Obtain the thickness value from the gui.
    float thickness = _appearance->thicknessEdit->text().toFloat();
    // Set the thickness in the params
    hParams->SetLineThickness((double)thickness);
}

/*********************************************************************************
 * Slots associated with HelloTab.
 * Any EventRouter needs the first two slots here.
 * Plus, each EventRouter needs a slot to respond to events in every non-text
 * widget, e.g. combo boxes, sliders, pushbuttons
 *********************************************************************************/

// Whenever a lineEdit changes, set the textChanged flag.
void HelloEventRouter::setHelloTextChanged(const QString &) { SetTextChanged(true); }

// Whenever the user pressed enter over a text box, we call confirmText.
void HelloEventRouter::helloReturnPressed(void) { confirmText(); }

// The selectColor tab launches a color selecter whenever the user
// clicks the color button.  This is somewhat more involved than
// most slots that respond to button clicks, because this slot
// needs to launch a color selector, and the new color needs
// to color a visible box in the GUI.

void HelloEventRouter::selectColor()
{
    // Start with confirmText() in case the user has changed some text values.
    // This should be in every slot that responds to a button click
    confirmText();

    // obtain the current palette of the color button
    QPalette pal(_appearance->colorBox->palette());

    // Launch the Qt color selector.
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);

    // Was a new color specified?
    if (!newColor.isValid()) return;

    // Apply the new color to the colorBox in the appearance sub-tab
    pal.setColor(QPalette::Base, newColor);
    _appearance->colorBox->setPalette(pal);

    // Obtain the active params, in which we will set the color
    HelloParams *aParams = (HelloParams *)GetActiveParams();

    // Get the color as an rgb triple from the QColor
    qreal rgb[3];
    newColor.getRgbF(rgb, rgb + 1, rgb + 2);

    // convert to float
    float rgbf[3];
    for (int i = 0; i < 3; i++) rgbf[i] = rgb[i];

    // The following two lines are typical:  We issue a SetValue to the Params, and
    // then force a render.

    // Set the color in the params.  Note that the UndoRedo automatically captures this event.
    aParams->SetConstantColor(rgbf);
}
void HelloEventRouter::HelloLayoutGUI::_updateTab()
{
    HelloParams *hParams = (HelloParams *)GetActiveParams();

    // Apply the start and end point coordinates to the text boxes in the geometry sub-tab
    // First obtain the point coordinates from the Params:
    vector<double> startPoint, endPoint;
    startPoint = hParams->GetPoint1();
    endPoint = hParams->GetPoint2();
    // Then set the lineEdits to the point coordinates.
    xCoord1Edit->setText(QString::number(startPoint[0]));
    yCoord1Edit->setText(QString::number(startPoint[1]));
    zCoord1Edit->setText(QString::number(startPoint[2]));
    xCoord2Edit->setText(QString::number(endPoint[0]));
    yCoord2Edit->setText(QString::number(endPoint[1]));
    zCoord2Edit->setText(QString::number(endPoint[2]));
}
void HelloEventRouter::HelloAppearanceGUI::_updateTab()
{
    HelloParams *hParams = (HelloParams *)GetActiveParams();

    // Setup the constant color box.  Obtain the color from the params
    float rgb[3];
    hParams->GetConstantColor(rgb);
    QColor newColor;
    newColor.setRgbF((qreal)rgb[0], (qreal)rgb[1], (qreal)rgb[2]);

    // Obtain the palette of the colorBox in the gui
    QPalette pal(colorBox->palette());

    // Set the palette base color to the value from the params
    pal.setColor(QPalette::Base, newColor);

    // Reset the colorBox's palette.
    colorBox->setPalette(pal);
    // The thickness lineEdit has its value set to the value in the Params.
    thicknessEdit->setText(QString::number(hParams->GetLineThickness()));
}
