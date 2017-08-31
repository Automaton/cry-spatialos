@echo off
if not exist "Solutions" mkdir Solutions
cd Solutions
cmake -"Ax64" .. 
pause