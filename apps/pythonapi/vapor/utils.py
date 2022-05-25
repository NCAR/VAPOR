from .session import *
import matplotlib.pyplot as plt
import numpy as np
import functools

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