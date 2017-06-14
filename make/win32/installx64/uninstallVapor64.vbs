
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

vaporbin = vaporhome & "bin;"
vaporshare = vaporhome & "share"

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
