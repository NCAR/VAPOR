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
    plt.show()

    return 0


# outFile   : string
# varNames  : list of strings
# sequences : list of list of floats. Has the Y values of the plot
# xValues   : list of floats.         has the X values of the plot
def plotSequences( outFile, varNames, sequences, xValues ):
    import matplotlib
    matplotlib.use('AGG')
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(10, 6))
    if len(varNames) > 0:
        for i in range(len(varNames)):
            ax.plot( xValues, sequences[i], label=varNames[i] )
        ax.legend(loc='best')

    ax.set(xlabel='Samples', ylabel='Values', title="Vapor Plot Utility")
    #ax.set_xticks( xValues ) # give it fixed xticks

    fig.savefig( outFile )

    return 0

if __name__ == "__main__":
    outFile = "/tmp/a.png"
    varNames = ["var1", "var2"]
    xValues  = (0, 1, 2, 3, 4)
    sequences = []
    seq = []
    for i in range(4):
        seq.append( float(i+5) )
    seq.append( float('nan') )
    sequences.append( seq )
    seq = []
    for i in range(5):
        seq.append( float(10-i) )
    sequences.append( seq )
        
 #   plotSine( outFile, varNames )
    plotSequences( outFile, varNames, sequences, xValues )
