# escape=`

FROM mcr.microsoft.com/windows:1809

# Make sure any failure in PowerShell scripts is fatal
SHELL ["powershell", "-ExecutionPolicy", "RemoteSigned", "-Command", "$ErrorActionPreference = 'Stop';"]
ENV ErrorActionPreference='Stop'

COPY install.ps1 C:\
RUN C:\install.ps1
