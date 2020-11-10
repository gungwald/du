# du
A Windows implementation of the Linux or UNIX "du" command. The name is a shorthand for "disk usage". It is a command line program that displays the disk space used by the files or directories that you specify. For directories, (or folders, in Microsoft terminology) it can display the cumulative size of all files and directories contained in the directory. The currently available Windows versions of this program are ported from Linux and do not display correct results.

The code is written to be as fast and efficient as possible. It uses the Win32 API which is the fastest available programming interface for Windows. The Win32 API also allows it to work on older systems. It is intended to work on Wine and ReactOS. If it doesn't work on a Windows system, that is a bug.

It also has no dependencies. For example, it does not require a Visual C++ or Visual Basic runtime library. So you can drop the du.exe file on any Windows system and it will run.
