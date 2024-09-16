set +e

installDir="/Applications"
dmgDir="tmp"

dmgName="VAPOR3-`$installDir/vapor.app/Contents/MacOS/vaporversion -numeric`-"
if [[ "/Applications/vapor.app/Contents/MacOS/vapor: Mach-O 64-bit executable arm64" == *arm64* ]]; then
    dmgName="${dmgName}AppleSilicon.dmg"
else
    dmgName="${dmgName}macOSx86.dmg"
fi

#for file in ${installDir}/vapor.app/Contents/Frameworks/*; do
#    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --timestamp --verbose $file
#done

# Deploy Qt with macdeployqt
# N.B. Apple's notorization will give an "bundle format is ambiguous (could be app or framework)" if Qt's frameworks are missing symlinks
# So, if copying the third party libraries use 'cp -a /old/libs /new/libs' and not 'cp -r /old/libs /new/libs' (-a and not -r)
/usr/local/VAPOR-Deps/current/bin/macdeployqt ${installDir}/vapor.app -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"

# Codesign libraries and frameworks
find ${installDir}/vapor.app/Contents/Frameworks -type f -exec file {} + | grep -E 'Mach-O .* shared library' | cut -d: -f1 | while read -r file; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --timestamp --force --deep --verbose --options runtime "$file"
done

# Codesign plugins and python
directories=(
    "${installDir}/vapor.app/Contents/share/plugins/"
    "${installDir}/vapor.app/Contents/Resources/"
    "${installDir}/vapor.app/Contents/Resources/python/lib/"
    "${installDir}/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/numpy/.dylibs"
    "${installDir}/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/PIL/.dylibs"
    "${installDir}/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/scipy/.dylibs"
)
for dir in "${directories[@]}"; do
    find "$dir" -type f \( -name "*.so" -o -name "*.dylib" \) | while read -r file; do
        codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --timestamp --force --deep --verbose $file
    done
done

# Codesign additional MacOS plugins
codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose ${installDir}/vapor.app/Contents/MacOS/platforms/libqcocoa.dylib
codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose ${installDir}/vapor.app/Contents/MacOS/styles/libqmacstyle.dylib

# Codesign all vapor executables with timestamp and hardended runtime
for file in ${installDir}/vapor.app/Contents/MacOS/*; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --timestamp --verbose --options runtime $file
done

# Create .dmg that contains symlink to /Applications
mkdir -p "${dmgDir}"
cp -R "${installDir}/vapor.app" "${dmgDir}"
ln -s /Applications "${dmgDir}/Applications"
hdiutil create -volname "vapor" -srcfolder ${dmgDir} -ov -format UDZO "${installDir}/${dmgName}"
rm -rf "${dmgDir}"

# Sign and notarize the .dmg
codesign --force --verify --verbose --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" ${installDir}/${dmgName}
xcrun -v notarytool submit ${installDir}/${dmgName} --keychain-profile "VAPOR3" --wait

# Staple the notarized .dmg
xcrun stapler staple ${installDir}/${dmgName}
xcrun stapler validate ${installDir}/${dmgName}

# Fetch the notorization log
# xcrun notarytool log --keychain-profile "testApp-password" <hash>
