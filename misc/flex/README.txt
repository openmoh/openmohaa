=======
versions 2.4.3/2.5.3
--------------
fix incorrect #line directives in win_flex.exe
see https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=542482

=======
versions 2.4.2/2.5.2
--------------
backport parallel invocations of win_bison version 2.7
win_bison of version 3.0 is unchanged

versions 2.4.1/2.5.1
--------------
remove XSI extention syntax for fprintf function (not implemented in windows)
this fixes Graphviz files generation for bison

NOTE:
2.4.x versions will include bison version 2.7
2.5.x versions will include bison version 3.0

version 2.5
--------------
upgrade win_bison to version 3.0 and make temporary win_bison's files process unique (so parallel invocations of win_bison are possible)

NOTE: There are several deprecated features were removed in bison 3.0 so this version can break your projects.
Please see http://savannah.gnu.org/forum/forum.php?forum_id=7663
For the reason of compatibility I don't change win_flex_bison-latest.zip to refer to win_flex_bison-2.5.zip file.
It still refer to win_flex_bison-2.4.zip 

version 2.4
--------------
fix problem with "m4_syscmd is not implemented" message. Now win_bison should output correct 
diagnostic and error messages.

version 2.3
--------------
hide __attribute__ construction for non GCC compilers

version 2.2
--------------
added --wincompat option to win_flex (this option changes <unistd.h> unix include with <io.h> windows analog
  also isatty/fileno functions changed to _isatty/_fileno)
fixed two "'<' : signed/unsigned mismatch" warnings in win_flex generated file

version 2.1
--------------
fixed crash when execute win_bison.exe under WindowsXP (argv[0] don't have full application path)
added win_flex_bison-latest.zip package to freeze download link

version 2.0
--------------
upgrade win_bison to version 2.7 and win_flex to version 2.5.37

version 1.2
--------------
fixed win_flex.exe #line directives (some #line directives in output file were with unescaped backslashes)

version 1.1
--------------
fixed win_flex.exe parallel invocations (now all temporary files are process specific)
added FLEX_TMP_DIR environment variable support to redirect temporary files folder
added '.exe' to program name in win_flex.exe --version output (CMake support)
fixed win_bison.exe to use /data subfolder related to executable path rather than current working directory
