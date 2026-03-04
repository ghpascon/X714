
@echo off
setlocal enabledelayedexpansion
echo Selecione o tipo de commit:
echo 1 - Patch (correcoes pequenas)
echo 2 - Minor (novas funcionalidades)
echo 3 - Major (mudancas incompativeis)
set /p type=Digite o numero (1/2/3): 

set state=
if "%type%"=="1" set state=patch
if "%type%"=="2" set state=minor
if "%type%"=="3" set state=major
if "%state%"=="" (
	echo Tipo invalido. Abortando.
	exit /b 1
)

set /p msg=Digite a mensagem do commit: 

git add .
git commit -m "%state% - %msg%"
git push
