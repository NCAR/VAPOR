import os
import pickle
import urllib
import requests
import collections
import datetime
import matplotlib

pickleFile = "metrics.pkl"

base  = "https://api.github.com"
owner = "NCAR"
repo  = "VAPOR"
tag   = "3.2.0"

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

def GetRepoIds( repoIds ):
    query = '/'.join([base, "repos", owner, repo, "releases"])

    #myJson = requests.get( query ).json()
    myJson = requests.get( query, auth=("sgpearse",token) ).json()

    for key in myJson:
        repoIds.append( key[ id_key ] );

def GetDownloadCount( repoId ):
    query = '/'.join([base, "repos", owner, repo, "releases", str(repoId)])
    #myJson = requests.get( query ).json()
    myJson = requests.get( query, auth=("sgpearse",token) ).json()

    assetDownloads = {}    
    for asset in myJson["assets"]:    
        assetDownloads[asset[assetName_key]] = asset[downloads_key]

    return assetDownloads

def SaveMetrics( metrics ):
    fileMode = 'wb'
    if os.path.getsize( pickleFile ) > 0:
        fileMode = 'ab'

    with open( pickleFile, fileMode ) as f:
        pickle.dump( metrics, f, pickle.HIGHEST_PROTOCOL)

def LoadMetrics():
    if os.path.getsize( pickleFile ) <= 0:
        return;

    with open( pickleFile , 'rb' ) as f:
        return pickle.load(f)
        

def PlotOSCount( myMetrics ):
    count = 0
    osCounts = dict()
    for os in operatingSystems:
        osCounts[os] = []
    
    CentOS = []
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

    print(dates)
    print(osCounts)

def main():
    requests.get(base, auth=("sgpearse",token))

    myMetrics = LoadMetrics()
    if myMetrics is None:
        myMetrics = collections.defaultdict(dict)

    date = datetime.datetime.today().strftime("%d-%m-%y %H:%M:%S")

    repoIds = []
    GetRepoIds( repoIds )

    for repoId in repoIds:
        assetDownloads = GetDownloadCount( repoId )
        for asset in assetDownloads:
            value = assetDownloads[asset]
            myMetrics[date][asset] = value

    PlotOSCount( myMetrics )

    SaveMetrics( myMetrics )
    #print(myMetrics) 

if __name__ == "__main__":
    main()
 
'''
print(os.path.basename(__file__))
f = open(os.path.basename(__file__), 'a')
f.write("foobarbaz\n")
f.close()'''
