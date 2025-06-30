from ..session import *
import matplotlib.pyplot as plt
import numpy as np
import functools
import bqplot as bq
import ipywidgets as widgets
from ..widget import VaporVisualizerWidget

link.include('vapor/Histo.h')

def GetSamples(session: Session, renderer: Renderer, varName: str):
    c_win = link.std.string()
    c_dat = link.std.string()
    c_typ = link.std.string()
    session.ce.RenderLookup(renderer.id, c_win, c_dat, c_typ)
    dataset = session.GetDataset(c_dat)
    return link.Histo(0).GetDataSamples(varName, dataset._wrappedInstance, renderer._params)


def GetMatPlotLibHistogram(session: Session, renderer: Renderer, **kwargs):
    """
    Shows a histogram for a given renderer.
    Wraps matplotlib.pyplot.hist(GetSamples(...))
    """
    varName = renderer._params.GetActualColorMapVariableName()
    samples = GetSamples(session, renderer, varName)

    kwargs.setdefault("range", renderer.GetPrimaryTransferFunction().GetMinMaxMapValue())
    kwargs.setdefault("bins", 128)

    # density=True is supposed to create a probability density however it is broken so this is an alternative
    weights = np.ones_like(samples) / float(len(samples))

    plt.hist(samples, weights=weights, **kwargs)
    return plt

def ShowMatPlotLibHistogram(session: Session, renderer: Renderer, **kwargs):
    GetMatPlotLibHistogram(session, renderer, **kwargs).show()


def transferFunctionWidget(ses, ren, preserveOpacities = True, nControlPoints=5):
    # Histogram setup
    data = np.array(GetSamples(ses, ren, ren._params.GetActualColorMapVariableName()))
    hist_values, edges = np.histogram(data, bins=50)
    hist_values = (hist_values / max(hist_values)) * 100 # Scale so y-axis is between 0 and 100. This mitigate's bqplot's auto-rounding
    hist_values = [np.ceil(y) for y in hist_values] # Round up to handle vanishing values
    bin_centers = (edges[:-1] + edges[1:]) / 2

    x_sc = bq.LinearScale()
    y_sc = bq.LinearScale(min=0, max=100)
    bars = bq.Bars(x=bin_centers, y=hist_values, scales={'x': x_sc, 'y': y_sc})

    # Create control points
    if(preserveOpacities):
        init_points = ren.GetPrimaryTransferFunction().GetOpacityControlPoints()
    else:
        init_points = [[x, 100] for x in np.linspace(min(data), max(data), nControlPoints)]
    # Or this
    control_x, control_y = zip(*init_points)
    ren.GetPrimaryTransferFunction().SetOpacityControlPoints([[x, float(y)/100] for x, y in init_points])

    # Set up visualizer widget
    viz = VaporVisualizerWidget(ses)
    viz_box = widgets.Box([viz])

    # Interactive scatter points
    scatter = bq.Scatter(
        x=list(control_x),
        y=list(control_y),
        scales={'x': x_sc, 'y': y_sc},
        enable_move=True,
        colors=['white'],
        stroke='black',
        default_size=64,
    )

    # Lines connecting control points
    lines = bq.Lines(
        x=list(control_x),
        y=list(control_y),
        scales={'x': x_sc, 'y': y_sc},
        stroke='white',
        stroke_width=2
    )

    # Update figure and tf on drag
    def on_drag(change):
        control_points = list(zip(scatter.x, scatter.y))
        control_points.sort(key=lambda p: p[0]) # Sort by x-values in case control points cross each other
        clamped_control_points = [(x, min(100, max(0, y))) for x, y in control_points] # Keep y within [0, 100]
        xs, ys = zip(*clamped_control_points)
        lines.x = xs
        lines.y = ys
        scatter.x = xs
        scatter.y = ys

        # Update transfer function
        ren.GetPrimaryTransferFunction().SetOpacityControlPoints([[x, float(y)/100] for x, y in control_points])
        viz_box.children = [VaporVisualizerWidget(ses)] # Refresh widget for live update


    # Slider for histogram range
    x_min, x_max = np.min(data), np.max(data)
    range_slider = widgets.FloatRangeSlider(
        value=[x_min, x_max],
        min=x_min,
        max=x_max,
        step=(x_max - x_min) / 100,
        description='Range:',
        continuous_update=True,
        readout=False
    )

    def update_histogram_range(change):
        x_low, x_high = range_slider.value

        # Remake histogram with given range bounds
        filtered_data = data[(data >= x_low) & (data <= x_high)]
        hist_y, new_edges = np.histogram(filtered_data, bins=50, range=(x_low, x_high))
        hist_y = (hist_y / hist_y.max()) * 100
        new_centers = (new_edges[:-1] + new_edges[1:]) / 2

        bars.x = new_centers
        bars.y = [np.ceil(y) for y in hist_y]

        # Keep control points in same relative position
        new_cp = [[x, y] for x, y in zip(np.linspace(min(filtered_data), max(filtered_data), nControlPoints), scatter.y)]
        xs, ys = zip(*new_cp)
        scatter.x = xs
        scatter.y = ys
        lines.x = xs
        lines.y = ys

        # Update x-axis
        x_sc.min = x_low
        x_sc.max = x_high

        # Update tf with new cp
        ren.GetPrimaryTransferFunction().SetOpacityControlPoints([[x, float(y)/100] for x, y in new_cp])


    # Set up interactivity
    scatter.observe(on_drag, names=['x', 'y'])
    range_slider.observe(update_histogram_range, names='value')


    # Create UI layout
    fig = bq.Figure(
        marks=[bars, lines, scatter], 
        scales={'x': x_sc, 'y': y_sc}, 
        animation_duration=0, 
        title=str(ren._params.GetVariableName())
        )

    # Layout Style
    fig.layout = widgets.Layout(
        width='99%',
        height='300px',
    )
    range_slider.layout = widgets.Layout(
        width='90%'
    )
    tf_controls = widgets.VBox([fig, range_slider], layout=widgets.Layout(
        width='500px',
        align_items='center'
    ))
    UI = widgets.HBox([viz_box, tf_controls], layout=widgets.Layout(
        align_items='flex-start'
    ))
    return UI