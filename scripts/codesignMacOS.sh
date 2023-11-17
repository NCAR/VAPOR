#!/bin/bash

clear
#echo $(bash --version)

baseDir="/Applications/vapor.app/Contents"
binDir="${baseDir}/MacOS"
libDir="${baseDir}/Frameworks"
pyDir="${libDir}/python3.9/__pycache__"
#codesignSignature="sudo codesign --force --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"
codesignSignature="sudo codesign --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"

fileTypes=(
    "library"
    "byte-compiled"
    "executable"
)

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
