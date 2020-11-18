@echo off
powershell Compress-Archive -LiteralPath '..\Debug\du.exe', '..\Debug\install.exe'					-DestinationPath "Disk_Usage_for_Windows_x86_32-bit_Debug.zip"
powershell Compress-Archive -LiteralPath '..\Release\du.exe', '..\Release\install.exe'				-DestinationPath "Disk_Usage_for_Windows_x86_32-bit.zip"
powershell Compress-Archive -LiteralPath '..\x64\Release\du.exe', '..\x64\Release\install.exe'		-DestinationPath "Disk_Usage_for_Windows_x64.zip"
powershell Compress-Archive -LiteralPath '..\ARM\Release\du.exe', '..\ARM\Release\install.exe'		-DestinationPath "Disk_Usage_for_Windows_ARM32.zip"
powershell Compress-Archive -LiteralPath '..\ARM64\Release\du.exe', '..\ARM64\Release\install.exe'	-DestinationPath "Disk_Usage_for_Windows_ARM64.zip"
