PARAM_SIGNING_IDENTITY="iPhone Distribution: Meedeor, LLC"
cd "platform/ios/DaBesEngine"
WORKSPACE=`pwd`
xcodebuild -target "DaBesEngine" -configuration "Release" BUILD_RELEASE_DEPENDENCIES=0 ARCHIVE_AFTER_BUILD=0 CODE_SIGN_IDENTITY="$PARAM_SIGNING_IDENTITY" CODE_SIGN_REQUIRED=YES clean build
/usr/bin/xcrun -sdk iphoneos PackageApplication \
    -v "$WORKSPACE/build/Release-iphoneos/DaBesEngine.app" \
    -o "$WORKSPACE/build/DaBesEngine.ipa" \
    --sign "${PARAM_SIGNING_IDENTITY}" \
    --embed "/Users/hbridges/Library/MobileDevice/Provisioning Profiles/D3E1C323-88E9-4AA4-A6D2-7037DE17E4D8.mobileprovision"
