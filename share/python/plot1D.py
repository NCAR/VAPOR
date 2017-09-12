

# Pass in the X dimension list as well, or just leave it blank
def plot1D( D, imagePath, X=None ):

  import numpy as np
  import matplotlib
  import platform
  #if (platform.system() == 'Darwin'):
  #   matplotlib.use('MacOSX')
  matplotlib.use('AGG')
	
  import matplotlib.pyplot as plt

 
  #
  # Setup Plot
  #
  # N.B. This method needs to be defined inside of plot1D. Otherwise the
  # invocation from the python interpreter in vaporgui will fail. 
  #
  # font sizes: xx-small, x-small, # small, medium, large, x-large, xx-large
  # linestyles: '-', '--', '-.', ':', 'None', ' ', ''
  # colors: 'blue', 'gree', 'red', 'cyan', 'magenta', 'yellow', 'black', 'white'
  # legend locations: 'best', 'upper right', 'upper left', 
  # 'lower left', 'right', 'center left', 'center right', 
  # 'lower center', 'upper center', 'center'
  #
  def setupPlot( title, leftLimit, rightLimit ):
    fig = plt.figure( figsize=(10,6), dpi=100, frameon=True )
    plt.title( title, color='Black', fontsize='xx-large')  
    plt.xlabel("X Dimension", color='Black', fontsize='large')
    plt.ylabel("Y Dimension", color='Black', fontsize='large')
    plt.xlim(leftLimit, rightLimit)
    plt.grid(True)

  
  plt.ioff()
  xRange = 0
  if X is None:
    for v in D.itervalues():
      if len(v) > xRange:
        xRange = len(v)
    X = range(xRange)

  setupPlot( 'Vapor Sample Plot Script', X[0], X[-1] )

  globalYMin = None
  globalYMax = None
  for k, v in D.iteritems():
    localYMin = min(v)
    localYMax = max(v)
    if globalYMin is None or globalYMin > localYMin:
      globalYMin = localYMin
    if globalYMax is None or globalYMax < localYMax:
      globalYMax = localYMax
    plt.plot(X, v, linewidth=1.0, linestyle='-', color=None, label=k) 
    # use log scale on Y axis:
    # plt.semilogy(X, v, linewidth=1.0, linestyle='-', color=None, label=k) 
  #plt.axis([X[0], X[-1], globalYMin, globalYMax])
    
  plt.legend(loc='best', frameon=True, fontsize='large') 

  #
  #plt.show()
    
  plt.savefig(imagePath)
  plt.gcf().clear()
  
#
# Test function for invoking plot1D() from command line
#
def runit():
  import os
  import numpy as np
  outputfile = os.path.join(os.path.expanduser('~'), 'foo.png')
  X = np.linspace(-np.pi, np.pi, 256,endpoint=True)
  C,S = np.cos(X) + 2, np.sin(X) + 2
  dic = {"cosine" : C, "sine" : S }
  print 'Output file: ', outputfile
  plot1D( dic, outputfile, X )

if __name__ == '__main__':
    runit()
