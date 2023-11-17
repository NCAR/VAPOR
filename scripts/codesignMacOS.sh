#!/bin/bash

clear
#echo $(bash --version)

baseDir="/Applications/vapor.app/Contents"
binDir="${baseDir}/MacOS"
libDir="${baseDir}/Frameworks"
pyDir="${libDir}/python3.9/__pycache__"
#declare -a libTypes=("so" "dylib" "framework" "pyc")
declare -a libTypes=("so" "dylib" "framework")
#codesignSignature="sudo codesign --force --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"
codesignSignature="sudo codesign --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"

libraries=(
    "libtbbmalloc"
    "libopenvkl"
    "libopenvkl_malloc"
    "libtbb"
    "libcrypto"
    "libispcrt"
    "libomp"
    "libssl"
    "libpng"
    "libjpeg"
    "libtiff"
    "libsqlite"
    "libssh"
    "libcurl"
    "libproj"
    "libgeotiff"
    "libz"
    "libassimp"
    "libsz"
    "libhdf5"
    "libnetcdf"
    "libexpat"
    "libudunits"
    "libfreetype"
    "libembree"
    "libospray"
    "libosgl"
)

fileTypes=(
    "library"
    "byte-compiled"
    "executable"
)

#python3.9
#Qt

#codesign()
#    for each library
#        compile list of dependencies
#            if num of @rpath == 1
#                if not codesigned
#                    codesign
#            else 
#                for each @rpath

declare depth=0
codeSign() {
    ((depth++))
    local fileName=$1
    local isBinary=$(file -L --mime-type -b $fileName | grep -c "application/x-mach-binary")

    if [ $isBinary == 1 ]; then 
        local deps=($(otool -L $fileName | awk '/@rpath/{print $1}')) # gather an array of @rpath dependencies
        local numDeps=${#deps[@]}
        if [ numDeps > 1 ]; then
            local index=$((numDeps-1))
            while [ $index -gt 0 ]; do
                local dep=${deps[$index]}
                dep="${dep//@rpath/$libDir}"
                codeSign $dep
                index=$((index-1))
            done
        fi
        printf "signing d:$depth %*s%s" $((depth * 2)) ' ' "$fileName"
        $codesignSignature $fileName
        echo
    fi

    ((depth--))
}

codeSign $libDir/QtWidgets.framework/QtWidgets

foo() {
    for (( i=10 ; i>0 ; i-- )); do
        echo $i
        foo
        echo $i
    done
}

#foo

#TO DO: iterate libraries in above order
# codesign libraries
#for library in "${libraries[@]}"; do
#    echo $library
#    for fileName in $(ls $libDir |grep $library); do
#        echo "    $fileName"
#        rm $baseDir/copies/$fileName
#        #$codesignSignature $file
#    done
#done

#for library in "${libraries[@]}"; do
#    echo $library
#    for fileName in $(ls $libDir |grep $library); do
#        myFileType=$(file fileName)
#        for fileType in "${fileTypes[@]}"; do
#            if [[ $myFileType == *"$fileType"* ]]; then
#                echo lib $file
#                #$codesignSignature $file
#            fi
#        done
#    done
#done

#for libType in "${libTypes[@]}"; do
    #for fileName in $(find $libDir -name *.$libType);

# NEED TO CONFIRM:
# codesign Python .pyc files
#for libType in "pyc"; do
#    echo $libType
#    for file in $(find $pyDir -name *.$libType); do
#        #sudo codesign --force --verbose=2 --sign "DQ4ZFL4KLF" "$file";
#        echo lib $file
#        $codesignSignature $file
#    done
#done
#
## codesign all executables
#for file in $(find $binDir -perm +111 -type f); do
#    echo exe $file
#    $codesignSignature $file
#    #sudo codesign --force --verbose=2 --sign "DQ4ZFL4KLF" "$file";
#done
#
## codesign the bundle
##$codesignSignature --entitlements $baseDir/Contents/share/entitlements/NetworkEntitlements.entitlements $baseDir
#echo codesign bundle
##$codesignSignature --entitlements /Users/vapor/VAPOR/share/entitlements/NetworkEntitlements.entitlements $baseDir
#$codesignSignature $baseDir
##sudo codesign --force --verbose=2 --sign "DQ4ZFL4KLF" "$baseDir";
#
## repackage the .dmg
##version=$($baseDir/Contents/MacOS/vaporversion)
#version=3.9.1
#hdiutil create -srcFolder $baseDir -o VAPOR3-$version-M1signed.dmg
#
## app-specific passwords: https://support.apple.com/en-us/102654
##xcrun notarytool store-credentials "notarytool-password" --apple-id pearse@ucar.edu --team-id DQ4ZFL4KLF --password <app-specific-password>
#
## notarize
##xcrun notarytool submit VAPOR3-3.9.1-M1signed.dmg --keychain-profile "notarytool-password" --wait
