def plotSine( title = "About as simple as it gets, folks" ):
    import matplotlib
    matplotlib.use('AGG')
    import matplotlib.pyplot as plt
    import numpy as np

    # Data for plotting
    t = np.arange(0.0, 2.0, 0.01)
    s = 1 + np.sin(2 * np.pi * t)

    # Note that using plt.subplots below is equivalent to using
    #   fig = plt.figure and then ax = fig.add_subplot(111)
    fig, ax = plt.subplots()
    ax.plot(t, s)

    ax.set(xlabel='time (s)', ylabel='voltage (mV)', title=title )
    ax.grid()

    fig.savefig("test.png")
    # plt.show()

    return 0


if __name__ == "__main__":
    plottest( "this is a new title" )
