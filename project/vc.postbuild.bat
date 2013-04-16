@ECHO OFF
IF "%2"=="Debug"   GOTO Debug
IF "%2"=="Release" GOTO Release
GOTO End
:Debug
XCOPY %OGRE_HOME%\Bin\%2\RenderSystem_Direct3D9_d.dll %~dp0..\bin\%1\%2\ /Y
XCOPY %OGRE_HOME%\Bin\%2\RenderSystem_GL_d.dll        %~dp0..\bin\%1\%2\ /Y
GOTO Common
:Release
XCOPY %OGRE_HOME%\Bin\%2\RenderSystem_Direct3D9.dll %~dp0..\bin\%1\%2\ /Y
XCOPY %OGRE_HOME%\Bin\%2\RenderSystem_GL.dll        %~dp0..\bin\%1\%2\ /Y
GOTO Common
:Common
XCOPY %~dp0plugins.cfg   %~dp0..\bin\%1\%2\ /Y
XCOPY %~dp0resources.cfg %~dp0..\bin\%1\%2\ /Y
GOTO End
:End
