call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" amd64

call vs2015x64_rebuild.bat

set SOLUTION_FILE=%BUILD_DIR%\voxigen.sln
rem cd ../../..
devenv "%SOLUTION_FILE%"