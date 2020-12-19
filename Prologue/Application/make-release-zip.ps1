$scriptPath = $MyInvocation.MyCommand.Path

$parentPath = Split-Path -Parent $scriptPath


$originalName = $parentPath + "\Prologue-For-Zip"
$fixedName = $parentPath + "\Prologue"

Rename-Item -Path $originalName $fixedName


$From = $parentPath + "\Prologue"
$To = $parentPath + "\Prologue.zip"

Compress-Archive -Path $From -DestinationPath $To -Force


Rename-Item -Path $fixedName $originalName
