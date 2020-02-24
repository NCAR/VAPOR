import gdown 
url = "https://drive.google.com/a/ucar.edu/uc?id=1sRlE06jSVrCScrt546G4UtI3hj93xV2W"
output = "2019-Aug-Win32.zip"
gdown.download(url, output, quiet=False)

import zipfile
with zipfile.ZipFile("2019-Aug-Win32.zip", 'r') as zip_ref:
    zip_ref.extractall(r'C:\')
