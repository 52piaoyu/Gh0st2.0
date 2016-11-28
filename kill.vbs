EndProcess "lx.exe"
EndProcess "loader.exe"
EndProcess "mydll.exe"

Sub EndProcess(ExeName)
    Set wshShell = WScript.CreateObject("WScript.Shell")
	wshShell.run "taskkill.exe /f /im " & ExeName,0
    Set wshSHell = Nothing
End Sub
