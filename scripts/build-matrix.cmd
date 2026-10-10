@echo off
REM build-matrix.cmd - build every branch x every preset, collecting the
REM final binaries into build\bin\<Config>\<arch>\slint-boilerplate-<branch>.exe
REM plus the Slint runtime DLL (shared: identical Slint version everywhere).
REM
REM Intermediate build trees live under build\build-win\<arch>\<branch>\ and
REM are intentionally kept (no deletion; remove that folder manually when
REM disk space matters). Worktrees are reused across runs.
REM
REM The per-arch Slint dependency cache (.deps-cache\<arch>) is reused from
REM the MAIN checkout via -DSLINT_CACHE_DIR so worktrees never re-download.
REM vcvars is invoked per target arch so CMake's CMAKE_SYSTEM_PROCESSOR
REM selects the right cache subdirectory (x86 vs AMD64).
REM
REM Usage: build-matrix.cmd [branch ...]   (default: main simple intermediate advanced)

setlocal enabledelayedexpansion
set "ROOT=%~dp0.."
for %%I in ("%ROOT%") do set "ROOT=%%~fI"
set "CMAKE=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat"
set "BINOUT=%ROOT%\build\bin"
set "WORK=%ROOT%\build\build-win"

set BRANCHES=%*
if "%BRANCHES%"=="" set BRANCHES=main simple intermediate advanced

set FAILURES=0
for %%B in (%BRANCHES%) do call :build_branch %%B
echo.
echo === DONE. Collected binaries: ===
dir /s /b "%BINOUT%\*.exe" 2>nul
if %FAILURES% gtr 0 echo(%FAILURES% preset build(s) FAILED - see output above.
echo Intermediate worktrees kept under %WORK% - delete manually if desired.
exit /b %FAILURES%

:build_branch
set "BR=%~1"
REM Accept both "simple" and "examples/simple"
git -C "%ROOT%" rev-parse --verify --quiet "examples/%BR%" >nul 2>&1
if not errorlevel 1 (set "FULL=examples/%BR%") else (set "FULL=%BR%")

echo.
echo ###########################################################
echo ### Building branch %FULL%
echo ###########################################################

set "WT_PREV="
for %%P in (x64-debug x64-release x86-debug x86-release) do call :build_preset %BR% %%P
exit /b 0

:build_preset
set "BR=%~1"
set "PRESET=%~2"

REM Substring checks (pure cmd - no find.exe, which may be GNU find via PATH)
set "CFG=Debug"
if not "%PRESET:release=%"=="%PRESET%" set "CFG=Release"
set "ARCH=x64"
if not "%PRESET:x86=%"=="%PRESET%" set "ARCH=x86"

echo.
echo --- %BR% / %PRESET% (%CFG% %ARCH%) ---

set "WT=%WORK%\%ARCH%\%BR%"
if not "%WT%"=="%WT_PREV%" (
    git -C "%ROOT%" worktree prune
    if exist "%WT%\CMakeLists.txt" (
        echo Reusing worktree at %WT%
        REM CRITICAL: a reused worktree sits at whatever commit it was
        REM created at. Re-point it at the branch tip or we silently
        REB build stale code while reporting success.
        git -C "%WT%" checkout --detach "%FULL%" >nul 2>&1
        if errorlevel 1 (echo CHECKOUT FAILED for %FULL% & set /a FAILURES+=1 & exit /b 0)
        for /f %%H in ('git -C "%WT%" rev-parse --short HEAD') do echo   at %%H
    ) else (
        git -C "%ROOT%" worktree add --detach "%WT%" "%FULL%"
        if errorlevel 1 (echo WORKTREE FAILED for %FULL% & set /a FAILURES+=1 & exit /b 0)
    )
    set "WT_PREV=%WT%"
)
set "BUILD_DIR=%WT%\build\%PRESET%"

REM vcvarsall prepends to PATH on every call; repeated calls grow the
REM environment until cmd dies with "The input line is too long".
REM Build each preset inside setlocal/endlocal so the polluted
REM environment is discarded afterwards.
setlocal
call "%VCVARS%" %ARCH% >nul 2>&1
if errorlevel 1 (endlocal & echo VCVARS FAILED for %ARCH% & set /a FAILURES+=1 & exit /b 0)
"%CMAKE%" --preset %PRESET% -S "%WT%" -B "%BUILD_DIR%" -DSLINT_CACHE_DIR="%ROOT%\.deps-cache"
if errorlevel 1 (endlocal & echo CONFIGURE FAILED %BR% %PRESET% & set /a FAILURES+=1 & exit /b 0)
"%CMAKE%" --build "%BUILD_DIR%" --target slint-boilerplate
if errorlevel 1 (
    endlocal
    echo BUILD FAILED %BR% %PRESET%
    set /a FAILURES+=1
    REM Deleting a stale exe is not enough: ninja leaves the previous binary
    REM in place on codegen failure. Without deleting it, a stale binary
    REM gets collected and reported as success.
    if exist "%BUILD_DIR%\slint-boilerplate.exe" del "%BUILD_DIR%\slint-boilerplate.exe"
    exit /b 1
)
endlocal

set "DEST=%BINOUT%\%CFG%\%ARCH%"
if not exist "%DEST%" mkdir "%DEST%"
if not exist "%BUILD_DIR%\slint-boilerplate.exe" (
    echo COLLECT SKIPPED %BR% %PRESET% - no binary produced by this build
    set /a FAILURES+=1
    exit /b 1
)
copy /y "%BUILD_DIR%\slint-boilerplate.exe" "%DEST%\slint-boilerplate-%BR%.exe" >nul
copy /y "%BUILD_DIR%\slint_cpp.dll" "%DEST%\slint_cpp.dll" >nul
echo COLLECTED %DEST%\slint-boilerplate-%BR%.exe
exit /b 0
