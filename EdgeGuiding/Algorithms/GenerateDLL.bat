@echo off
:: 不使用 UTF-8，避免乱码
:: chcp 65001

setlocal enabledelayedexpansion

set "GCC_PATH=E:/QT/Tools/mingw730_32/bin/g++.exe"
set "OPENCV_INC=-I E:/QT/Tools/opencv_MinGW/include"
set "OPENCV_LIB=-L E:/QT/Tools/opencv_MinGW/lib -lopencv_core460 -lopencv_imgproc460 -lopencv_highgui460"

set "COMMON_FLAGS=-shared -fPIC -std=c++11 -static-libgcc -static-libstdc++ -DBUILD_DLL"

echo ============================
echo Building AlgorithmBlock.dll
echo ============================

"%GCC_PATH%" ./AlgorithmBlock.cpp %COMMON_FLAGS% %OPENCV_INC% %OPENCV_LIB% -o AlgorithmBlock.dll

if !errorlevel! equ 0 (
    echo [OK] AlgorithmBlock.dll built
) else (
    echo [ERROR] build failed
    pause
    exit /b 1
)

echo.
echo ============================
echo Building AlgorithmLine.dll
echo ============================

"%GCC_PATH%" ./AlgorithmLine.cpp %COMMON_FLAGS% %OPENCV_INC% %OPENCV_LIB% -o AlgorithmLine.dll

if !errorlevel! equ 0 (
    echo [OK] AlgorithmLine.dll built
) else (
    echo [ERROR] build failed
    pause
    exit /b 1
)

pause
endlocal