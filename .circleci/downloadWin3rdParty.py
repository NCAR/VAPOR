import gdown 
url = "https://drive.google.com/a/ucar.edu/uc?id=1sE-mTQCa8MCUOVEgw-6ksT80ebmDVsu4"
output = "2019-Aug-Win32.zip"
gdown.download(url, output, quiet=False)

import zipfile
with zipfile.ZipFile("2019-Aug-Win32.zip", 'r') as zip_ref:
    zip_ref.extractall('C:\\')
