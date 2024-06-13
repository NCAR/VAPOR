#!/bin/bash

#set -e

clear
#echo $(bash --version)

baseDir="/Applications/vapor.app"
binDir="${baseDir}/Contents/MacOS"
libDir="${baseDir}/Contents/Frameworks"
pluginDir="${baseDir}/Contents/PlugIns"
pyDir="${libDir}/python3.9"
#codesignSignature="sudo codesign --force --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"
declare codesignSignature="sudo codesign --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"
declare removeCodeSignature="sudo codesign --remove-signature"
declare verifyCodeSignature="codesign --verify --deep --strict --verbose=2"

declare depth=0
recurseOnRpath() {
    ((depth++))
    local fileName=$1
    local command=$2
    local isBinary=$(file -L --mime-type -b $fileName | grep -c "application/x-mach-binary")

    if [ $isBinary == 1 ]; then 
        local deps=($(otool -L $fileName | awk '/@rpath/{print $1}')) # gather an array of @rpath dependencies
        local numDeps=${#deps[@]}
        if [ numDeps > 1 ]; then
            local index=$((numDeps-1))
            while [ $index -gt 0 ]; do
                local dep=${deps[$index]}
                dep="${dep//@rpath/$libDir}"
                #if [ "$fileName" != "${deps[$index]}" ]; then
                #    echo "          $fileName ${deps[$index]}"
                    recurseOnRpath $dep $command
                #fi
                index=$((index-1))
            done
        fi
        printf "signing d:$depth %*s%s" $((depth * 2)) ' ' "$fileName"
        echo
        if [ "$command" == "add" ]; then
            echo "ADD    $codesignSignature $fileName"
            $codesignSignature $fileName
        elif [ "$command" == "remove" ]; then
            echo "REMOVE $removeCodeSignature $fileName"
            $removeCodeSignature $fileName
        elif [ "$command" == "verify" ]; then
            echo "VERIFY $verifyCodeSignature $fileName"
            $verifyCodeSignature $fileName
        fi
    fi
    printf "\n"
    ((depth--))
}

recurseOnLoaderPath() {
    ((depth++))
    local fileName=$1
    local command=$2
    local isBinary=$(file -L --mime-type -b $fileName | grep -c "application/x-mach-binary")

    if [ $isBinary == 1 ]; then 
        local deps=($(otool -L $fileName | awk '/@loader_path/{print $1}')) # gather an array of @rpath dependencies
        local numDeps=${#deps[@]}
        if [ numDeps > 0 ]; then
            local index=$((numDeps-1))
            while [ $index -gt 0 ]; do
                local dep=${deps[$index]}
                dep="${dep//@loader_path/$pyDir/site-packages}"
                recurseOnLoaderPath $dep $command
                index=$((index-1))
            done
        fi
        #printf "signing d:$depth %*s%s" $((depth * 2)) ' ' "$fileName"
        #echo
        if [ "$command" == "add" ]; then
            echo "$codesignSignature $fileName"
            $codesignSignature $fileName
        elif [ "$command" == "remove" ]; then
            echo "$removeCodeSignature $fileName"
            $removeCodeSignature $fileName
        elif [ "$command" == "verify" ]; then
            #echo "$verifyCodeSignature $fileName"
            $verifyCodeSignature $fileName
        fi
    fi

    ((depth--))
}

#codeSign $libDir/QtWidgets.framework/QtWidgets
#codeSign $libDir/QtMultimedia.framework/QtMultimedia

###################
#
# Libraries
#
###################

# Remove all pre-existing code signatures
for file in $(find $libDir -name "*"); do
    recurseOnRpath $file "remove"
done

# Codesign all libraries, dependencies first
for file in $(find $libDir -name "*"); do
    recurseOnRpath $file "add"
done

# Verify codesignature of all libraries
for file in $(find $libDir -name "*"); do
    recurseOnRpath $file "verify"
done

###################
#
# Plugins
#
###################

# Remove all pre-existing code signatures
for file in $(find $pluginDir -name "*"); do
    recurseOnRpath $file "remove"
done

# Codesign all plugins, dependencies first
for file in $(find $pluginDir -name "*"); do
    recurseOnRpath $file "add"
done

# Verify codesignature of all plugins
for file in $(find $pluginDir -name "*"); do
    recurseOnRpath $file "verify"
done

###################
#
# Python
#
###################

## Codesign all python .so files
#for file in $(find $pyDir/lib-dynload -name "*.so"); do
#    $removeCodeSignature $file
#    $codesignSignature $file
#done
#
## Codesign this one too
#$codesignSignature $pyDir/config-3.9-darwin/python.o
#
## Codesign python site-packages libraries
#for file in $(find $pyDir/site-packages -name "*.so"); do
#    recurseOnLoaderPath $file "add"
#done
#
## Codesign all .pyc files
#for file in $(find $pyDir -name "*.pyc"); do
#    $codesignSignature $file
#done

###################
#
# Executables
#
###################

# Codesign all executables
for file in $(find $binDir -name "*"); do
    $removeCodeSignature $file
    $codesignSignature $file
done

#baseDir="/Applications/vapor.app"
#binDir="${baseDir}/Contents/MacOS"
#libDir="${baseDir}/Contents/Frameworks"
#pluginDir="${baseDir}/Contents/PlugIns"
#pyDir="${libDir}/python3.9"
##codesignSignature="sudo codesign --force --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"
#declare codesignSignature="sudo codesign --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp"
#declare removeCodeSignature="sudo codesign --remove-signature"
#declare verifyCodeSignature="codesign --verify --deep --strict --verbose=2"

# Codesign the bundle
printf "Codesign the bundle"
$codesignSignature --force $baseDir

# repackage the .dmg
version=$($baseDir/Contents/MacOS/vaporversion)
version=3.9.1
hdiutil create -ov -srcFolder $baseDir -o VAPOR3-$version-M1signed.dmg

# try to sign the bundle
#sudo codesign --force --verbose=2 --sign DQ4ZFL4KLF --options runtime --timestamp --force /Applications/testApp.app

# Codesign unsigned frameworks that macdeployqt complains about
#codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/testApp.app/Contents/Frameworks/QtUiPlugin.framework
#codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/testApp.app

# app-specific passwords: https://support.apple.com/en-us/102654
xcrun notarytool store-credentials "testApp-password" --apple-id pearse@ucar.edu --team-id DQ4ZFL4KLF --password ggjp-lbdh-ziib-ruqg
#xcrun notarytool store-credentials "notarytool-password" --apple-id pearse@ucar.edu --team-id DQ4ZFL4KLF --password vvnd-oaaz-rxge-ukma

codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/testApp.app/Contents/MacOS/platforms/libqcocoa.dylib
codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/testApp.app/Contents/MacOS/styles/libqmacstyle.dylib

# run macdeployqt
#/usr/local/VAPOR-Deps/current/bin/macdeployqt /Applications/testApp.app -dmg -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF), pearse@ucar.com, ggjp-lbdh-ziib-ruqg"
#/usr/local/VAPOR-Deps/current/bin/macdeployqt /Applications/testApp.app -timestamp -dmg -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"
/usr/local/VAPOR-Deps/current/bin/macdeployqt /Applications/testApp.app -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"

# create and sign the .dmg
hdiutil create -volname "testApp" -srcfolder /Applications/testApp.dmg -ov -format UDZO /Applications/testApp.app
codesign --force --verify --verbose --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" /Applications/testApp.dmg

# notarize
xcrun -v notarytool submit /Applications/testApp.dmg --keychain-profile "testApp-password" --wait
#xcrun -v notarytool submit testApp-M1signed.dmg --keychain-profile "testApp-password" --wait
#xcrun -v notarytool submit VAPOR3-3.9.1-M1signed.dmg --keychain-profile "notarytool-password" --wait

# Fetch the notorization log
xcrun notarytool log --keychain-profile "testApp-password" 246f94c0-9efa-4f2f-9513-2da8a696573b
#xcrun -v notarytool log f34e7964-93ab-4693-941d-c562cf7062a6
# % xcrun notarytool log CREDENTIALS REQUEST_UUID
# where:
# CREDENTIALS is your notary service credentials, the same credentials you used to submit your request
# REQUEST_UUID is the UUID printed by notarytool when you submitted your request.

xcrun stapler staple /Applications/testApp.dmg
xcrun stapler validate /Applications/testApp.dmg



# Must install gcc@11 via homebrew to be compliant with the third-party-library build
# > /opt/homebrew/bin/brew install gcc@11
# > rm /opt/homebrew/opt/gcc
# > ln -s /opt/homebrew/Cellar/gcc@11/11.4.0 /opt/homebrew/opt/gcc
#for file in /Applications/testApp.app/Contents/Frameworks/libhdf5*.dylib; do
for file in /Applications/testApp.app/Contents/Frameworks/*.dylib; do
    # Check if the file exists to avoid printing the pattern itself when no files match
    if [ -e "$file" ]; then
        codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose $file
    fi
done

codesign --verify --deep --strict --verbose=2

codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/testApp.app/Contents/MacOS/platforms/libqcocoa.dylib \
&& codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/testApp.app/Contents/MacOS/styles/libqmacstyle.dylib \
&& /usr/local/VAPOR-Deps/current/bin/macdeployqt /Applications/testApp.app -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" \
&& codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose --options runtime /Applications/testApp.app/Contents/MacOS/testApp \
&& hdiutil create -volname "testApp" -srcfolder /Applications/testApp.app -ov -format UDZO /Applications/testApp.dmg \
&& codesign --force --verify --verbose --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" /Applications/testApp.dmg \
&& xcrun -v notarytool submit /Applications/testApp.dmg --keychain-profile "testApp-password" --wait \
&& xcrun stapler staple /Applications/testApp.dmg \
&& xcrun stapler validate /Applications/testApp.dmg


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
