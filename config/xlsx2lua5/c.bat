@echo off
set dir=%1%
for /f "delims=" %%i in ('dir /ad/b/s/a "%dir%"') do (echo  %%i)
