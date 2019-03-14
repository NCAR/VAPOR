#include "VolumeSubtabs.h"

using namespace VAPoR;

void VolumeVariablesSubtab::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *params) {
    VolumeParams *vp = dynamic_cast<VolumeParams *>(params);
    _volumeParams = vp;
    assert(vp);

    string algorithm = vp->GetAlgorithm();
    int index = _castingModeComboBox->findText(QString::fromStdString(algorithm));

    if (index == -1) {
        _castingModeComboBox->clear();
        const vector<string> algorithms = VolumeParams::GetAlgorithmNames(VolumeParams::Type::DVR);
        for (const string &s : algorithms)
            _castingModeComboBox->addItem(QString::fromStdString(s));

        index = _castingModeComboBox->findText(QString::fromStdString(algorithm));
    }

    _castingModeComboBox->setCurrentIndex(index);

    _variablesWidget->Update(dataMgr, paramsMgr, params);
}

void VolumeVariablesSubtab::on__castingModeComboBox_currentIndexChanged(const QString &text) {
    if (!text.isEmpty())
        _volumeParams->SetAlgorithm(text.toStdString());
}
