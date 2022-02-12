@echo off

:init
setlocal DisableDelayedExpansion
set "batchPath=%~0"
for %%k in (%0) do set batchName=%%~nk
set "vbsGetPrivileges=%temp%\OEgetPriv_%batchName%.vbs"
setlocal EnableDelayedExpansion

:checkPrivileges
NET FILE 1>NUL 2>NUL
if '%errorlevel%' == '0' ( goto gotPrivileges ) else ( goto getPrivileges )

:getPrivileges
if '%1'=='ELEV' (echo ELEV & shift /1 & goto gotPrivileges)

echo Set UAC = CreateObject^("Shell.Application"^) > "%vbsGetPrivileges%"
echo args = "ELEV " >> "%vbsGetPrivileges%"
echo For Each strArg in WScript.Arguments >> "%vbsGetPrivileges%"
echo args = args ^& strArg ^& " "  >> "%vbsGetPrivileges%"
echo Next >> "%vbsGetPrivileges%"
echo UAC.ShellExecute "!batchPath!", args, "", "runas", 1 >> "%vbsGetPrivileges%"
"%SystemRoot%\System32\WScript.exe" "%vbsGetPrivileges%" %*
exit /B

:gotPrivileges
setlocal & pushd .
cd /d %~dp0
if '%1'=='ELEV' (del "%vbsGetPrivileges%" 1>nul 2>nul  &  shift /1)

:: Start of the main sector
if not exist yosen_lang.exe cd ..

:: Retrieve the full binary path
set bin_path="%cd%\build\bin\Windows\Release\"

if exist yosen_lang.exe set bin_path="%cd%"

:: Get the installation path
set install_path=C:\yosen_lang

:: Make sure the installation folder is ready
if not exist %install_path% mkdir %install_path%

:: Copy all the binary files and
:: modules into the installation directory.
xcopy /e /v %bin_path% %install_path%

:: Add yosen to path
set NEW_PATH="%PATH%;%install_path%"
setx PATH %NEW_PATH%

:: Renaming yosen_lang => yosen
cd %install_path%
ren yosen_lang.exe yosen.exe

pause