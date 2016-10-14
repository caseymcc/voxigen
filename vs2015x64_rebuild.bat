rem call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" amd64

set CURRDIR=%CD%
set BUILD_DIR=%CURRDIR%\build\VS2015\x64

mkdir %BUILD_DIR%
cd %BUILD_DIR%

set HUNTER_PACKAGE_DIR=D:/projects/hunterPackages

if not exist %HUNTER_PACKAGE_DIR% mkdir %HUNTER_PACKAGE_DIR%

cmake^
 -DHUNTER_PACKAGE_DIR=%HUNTER_PACKAGE_DIR%^
 -G "Visual Studio 14 2015 Win64" %CURRDIR%

cd ../../..