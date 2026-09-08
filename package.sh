#!/bin/bash
set -e
mkdir -p Orca-Light-Windows-x64
cp Orca-Light.exe install.bat run.bat uninstall.bat config.ini README.txt Orca-Light-Windows-x64/
rm -f Orca-Light-Windows-x64.zip
zip -r Orca-Light-Windows-x64.zip Orca-Light-Windows-x64
