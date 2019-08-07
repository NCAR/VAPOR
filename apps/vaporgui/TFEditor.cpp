#include "TFEditor.h"
#include <QBoxLayout>
#include <QLabel>
#include "TFFunctionEditor.h"

TFEditor::TFEditor()
{
    addTab(new QWidget(this), "Transfer Function");
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    _tab()->setLayout(layout);
    layout->addWidget(new QLabel("Testing"));
    layout->addWidget(tff = new TFFunctionEditor);
}

void TFEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    tff->Update(dataMgr, paramsMgr, rParams);
}

QWidget *TFEditor::_tab() const
{
    return this->widget(0);
}

