# Error history of cross build tensorflowlite_c shred library for windows on WSL2

I needed to build tensorflowlite_c library for windows platform. Since I already built it on WSL for linux. I though it would be easy because of I'm quite familiar with cmake now. But it was my mistake. I would do just download prebuilt one through online if I knew this will cost so much time to me. 

From in my previous experience, I already had built the tensorflowlite_c.dll using bazel, msys2 etc.  

At this time I decided to use only cmake and single compiler with cross compile methods...

***
* First error with wrong flags. This flag '/bigobj' is for windows compiler flag type.
* The compiler couln't parse it properly. Just accept it as a path not a flag.
![problem1 first error with wrong flags](./images/01_build_error_tflite_c_for_win_on_wsl_problem1.jpg)

***
* You can find jenerated make files like 'flags.make'. 
* The flags are for windows compiler like msvc. so erase it.
![problem1_solve1 problem1 erase wrong flags on make file](./images/02_build_error_tflite_c_for_win_on_wsl_problem1_solve1.jpg)

***
* There's weird error occured. the thread library is simply included standard library. Basically, this would not happen.
![problem2 standard library fault thread](./images/03_build_error_tflite_c_for_win_on_wsl_problem2.jpg)

***
* I found there're two kinds of thread library types.
![problem2_solve1 two kinds of thread library types](./images/04_build_error_tflite_c_for_win_on_wsl_problem2_solve1.jpg)

***
* Change simlink to posix version. Then the problem solved.
![Problem2_solve2 change to posix compiler](./images/05_build_error_tflite_c_for_win_on_wsl_problem2_solve2.jpg)

***
* sys/mman.h is for linux not windows.
* This might connect with mmap library. AFAK that package flag should be false for windows build configuration including other affected libraries. 
* AS I stucked this problem, I escape WSL environment and try again with windows host environment. Then I don't need to struggle with these hidden traps anymore.
![Problem3 sys/mman.h problem](./images/06_build_error_tflite_c_for_win_on_wsl_problem3.jpg)

***
* My system already installed visual studio tools for c++ because I've been trying to make flutter app. However, python is only installed on wsl. so I
tried to make simlink to call wsl's python interpreter.
![On_windows_problem1 python interpreter not found](./images/07_build_error_tflite_c_for_win_on_windows_problem1.jpg)

***
* Fail to using wsl's python interpreter. It might not have compatibility.
![On_windows_problem1_solve1 not work wsl's python](./images/08_build_error_tflite_c_for_win_on_windows_problem1_solve1.jpg)

***
* Install miniconda on Windows host. The problem solved.
![On_windows_problem1_solve2 intall miniconda](./images/09_build_error_tflite_c_for_win_on_windows_problem1_solve2.jpg)

***
* Require standard c++ 20 not 17. You can correct this error simply modify 17 flag to 20 in CMakeList.txt file. Please do not follow my struggles.
![On_windows_problem2 standard c++ 20 required](./images/10_build_error_tflite_c_for_win_on_windows_problem2.jpg)

***
* I tried to modify that /std:c++17 flag.
![On_windows_problem2_solve1 change flag](./images/11_build_error_tflite_c_for_win_on_windows_problem2_solve1.jpg)

***
* I found the flags in vcxproj files. Please do not follow this way you can just modify cmake file. It is way much of easy.
![On_windows_problem2_solve2 find flag location](./images/12_build_error_tflite_c_for_win_on_windows_problem2_solve2.jpg)

***
* Finally, success to build. But there're many problems left.
![On_windows_problem2_solve3 temp success build](./images/12_build_error_tflite_c_for_win_on_windows_problem2_solve3.jpg)



