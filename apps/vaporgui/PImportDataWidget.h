#pragma once

#include "PGroup.h"
#include "PSection.h"
#include "PLineItem.h"

namespace VAPoR {
    class ControlExec;
}
class DatasetImporter;

class PImportDataWidget : public PGroup {
    Q_OBJECT

public:
    PImportDataWidget(VAPoR::ControlExec* ce, DatasetImporter *di);
};
