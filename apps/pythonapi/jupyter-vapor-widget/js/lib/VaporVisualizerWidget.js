var widgets = require('@jupyter-widgets/base');
var _ = require('lodash');

// NOTE
//
// cd
// yarn run build
//
// fails to update dev install of js code and dist needs to be manually
// copied to jupyter extension dir


// Custom Model. Custom widgets models must at least provide default values
// for model attributes, including
//
//  - `_view_name`
//  - `_view_module`
//  - `_view_module_version`
//
//  - `_model_name`
//  - `_model_module`
//  - `_model_module_version` //
//  when different from the base class.

// When serialiazing the entire widget state for embedding, only values that
// differ from the defaults will be specified.
var VaporVisualizerModel = widgets.DOMWidgetModel.extend({
    defaults: _.extend(widgets.DOMWidgetModel.prototype.defaults(), {
        _model_name : 'VaporVisualizerModel',
        _view_name : 'VaporVisualizerView',
        _model_module : 'jupyter-vapor-widget',
        _view_module : 'jupyter-vapor-widget',
        _model_module_version : '1.0',
        _view_module_version : '1.0',
        value : 'Hello World!',
    })
});


// iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAADElEQVQImWNISUkBAAJcAS0OBgnuAAAAAElFTkSuQmCC


// Custom View. Renders the widget model.
var VaporVisualizerView = widgets.DOMWidgetView.extend({
    // Defines how the widget gets rendered into the DOM
    render: function() {

        this.$img = $('<img>', {src:'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAADElEQVQImWNISUkBAAJcAS0OBgnuAAAAAElFTkSuQmCC'})
        let $img = this.$img;
        $img.attr('draggable', false);
        $img.css('user-select', 'none');

        $img.attr('title', "Left Drag: Rotate\nRight Drag: Zoom\nShift Left Drag: Pan");
        $img.css('cursor', 'move');


        let model = this.model;
        // $img.mousedown(function(){
        //     $img.css('border', '5px solid orange');
        //     model.set('mouseDown', true);
        //     model.save_changes();
        //     });
        // $(window).mouseup(function(){
        //     $img.css('border', 'none');
        //     model.set('mouseDown', false);
        //     model.save_changes();
        // });

        $img.mousedown(this.mousedown.bind(this));
        $img.mouseup(this.mouseup.bind(this));
        $img.mousemove(this.mousemove.bind(this));
        $img.click(function(e){e.preventDefault();});
        $img.bind('contextmenu', function(e){return false;});


        $el = $(this.el)
        $el.append($img);

        // this.$valueDisplay = $('<div>');
        // $el.append($('<h3>Value</h3>'));
        // $el.append(this.$valueDisplay);

        // this.$log = $('<div>');
        // $el.append($('<h3>Log</h3>'));
        // $el.append(this.$log);



        // Observe changes in the value traitlet in Python, and define
        // a custom callback.
        // this.model.on('change:value', this.value_changed, this);
        this.model.on('change:imageData', this.image_changed, this);

        this.model.on('change:value', this.updateValueDisplay, this);

        this.image_changed();
        this.updateValueDisplay();
    },

    logClear: function() {
        // this.$log.empty();
    },
    log: function(s) {
        // this.$log.append($('<p>').text(s));
    },

    updateValueDisplay: function() {
        // this.$valueDisplay.html(this.model.get('value'))
    },

    image_changed: function() {
        // this.logClear();

        // this.log("image_change");
        // this.el.textContent = this.model.get('value');
        let $img = this.$img;
        let model = this.model;

        width = model.get('resolution')[0]
        height = model.get('resolution')[1]

        $img.attr("width", width);
        $img.attr("height", height);


        if (model.get('imageFormat') !== "") {
            // this.log("do set data");
            $img.attr("src", "data:image/" + model.get('imageFormat') + ";base64," + model.get('imageData'));
        }

        $(this.p).html("JQ test: " + this.model.get('value') + " w: " + width + "<br> h: " + height);
    },

    mousedown: function(e) {
        e.preventDefault();

        var key = e.which;
        if (e.originalEvent.shiftKey && key == 1)
            key = 2;

        // this.$img.css('border', '2px solid blue');
        this.model.set('mouseButton', key);
        this.model.set('mouseDown', true);
        this.model.save_changes();
    },
    mouseup: function(e) {
        e.preventDefault();
        this.$img.css('border', 'none');
        this.model.set('mouseDown', false);
        this.model.save_changes();
    },
    mousemove: function(e) {
        this.logClear();
        // this.$img.css('border', '2px solid orange');
        var offset = this.$img.offset();
        var nx = e.pageX - offset.left;
        var ny = e.pageY - offset.top;
        if (this.$img.width() !== 0 && this.$img.height() !== 0) {
            nx /= this.$img.width();
            ny /= this.$img.height();
        } else {
            nx = 0;
            ny = 0;
        }
        this.log("POS " + nx + ", " + ny);
        this.model.set('mousePos', [nx, ny]);
        this.model.save_changes();
    },
});


module.exports = {
    VaporVisualizerModel: VaporVisualizerModel,
    VaporVisualizerView: VaporVisualizerView
};
