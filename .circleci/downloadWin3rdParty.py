import gdown 
url = "https://drive.google.com/a/ucar.edu/uc?id=1fzZ-mbY4Cek1TRsaKm79a08Od5gNCogk"
output = "2019-Aug-Win32.zip"
gdown.download(url, output, quiet=False)

import zipfile
with zipfile.ZipFile("2019-Aug-Win32.zip", 'r') as zip_ref:
    zip_ref.extractall('C:\\')
