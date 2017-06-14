
inputstring = Session.Property("CustomActionData")
'inputstring = "1<>C:\Program Files\NCAR\"
' the inputstring either starts with <>, or it has a "1<>" at the start.
' If it begins with a "1", then it was an "ALLUSER" install
posn = inStr(inputstring, "<>")
leftSide = Left(inputstring, posn-1)
if leftSide = "1" Then
	allUserProp = true
Else
	allUserProp = false

End if
vaporhome = Right(inputstring, len(inputstring) - posn -1)
vaporhome = vaporhome & "VAPOR"
vaporbin = vaporhome & "\bin;"
vaporshare = vaporhome & "\share"
vaporidl = vaporhome & "\bin"
vaporidl2 = vaporhome & "\bin;"

vaporpythonhome = vaporhome & "\lib\python2.6"

set shell = CreateObject("wscript.shell")
If allUserProp Then
	set sysEnv = shell.Environment("SYSTEM")
Else
	set sysEnv = shell.Environment("USER")

End If

'  unset the VAPOR_HOME variable:
envVar = sysEnv("VAPOR_HOME")
if (Len(envVar) > 0) then
    sysEnv.Remove("VAPOR_HOME")
end if
envVar = sysEnv("VAPOR_SHARE")
if (Len(envVar) > 0) then
    sysEnv.Remove("VAPOR_SHARE")
end if

'  Find VAPOR_HOME\bin; in the path
pathvar = sysEnv("path")
posn = inStr(pathvar,vaporbin)
if  posn <> 0 Then
	pathvar = Replace(pathvar, vaporbin, "")
	SysEnv("path") = pathvar
End If

'Find vaporhome in the PYTHONHOME.  If so, unset PYTHONHOME
pathvar = sysEnv("PYTHONHOME")
if (Len(pathvar) > 0) Then
    posn = inStr(pathvar,vaporhome)
    if  posn <> 0 Then
	    sysEnv.Remove("PYTHONHOME")
    End If
End If

idlpath = sysenv("IDL_DLM_PATH")
' the dlm path may either be followed by a : or not:
posn = inStr(idlpath,vaporidl2)
if  posn <> 0 Then
	idlpath = Replace(idlpath, vaporidl2, "")
	SysEnv("IDL_DLM_PATH") = idlpath
Else 
	posn = inStr(idlpath, vaporidl)
	if posn = 0 Then
		MsgBox("Unable to remove VAPOR from IDL_DLM_PATH")
	Else
		idlpath = Replace(idlpath, vaporidl, "")
		if idlpath = "" Then
			SysEnv.Remove("IDL_DLM_PATH")
		Else
			SysEnv("IDL_DLM_PATH") = idlpath
		End If
	End If
End If

Set fso = CreateObject("Scripting.FileSystemObject")

'Delete shortcuts to vaporgui
DesktopPath = shell.SpecialFolders("Desktop")
if (fso.FileExists(DesktopPath & "\vaporgui.lnk")) then
    fso.DeleteFile (DesktopPath & "\vaporgui.lnk")
end if
    
if(allUserProp) then
    ProgramPath = shell.SpecialFolders("AllUsersPrograms")
else
    ProgramPath = shell.SpecialFolders("Programs")
end if

if (fso.FileExists(ProgramPath & "\vaporgui.lnk")) then
    fso.DeleteFile (ProgramPath & "\vaporgui.lnk")
end if

Set fso = Nothing