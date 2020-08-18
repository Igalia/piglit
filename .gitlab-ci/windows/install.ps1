# we want more secure TLS 1.2
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12;

Get-Date
Write-Host "Installing Chocolatey"
Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
Import-Module "$env:ProgramData\chocolatey\helpers\chocolateyProfile.psm1"
Update-SessionEnvironment
Write-Host "Installing Chocolatey packages"
choco install --allow-empty-checksums -y cmake --installargs "ADD_CMAKE_TO_PATH=System"
choco install --allow-empty-checksums -y python3 git ninja

# Add Chocolatey's native install path
Update-SessionEnvironment
# Python and CMake add themselves to the system environment path, which doesn't get refreshed
# until we start a new shell
$env:PATH = "C:\python3;C:\python3\scripts;C:\Program Files\CMake\bin;$env:PATH"

Start-Process -NoNewWindow -Wait git -ArgumentList 'config --global core.autocrlf false'

Get-Date
Write-Host "Installing Python PIP packages"
pip3 install mako numpy
if (!$?) {
  Write-Host "Failed to install dependencies from pip"
  Exit 1
}

# VS16.x is 2019
$msvc_2019_url = 'https://aka.ms/vs/16/release/vs_buildtools.exe'

Get-Date
Write-Host "Downloading Visual Studio 2019 build tools"
Invoke-WebRequest -Uri $msvc_2019_url -OutFile C:\vs_buildtools.exe

Get-Date
Write-Host "Installing Visual Studio 2019"
Start-Process -NoNewWindow -Wait C:\vs_buildtools.exe -ArgumentList '--wait --quiet --norestart --nocache --installPath C:\BuildTools --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.VC.ATL --add Microsoft.VisualStudio.Component.VC.ATLMFC --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Graphics.Tools --add Microsoft.VisualStudio.Component.Windows10SDK.18362 --includeRecommended'
if (!$?) {
  Write-Host "Failed to install Visual Studio tools"
  Exit 1
}
Remove-Item C:\vs_buildtools.exe -Force

Get-Date
Write-Host "Downloading Freeglut"
$freeglut_zip = 'freeglut-MSVC.zip'
$freeglut_url = "https://www.transmissionzero.co.uk/files/software/development/GLUT/$freeglut_zip"
Invoke-WebRequest -Uri $freeglut_url -OutFile $freeglut_zip
if (!$?) {
  Write-Host "Failed to download Freeglut"
  Exit 1
}

Get-Date
Write-Host "Installing Freeglut"
Expand-Archive $freeglut_zip -DestinationPath C:\
if (!$?) {
  Write-Host "Failed to install Freeglut"
  Exit 1
}

Get-Date
Write-Host "Downloading glext.h"
New-Item -ItemType Directory -Path "C:\glext" -Name "GL"
$ProgressPreference = "SilentlyContinue"
Invoke-WebRequest -Uri 'https://www.khronos.org/registry/OpenGL/api/GL/glext.h' -OutFile 'C:\glext\GL\glext.h' | Out-Null

Get-Date
Write-Host "Complete"
