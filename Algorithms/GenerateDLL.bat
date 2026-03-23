@echo off
chcp 65001 >nul 2>&1
setlocal enabledelayedexpansion

:: 设置编译器路径（根据你的Qt MinGW路径调整）
set "GCC_PATH=C:\Qt\Qt5.14.2\Tools\mingw730_32\bin\g++.exe"
:: 设置OpenCV的include和lib路径
set "OPENCV_INC=-I E:/Libs/opencv_MinGW/include"
set "OPENCV_LIB=-L E:/Libs/opencv_MinGW/bin -lopencv_core460 -lopencv_imgproc460 -lopencv_highgui460"
:: 通用编译参数
set "COMMON_FLAGS=-shared -fPIC -std=c++11 -static-libgcc -static-libstdc++ -DBUILD_DLL"

echo ======================================
echo 开始编译AlgorithmBlock.dll...
echo ======================================
:: 编译第一个DLL
"%GCC_PATH%" ./AlgorithmBlock.cpp %COMMON_FLAGS% %OPENCV_INC% %OPENCV_LIB% -o AlgorithmBlock.dll

:: 检查编译是否成功
if %errorlevel% equ 0 (
    echo [成功] AlgorithmBlock.dll 编译完成！
) else (
    echo [错误] AlgorithmBlock.dll 编译失败！
    pause
    exit /b 1
)

echo.
echo ======================================
echo 开始编译AlgorithmLine.dll...
echo ======================================
:: 编译第二个DLL
"%GCC_PATH%" ./AlgorithmLine.cpp %COMMON_FLAGS% %OPENCV_INC% %OPENCV_LIB% -o AlgorithmLine.dll

:: 检查编译是否成功
if %errorlevel% equ 0 (
    echo [成功] AlgorithmLine.dll 编译完成！
    echo.
    echo 所有DLL编译完成！
) else (
    echo [错误] AlgorithmLine.dll 编译失败！
    pause
    exit /b 1
)

pause
endlocal