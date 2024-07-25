set +e

for file in /Applications/vapor.app/Contents/Frameworks/*; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --timestamp --verbose $file
done

/usr/local/VAPOR-Deps/current/bin/macdeployqt /Applications/vapor.app -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"
#/usr/local/VAPOR-Deps/2023-Sept-codesigning/bin/macdeployqt /Applications/vapor.app -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"
#/usr/local/VAPOR-Deps/current/bin/macdeployqt /Applications/vapor.app -hardened-runtime -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"

#find "/Applications/vapor.app/Contents/Frameworks" -type f \( -name "*.dylib" -o -name "*.framework" \) | while read -r file; do
find /Applications/vapor.app/Contents/Frameworks -type f -exec file {} + | grep -E 'Mach-O .* shared library' | cut -d: -f1 | while read -r file; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --timestamp --force --deep --verbose --options runtime "$file"
done

    #"/Applications/vapor.app/Contents/Frameworks/python3.9"
    #"/Applications/vapor.app/Contents/Frameworks/python3.9/site-packages/numpy/.dylibs"
    #"/Applications/vapor.app/Contents/Frameworks/python3.9/site-packages/PIL/.dylibs"
    #"/Applications/vapor.app/Contents/Frameworks/python3.9/site-packages/scipy/.dylibs"
directories=(
    "/Applications/vapor.app/Contents/share/plugins/"
    "/Applications/vapor.app/Contents/Resources/"
    "/Applications/vapor.app/Contents/Resources/python/lib/"
    "/Applications/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/numpy/.dylibs"
    "/Applications/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/PIL/.dylibs"
    "/Applications/vapor.app/Contents/Resources/python/lib/python3.9/site-packages/scipy/.dylibs"
)
#directories=(
#    #"/usr/local/VAPOR-Deps/current/lib"
#    #"/usr/local/VAPOR-Deps/current/Ospray"
#    "/usr/local/VAPOR-Deps/2023-Sept-codesigning/lib"
#    "/usr/local/VAPOR-Deps/2023-Sept-codesigning/Ospray"
#)
for dir in "${directories[@]}"; do
    find "$dir" -type f \( -name "*.so" -o -name "*.dylib" \) | while read -r file; do
        codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --timestamp --force --deep --verbose $file
    done
done

codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/vapor.app/Contents/MacOS/platforms/libqcocoa.dylib
codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --verbose /Applications/vapor.app/Contents/MacOS/styles/libqmacstyle.dylib

for file in /Applications/vapor.app/Contents/MacOS/*; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --timestamp --verbose --options runtime $file
done

for file in /Applications/vapor.app/Contents/Resources/python/bin*; do
    codesign --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" --force --deep --timestamp --verbose --options runtime $file
done

#/usr/local/VAPOR-Deps/current/bin/macdeployqt /Applications/vapor.app -hardened-runtime -timestamp -sign-for-notarization="Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)"

hdiutil create -volname "vapor" -srcfolder /Applications/vapor.app -ov -format UDZO /Applications/vapor.dmg
codesign --force --verify --verbose --sign "Developer ID Application: University Corporation for Atmospheric Research (DQ4ZFL4KLF)" /Applications/vapor.dmg
xcrun -v notarytool submit /Applications/vapor.dmg --keychain-profile "testApp-password" --wait

xcrun stapler staple /Applications/vapor.dmg
xcrun stapler validate /Applications/vapor.dmg

# Fetch the notorization log
#xcrun notarytool log --keychain-profile "testApp-password" <hash>
