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
            ax.plot( xValues, sequences[i], '-x', label=varNames[i] )
        ax.legend(loc='best')

    ax.set(xlabel='Samples', ylabel='Values', title="Vapor Plot Utility")
    #ax.set_xticks( xValues ) # give it fixed xticks

    fig.savefig( outFile )
    plt.close( fig )

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
