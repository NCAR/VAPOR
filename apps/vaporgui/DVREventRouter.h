#ifndef DVREVENTROUTER_H
#define DVREVENTROUTER_H

#include <QWidget>

#include "RenderEventRouter.h"
#include <vapor/DVRParams.h>
#include <vapor/DirectVolumeRenderer.h>

namespace Ui {
class DVREventRouter;
}

class DVREventRouter : public QWidget, public RenderEventRouter {
    Q_OBJECT

public:
    explicit DVREventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    ~DVREventRouter();

    void               GetWebHelp(vector<pair<string, string>> &help) const;
    static std::string GetClassType() { return (VAPoR::DirectVolumeRenderer::GetClassType()); }
    std::string        GetType() const { return GetClassType(); }

protected:
    // Required pure virtual functions.
    void        _updateTab();
    std::string _getDescription() const;
    std::string _getSmallIconImagePath() const;
    std::string _getIconImagePath() const;

private:
    Ui::DVREventRouter *ui;
};

#endif    // DVREVENTROUTER_H
