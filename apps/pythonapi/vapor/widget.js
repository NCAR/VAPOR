import $ from './widget-jquery';

function render({ model, el }) {
    let $root = $("<div>");
    $root.addClass("vapor-jupyter-widget");

    let $img = $('<img>', {src:'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAADElEQVQImWNISUkBAAJcAS0OBgnuAAAAAElFTkSuQmCC'});
    for (var i = 0; i < 2; i++) $root.append($img.clone().attr("id", ""+i));
    let $imgs = $root.find('img');
    var img_i = 0;

    $imgs.on("load", e=>{
        let $e = $(e.target);
        $e.parent().append($e);
    });
    $imgs.on("dragstart", e=>e.preventDefault());

    function loadImage() {
        let width = model.get('resolution')[0];
        let height = model.get('resolution')[1];
        $root.width(width);
        $root.height(height);
        // console.log(`Image data received: ${width}x${height}`);
        if (model.get('imageFormat') !== "") {
            $($imgs[img_i]).attr("src", "data:image/" + model.get('imageFormat') + ";base64," + model.get('imageData'));
            img_i = (img_i+1)%2;
        }
    }

    model.on("change:imageData", loadImage);
    if (model.get('imageData'))
        loadImage();

    $root.on("mousedown", e => {
        e.preventDefault();
        var key = e.which;
        if (e.originalEvent.shiftKey && key == 1)
            key = 2;

        model.set('mouseButton', key);
        model.set('mouseDown', true);
        model.save_changes();
    });

    $root.on("mouseup", e => {
        e.preventDefault();
        model.set('mouseDown', false);
        model.save_changes();
    });

    $root.on("mousemove", e => {
        e.preventDefault();
        // this.$img.css('border', '2px solid orange');
        var offset = $root.offset();
        var nx = e.pageX - offset.left;
        var ny = e.pageY - offset.top;
        // console.log(`offset=(${offset.left}, ${offset.top})  e.page=(${e.pageX}, ${e.pageY})  n=(${nx}, ${ny})`)
        if ($root.width() !== 0 && $root.height() !== 0) {
            nx /= $root.width();
            ny /= $root.height();
        } else {
            nx = 0;
            ny = 0;
        }
        // console.log("mouse move " + nx + ", " + ny);
        model.set('mousePos', [nx, ny]);
        model.save_changes();
    });

    el.appendChild($root.get(0));
}
export default { render };