# outFile   : string
# varNames  : list of strings
# sequences : list of list of floats. Has the Y values of the plot
# xValues   : list of floats.         Has the X values of the plot
# xLabel    : a single string.        Has the X label.
# yLabel    : a single string.        Has the Y label.
def plotSequences( outFile, varNames, sequences, xValues, xLabel, yLabel ):
    import matplotlib
    matplotlib.use('AGG')
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(8, 6))
    if len(varNames) > 0:
        for i in range(len(varNames)):
            ax.plot( xValues, sequences[i], '-x', label=varNames[i] )
        ax.legend(loc='best')

    ax.set(xlabel=xLabel, ylabel=yLabel, title="Vapor Plot Utility")
    #ax.set_xticks( xValues ) # give it fixed xticks

    fig.savefig( outFile )
    plt.close( fig )

    return 0
