@echo off
cd /d "%~dp0"
py plotLODStatic.py
py plotLODDynamic.py
pause