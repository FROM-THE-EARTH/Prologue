$scriptPath = $MyInvocation.MyCommand.Path

$parentPath = Split-Path -Parent $scriptPath

$From = $parentPath + "\Prologue"
$To = $parentPath + "\Prologue.zip"

Compress-Archive -Path $From -DestinationPath $To -Force