#ifndef IMAGEEVENTROUTER_H
#define IMAGEEVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include <GL/glew.h>
#include <vapor/ImageParams.h>
#include <vapor/ImageRenderer.h>
#include <RenderEventRouter.h>
#include <VariablesWidget.h>
#include <ImageSubtabs.h>

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;

class ImageEventRouter : public QTabWidget, public RenderEventRouter {

    Q_OBJECT

  public:
    ImageEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    ~ImageEventRouter();

    void GetWebHelp(vector<pair<string, string>> &help) const;

    static std::string GetClassType() {
        return (VAPoR::ImageRenderer::GetClassType());
    }
    std::string GetType() const {
        return GetClassType();
    }

  protected:
    void _updateTab();

  private:
    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events will always
    //! affect the combo boxes and other widgets in the tab, and it would be confusing
    //! if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    ImageVariablesSubtab *_variables;
    ImageGeometrySubtab *_geometry;
    ImageAppearanceSubtab *_appearance;
};

} // namespace VAPoR

#endif
