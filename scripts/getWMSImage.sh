#!/bin/bash
#


# defaults
minLon=""
maxLon=""
minLat=""
maxLat=""
xres=1024
yres=768
styles="default"
version="1.1"
imageFile=""
imageFormat="image/tiff"
tempFile="tmpImage"
layer="bmng200406"
host="http://data.worldwind.arc.nasa.gov/wms"
amp="&"
decLLRegEx=^[\-+]?[0-9]+[\.]?[0-9]*$
transparent="FALSE"
debugMode=0
map=""
compression="-compress none"
depth=""


printUsage() {
      echo "Usage: " $0 "{optional parameters} minLon minLat maxLon maxLat"
      echo "Optional parameters:"
      echo "    -r xres yres"
      echo "          resolution of requested image; default is " ${xres}"x"${yres}
      echo "    -o imageFilename"
      echo "          name for the requested image file; default is named after requested image layer"
      echo "    -m map_name"
      echo "          Requests a predefined map from a well-known server (overrides any expert options)."
      echo "          Available map names:"
      echo "              \"BMNG\"         (NASA BlueMarble, the default)"
      echo "              \"landsat\"      (Landsat imagery)"
      echo "              \"USstates\"     (US state boundaries)"
      echo "              \"UScounties\"   (US state and county boundaries)"
      echo "              \"world\"        (world political boundaries)"
      echo "              \"rivers\"       (major rivers)"
      echo "    -t"
      echo "          request a transparent background"
      echo ""
      echo "Expert-only parameters (see documentation):"
      echo "    -s URL"
      echo "          URL for WMS server"
      echo "    -l layerName"
      echo "          arbitrary image-layer name to fetch"
      echo "    -f format"
      echo "          image format; default is \"image/tiff\""
      echo "    -z"
      echo "          compress the resultant geotiff file"
      echo "          (may not work on all platforms)"
      echo "    -d"
      echo "          debug mode; do not delete temporary files"
}


realValGT() {
    a=`bc <<EOF
      $1 > $2
EOF
`
    echo $a
}

realRange() {
    a=`bc <<EOF
       $2 - $1
EOF
`
    echo $a
}

realSub() {
    a=`bc <<EOF
       $1 - $2
EOF
`
    echo $a
}

##########################

if [ $# -lt 4 ]
then
    printUsage
    exit 1
fi

# Make sure we have the utilities we'll need...
fetchProg=""
nosave=`which curl 2>/dev/null`
if  [ $? -eq 0 ]; then
    fetchProg="curl -w HTTP_RESPONSE:%{http_code}\n -L -o"
fi

nosave=`which wget 2>/dev/null`
if  [ $? -eq 0 ]; then
    fetchProg="wget -O"
fi

if [ "$fetchProg" = "" ]; then
    echo "Could not find \"wget\" or \"curl\"; needed to retrieve requested images."
    exit 1
fi

nosave=`which tiff2geotiff 2>/dev/null`
if ! [ $? -eq 0 ]; then
    echo "Could not find \"tiff2geotiff\"; needed to convert requested image into geotiff."
    exit 1
fi


# parse command-line options...
#
while [ $# -gt 4 ]
do
  case $1 in

  -r) if ! ( [[ $2 =~ ^[0-9]+$ ]] && [[ $3 =~ ^[0-9]+$ ]]) ; then
          echo "bad resolution values given: " $2 " x " $3
          exit 1
      fi
      xres=$2
      yres=$3
      shift; shift
      ;;

  -s) host=$2
      shift
      ;;

  -o) imageFile=$2
      shift
      ;;

  -l) layer=$2
      shift
      ;;

  -m) map=$2
      shift
      ;;

  -f) imageFormat=$2
      shift
      ;;

  -d) debugMode=1
      ;;

  -t) transparent="TRUE"
      ;;

  -z) compression="-compress lzw"
      ;;

  -8) depth="-depth 8"
      ;;

  *)
      printUsage
      exit 0
  esac
  shift
done

# remaining parameters form a possible bounding box?
#
for i in $1 $2 $3 $4
do
    if ! [[ $i =~ ${decLLRegEx} ]] ; then
        echo $i " is not a valid decimal lon/lat string"
        exit 1
    fi
done
minLon=$1
minLat=$2
maxLon=$3
maxLat=$4

# further test bounds for sanity...
#
if [ `realValGT ${minLon} ${maxLon}` -eq 1 ]
then
    echo "Invalid bounding box: minLon > maxLon"
    exit 1
fi
if [ `realValGT ${minLon} 180` -eq 1 ] || [ `realValGT ${maxLon} 180` -eq 1 ]
then
  minLon=`realSub ${minLon} 180`
  maxLon=`realSub ${maxLon} 180`
  echo "min/max lon remapped to: " ${minLon} ${maxLon}
fi 
if [ `realValGT -180 ${minLon}` -eq 1 ] || [ `realValGT ${maxLon} 180` -eq 1 ] 
then
    echo "Invalid bounding box:"
    echo "  longitudes must range from -180 to 180, or 0 to 360"
    exit 1
fi

if [ `realValGT -90 ${minLat}` -eq 1 ] || [ `realValGT ${maxLat} 90` -eq 1 ] || \
   [ `realValGT ${minLat} ${maxLat}` -eq 1 ]
then
    echo "Invalid bounding box:"
    echo "  latitudes must range from -90 to 90, with minLat < maxLat"
    exit 1
fi

wmsLayer=${layer}

# Did the user specify a predefined map?
#
if [ "${map}" = "BMNG" ] ; then
    wmsLayer="bmng200406"
    host="http://www.nasa.network.com/wms"
    imageFormat="image/tiff"

elif [ "${map}" = "landsat" ] ; then
    wmsLayer="esat"
    host="http://www.nasa.network.com/wms"
    imageFormat="image/tiff"

elif [ "${map}" = "USstates" ] ; then
    # these range vs. scale factors are empirically determined!
# This server has been deemed unreliable; substituting the worldwind server below...
#    lonRange=`realRange ${minLon} ${maxLon}`
#    if [ `realValGT ${lonRange} 59` -eq 1 ] ; then        
#        wmsLayer="ATLAS_STATES_150"
#    elif [ `realValGT ${lonRange} 25` -eq 1 ] ; then
#        wmsLayer="ATLAS_STATES_075"
#    else
#        wmsLayer="ATLAS_STATES"
#    fi
#    host="http://imsref.cr.usgs.gov:80/wmsconnector/com.esri.wms.Esrimap/USGS_EDC_National_Atlas"
#    imageFormat="image/png"
#    depth="-depth 8"
    wmsLayer="topp:states"
    host="http://worldwind22.arc.nasa.gov/geoserver/wms"
    styles="countryboundaries"
    transparent="TRUE"

elif [ "${map}" = "UScounties" ] ; then
    # these range vs. scale factors are empirically determined!
    lonRange=`realRange ${minLon} ${maxLon}`
    if [ `realValGT ${lonRange} 59` -eq 1 ] ; then        
        wmsLayer="ATLAS_STATES_150"
    elif [ `realValGT ${lonRange} 25` -eq 1 ] ; then
        wmsLayer="ATLAS_STATES_075"
    else
        wmsLayer="ATLAS_STATES"
    fi
    wmsLayer="ATLAS_COUNTIES_2000,"${wmsLayer}
    host="http://imsref.cr.usgs.gov:80/wmsconnector/com.esri.wms.Esrimap/USGS_EDC_National_Atlas"
    imageFormat="image/png"
    depth="-depth 8"

elif [ "${map}" = "world" ] ; then 
# This server has been ddeemed unreliable; substituting the worldwind server instead --RLB
#    wmsLayer="1:1"
#    host="http://columbo.nrlssc.navy.mil/ogcwms/servlet/WMSServlet/Earth_Satellite_Corp_Maps.wms"
#    imageFormat="image/png"
#    version="1.1.0"
#    styles=""
    wmsLayer="topp:cia"
    host="http://worldwind22.arc.nasa.gov/geoserver/wms"
    styles="countryboundaries"
    transparent="TRUE"
    
elif [ "${map}" = "rivers" ] ; then
    wmsLayer="RIVERS"
    host="http://viz.globe.gov/viz-bin/wmt.cgi"
    imageFormat="image/png"

elif [ "${map}" != "" ] ; then
    echo "unknown map name: " ${map}
    exit 1
fi

# If image-format is not tiff, we'll need the convert utility from 
# Imagemagick...
if [ "${imageFormat}" != "image/tiff" ] ; then
    nosave=`which convert 2>/dev/null`
    if ! [ $? -eq 0 ]; then
        echo "Could not find Imagemagick's \"convert\" utility, which is required when requesting non-tiff images."
        echo "See http://www.imagemagick.org"
        exit 1
    fi
fi

# If no image filename specified, name it after the layer...
#
if [ "${imageFile}" = "" ] ; then
    if [ "${map}" != "" ] ; then
        imageFile=${map}.tiff
    else
        imageFile=${layer}.tiff
    fi
fi          

echo "Extent:           " $minLon","$minLat " (LL)  " $maxLon","$maxLat "(UR)"
echo "Image resolution: " $xres "X" $yres
echo "Image filename:   " $imageFile
echo "Image layer:      " $wmsLayer
echo "WMS URL:          " $host

# compose the URL
#
url1=${host}"?""request=GetMap"${amp}"service=wms"${amp}"version="${version}${amp}"layers="${wmsLayer}
url2="styles="${styles}${amp}"bbox="${minLon}","${minLat}","${maxLon}","${maxLat}
url3="format="${imageFormat}${amp}"height="${yres}${amp}"width="${xres}${amp}"srs=epsg:4326"${amp}"transparent="${transparent}
url=${url1}${amp}${url2}${amp}${url3}

cmd="${fetchProg} ${tempFile} ${url}"
echo ${cmd}
${cmd}

if  ( grep -s "ServiceException" ${tempFile} ); then
    echo "Received WMS ServiceException:"
    cat ${tempFile}
    exit 1
fi

# Need to convert non-tiffs into tiff;  this is where the dependency on
# Imagemagick comes from
#
if [ "${imageFormat}" != "image/tiff" ] ; then
    mv ${tempFile} ${tempFile}2
    cmd="convert ${compression} ${depth} ${tempFile}2 tiff:${tempFile}"
    echo ${cmd}
    ${cmd}
    if [ ${debugMode} -ne 1 ] ; then
        rm ${tempFile}2
    fi
fi

# build this command in pieces -- the lon/lat min/max parameters need to appear
# as one logical token on the command-line
#
cmd1="tiff2geotiff -4 +proj=longlat -n"
cmd2="${minLon} ${minLat} ${maxLon} ${maxLat}"
cmd3="${tempFile} ${imageFile}"
echo ${cmd1} ${cmd2} ${cmd3}
${cmd1} "${cmd2}" ${cmd3}

if [ -f ${imageFile} ] ; then
    echo "Image filename is: " ${imageFile}
else
    echo "Image fetch seems to have failed."
fi

if [ ${debugMode} -ne 1 ] ; then 
    rm -f ${tempFile}
fi
