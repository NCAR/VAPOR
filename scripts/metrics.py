import os
import sys
import pickle
import urllib
import requests
import collections
import datetime
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import matplotlib.axes

pickleFile = "metrics.pkl"

base  = "https://api.github.com"
owner = "NCAR"
repo  = "VAPOR"
tag   = "3.2.0"

username = ''
token = ''

id_key = "id"
downloads_key = "download_count"
assetName_key = "name"

operatingSystems = [
    'Darwin', 
    'win64', 
    'CentOS', 
    'Ubuntu',
    'Other'
]

def GetRepoIds( releaseIds ):
    query = '/'.join([base, "repos", owner, repo, "releases"])

    r = requests.get( query, auth=(username, token) )
    if r.status_code != 200:
        print("\nRequest to https://api.github.com failed.  Have you set your username and token in this script?\n")
        sys.exit(1)
    myJson = r.json()

    for key in myJson:
        releaseIds.append( key[ id_key ] );

def GetDownloadCount( releaseId ):
    query = '/'.join([base, "repos", owner, repo, "releases", str(releaseId)])

    r = requests.get( query, auth=(username, token) )
    if r.status_code != 200:
        print("\nRequest to https://api.github.com failed.  Have you set your username and token in this script?\n")
        sys.exit(1)
    myJson = r.json()

    assetDownloads = {}    
    for asset in myJson["assets"]:    
        assetDownloads[asset[assetName_key]] = asset[downloads_key]

    return assetDownloads

def SaveMetrics( metrics ):
    # Create pickle file if one does not exist
    if not os.path.exists( pickleFile ):
        open( pickleFile, 'w' ).close()

    with open( pickleFile, 'wb' ) as f:
        pickle.dump( metrics, f, pickle.HIGHEST_PROTOCOL)

def LoadMetrics():
    if not os.path.exists( pickleFile ):
        return;
    if os.path.getsize( pickleFile ) <= 0:
        return;

    with open( pickleFile , 'rb' ) as f:
        return pickle.load(f)
        

def PlotOSCount( myMetrics ):
    count = 0
    osCounts = dict()
    for os in operatingSystems:
        osCounts[os] = []
    
    dates = []
    for date in myMetrics:
        dates.append(date)
        for os in operatingSystems:
            osCounts[os].append(0);

        for asset in myMetrics[date]:
            numDownloaded = myMetrics[date][asset]
            for os in operatingSystems:
                if os in asset:
                    osCounts[os][-1] += numDownloaded

    fig, ax = plt.subplots()
    locator = ticker.MaxNLocator(nbins=8)
    ax.xaxis.set_major_locator(locator)

    totalDownloads = 0
    for os in operatingSystems:
        ax.plot(dates, osCounts[os], label=os)
        totalDownloads += osCounts[os][-1]

    print("Total downloads: " + str(totalDownloads))
    print(osCounts)

    plt.title("Total downloads: " + str(totalDownloads) + "\nCollected on " + dates[-1])
    #plt.legend(bbox_to_anchor=(.85, 1), loc='upper left', borderaxespad=0.)
    plt.legend()
    fig.autofmt_xdate()
    
    plt.show()

def main():
    requests.get(base, auth=("sgpearse",token))

    myMetrics = LoadMetrics()
    if myMetrics is None:
        myMetrics = collections.defaultdict(dict)

    date = datetime.datetime.today().strftime("%Y-%b-%d %H:%M:%S")

    releaseIds = []
    GetRepoIds( releaseIds )

    for releaseId in releaseIds:
        assetDownloads = GetDownloadCount( releaseId )
        for asset in assetDownloads:
            value = assetDownloads[asset]
            myMetrics[date][asset] = value

    SaveMetrics( myMetrics )

    PlotOSCount( myMetrics )

if __name__ == "__main__":
    main()
