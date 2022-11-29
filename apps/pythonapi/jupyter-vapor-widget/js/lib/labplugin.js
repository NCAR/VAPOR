import {HelloModel, HelloView} from './index';
import {VaporVisualizerModel, VaporVisualizerView, version} from './index';
import {IJupyterWidgetRegistry} from '@jupyter-widgets/base';

export const helloWidgetPlugin = {
  id: 'jupyter-vapor-widget:pluginexample',
  requires: [IJupyterWidgetRegistry],
  activate: function(app, widgets) {
      widgets.registerWidget({
          name: 'jupyter-vapor-widget-example',
          version: version,
          exports: { HelloModel, HelloView }
      });
  },
  autoStart: true
};

// export default helloWidgetPlugin;

export const vaporVisualizerWidgetPlugin = {
  id: 'jupyter-vapor-widget:plugin',
  requires: [IJupyterWidgetRegistry],
  activate: function(app, widgets) {
      widgets.registerWidget({
          name: 'jupyter-vapor-widget',
          version: version,
          exports: { VaporVisualizerModel, VaporVisualizerView }
      });
  },
  autoStart: true
};

export default {
    helloWidgetPlugin,
    vaporVisualizerWidgetPlugin
};
