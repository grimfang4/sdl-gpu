#!/bin/sh

mkdir buildandroidcmake
cd buildandroidcmake
rm -rf SDL_image
../AndroidCMake/genproj_android.pl --blurrrsdkpath=~/Source/Blurrr/Release/Blurrr_Apple_DP1/Libraries/Android/SDK/C  --standalonetoolchainroot=$ANDROID_NDK_ROOT/standalone ..
./make_ndk.sh
mv dist SDL_image

