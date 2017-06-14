
inputstring = Session.Property("CustomActionData")

' the inputstring either starts with <>, or it has a "1<>" at the start.
' If it begins with a "1", then it's an "ALLUSER" install

posn = inStr(inputstring, "<>")

leftSide = Left(inputstring, posn-1)

if leftSide = "1" Then
	allUserProp = true
Else
	allUserProp = false
End if

vaporhome = Right(inputstring, len(inputstring) - posn -1)
vaporshare = vaporhome & "\share"
vaporbin = vaporhome & "\bin"

set shell = CreateObject("wscript.shell")

'If allUserProp Then
	set sysEnv = shell.Environment("SYSTEM")
'Else
'	set sysEnv = shell.Environment("USER")
'End If

SysEnv("VAPOR3_HOME") = vaporhome
SysEnv("VAPOR3_SHARE") = vaporshare

'Create shortcuts on Desktop and Program Menu

DesktopPath = shell.SpecialFolders("Desktop")
Set link = shell.CreateShortcut(DesktopPath & "\vaporgui.lnk")
link.Description = "Vaporgui"
link.IconLocation = vaporhome & "\vapor-win-icon.ico"
link.TargetPath = vaporhome & "\bin\vaporgui.exe"
link.WindowStyle = 1
link.WorkingDirectory = vaporhome
link.Save
if (allUserProp) then
    LinkPath = Shell.SpecialFolders("AllUsersPrograms")
else
    LinkPath = Shell.SpecialFolders("Programs")
end if

Set link = Shell.CreateShortcut(LinkPath & "\vaporgui.lnk")
link.Description = "Vaporgui"
link.IconLocation = vaporhome &"\vapor-win-icon.ico"
link.TargetPath = vaporhome & "\bin\vaporgui.exe"
link.WindowStyle = 1
link.WorkingDirectory = vaporhome
link.Save
