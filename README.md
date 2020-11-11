# du
This is a Windows implementation of the UNIX (i.e. Linux, FreeBSD, MacOS X, Solaris) "du" command. The name is a shorthand for "disk usage". It is a command line program that displays the disk space used by the files or directories that you specify. For directories, (or folders, in Microsoft terminology) it can display the cumulative size of all files and directories contained in the directory. The currently available Windows versions of this program are ported from Linux and do not display correct results.

The code is written to be as fast and efficient as possible. It uses the Win32 API which is the fastest available programming interface for Windows. The Win32 API also allows it to work on older systems. It should work on systems as old as Windows 95 and possibly Windows 3.1 with Win32s installed. It is intended to work on Wine and ReactOS. If it doesn't work on a Windows system, that is a bug.

It also has no dependencies. For example, it does not require a Visual C++ or Visual Basic runtime library. So you can drop the du.exe file on any Windows system and it will run.

This builds with Microsoft Visual Studio Community 2019. That is currently the only way to generate executables for Windows ARM. I would like to build it in Fedora Linux with mingw32. That will build the x86 and x64 versions, but not the ARM versions. I'm waiting for a solution for that.

![Example](du-example-run.png)
