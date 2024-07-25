set +e

installDir="/Applications"
dmgName="VAPOR3-3.9.3-AppleSilicon.dmg"

#for file in $installDir/vapor.app/Contents/Frameworks/*; do
#    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --timestamp --verbose $file
#done

# Deploy Qt with macdeployqt
/usr/local/VAPOR-Deps/current/bin/macdeployqt $installDir/vapor.app -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"

# Codesign libraries and frameworks
find $installDir/vapor.app/Contents/Frameworks -type f -exec file {} + | grep -E 'Mach-O .* shared library' | cut -d: -f1 | while read -r file; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --timestamp --force --deep --verbose --options runtime "$file"
done

# Codesign plugins and python
directories=(
    "$installDir/vapor.app/Contents/share/plugins/"
    "$installDir/vapor.app/Contents/Resources/"
    "$installDir/vapor.app/Contents/Resources/python/lib/"
    "$installDir/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/numpy/.dylibs"
    "$installDir/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/PIL/.dylibs"
    "$installDir/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/scipy/.dylibs"
)
for dir in "${directories[@]}"; do
    find "$dir" -type f \( -name "*.so" -o -name "*.dylib" \) | while read -r file; do
        codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --timestamp --force --deep --verbose $file
    done
done

# Codesign additional MacOS plugins
codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose $installDir/vapor.app/Contents/MacOS/platforms/libqcocoa.dylib
codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose $installDir/vapor.app/Contents/MacOS/styles/libqmacstyle.dylib

# Codesign all vapor executables with timestamp and hardended runtime
for file in $installDir/vapor.app/Contents/MacOS/*; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --timestamp --verbose --options runtime $file
done

# Create .dmg, sign, and notarize it
hdiutil create -volname "vapor" -srcfolder $installDir/vapor.app -ov -format UDZO $installDir/$dmgName
codesign --force --verify --verbose --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" $installDir/$dmgName
xcrun -v notarytool submit $installDir/$dmgName --keychain-profile "testApp-password" --wait

# Staple the notarized .dmg
xcrun stapler staple $installDir/$dmgName
xcrun stapler validate $installDir/$dmgName

# Fetch the notorization log
#xcrun notarytool log --keychain-profile "testApp-password" <hash>
