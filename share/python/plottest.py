# outFile is a string
# varNames is a list of strings

def plotSine( outFile, varNames ):
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
    if len(varNames) > 0 :
        ax.plot(t, s, label=varNames[0] )
        ax.legend()
    else:
        ax.plot( t, s )

    ax.set(xlabel='time (s)', ylabel='voltage (mV)', title="About as simple as it gets, folks")
    ax.grid()

    fig.savefig( outFile )
    # plt.show()

    return 0


if __name__ == "__main__":
    outFile = "/tmp/a.png"
    varNames = []
    varNames.append( "variable 1" );
    plotSine( outFile, varNames )
