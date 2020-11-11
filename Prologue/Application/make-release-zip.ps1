$scriptPath = $MyInvocation.MyCommand.Path

$parentPath = Split-Path -Parent $scriptPath

$From = $parentPath + "\Release"
$To = $parentPath + "\Prologue.zip"

Compress-Archive -Path $From -DestinationPath $To -Force