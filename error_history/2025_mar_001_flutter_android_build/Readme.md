# Build and test app for android.
There are several errors from the start. I needed to update gradle and some code by myself. Unfortunately, even I follow instructions that I found on the Internet. dlopen keeps failing. First time I think it was my fault. However, I found someone stuck error similar with me. It seems that the problem is flutter itself.

*** 
* Errors from the start to build android. flutter doctor shows me no problem.
![01_2025_mar_001_android_build_fail](./images/01_2025_mar_001_android_build_fail.jpg)

***
* Prebuilt library open fail. It seems that jniLibs merged well but the app failed to load.
![02_2025_mar_001_android_app_load_library_fail](./images/02_2025_mar_001_android_app_load_library_fail.jpg)

***
* I found someone suffered similar problem. When I try to his code, I fail to dlopen even debug mode(he said he's success)
![03_2025_mar_001_android_app_load_library_fail2](./images/03_2025_mar_001_android_app_load_library_fail2.jpg)