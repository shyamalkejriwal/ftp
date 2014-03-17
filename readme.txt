Date - 17/03/2014
Authors - 
1. Akshay Jajoo 11010104
2. Shyamal Kejriwal 11010174
--------------------------------------

Instructions for compilation:

1. make clean
1. To get 'server' program: make server
	(Note: The 'server' program has following dependencies: server.cpp, server.hpp, connect.cpp, connect.hpp, main_server.cpp, parser.cpp, reply.hpp)

2. To get 'myftp' program(client side): make myftp
	(Note: The 'myftp' program has following dependencies: client.cpp, client.hpp, connect.cpp, connect.hpp, main_client.cpp, parser.cpp, reply.hpp)
-------------------------------------

Instructions for execution:

1. At server side, run "./server"
2. At client side, run "./myftp <server-ip> 5000"

IMPORTANT - If 'bind: failed to bind' error occurs, close the terminal and run the executable again.

If connection is successful after the above steps: 
1. Response at server side:
	Connected to <client-ip>
2. Response at client side:
	Connected to server
	ftp> 

After the ftp> prompt is available, the user(client) can execute the following commands:

1. put <local filename> [remote filename]
	Note - The remote filename is optional. Filepath can be used instead of filename.
2. get <remote filename> [local filename]
	Note - The local filename is optional. Filepath can be used instead of filename.
3. ls [OPTION]... [FILE]...
4. cd <dir-name>
    Note - Dir-path can be used instead of dir-name.
5. pwd
6. !ls [OPTION]... [FILE]...
7. !cd <dir-name>
    Note - dir-path can be used instead of dir-name.
8. !pwd
9. !quit
--------------------------------------

Salient features:

1. Appropriate success and error replies from server along with reply codes (defined in reply.hpp).
2. Appropriate messages and errors from local host.
3. Error messages thrown on invalid commands, invalid or/and fewer arguments, trying to read/write non-existent files, trying to overwrite existing files, violating access permissions, trying to access non-existent directory, etc.
4. Filepath and dir-path can be used instead of filename and dir-name, resp.
5. Filename, filepath, dir-name and dir-path can have white spaces, quotes, backslashes. The following rules are used for parsing command arguments:
	a. Arguments are delimited by white space(s), which is/are either (a) space(s) or (a) tab(s).
	b. A string surrounded by double quotation marks ("string") is interpreted as a single argument, 
	regardless of white space contained within. A quoted string can be embedded in an argument.
	c. A double quotation mark preceded by a backslash (\") is interpreted as a literal double quotation mark character (").
	d. Any character 'ch' preceded by a backslash (\ch) is interpreted as a literal character 'ch' (including '\').

6. The server sends the size of file transfer along with success reply.
7. All options supported with 'ls'.
-----------------------------------------

Brief descriptions of source files:

1. connect.hpp and connect.cpp
	These file contain the definitions for class 'connection' and class 'myftp-data'. The class 'connection' handles all connection setups and requests. The class 'myftp-data' handles transfer of files. These classes are used by both server and client programs.

2. reply.hpp
	This file contains the definitions for success and error replies and also reply codes from server.

3. parse.cpp
	This file contains the function to parse the command arguments as per rules described above.

4. client.hpp and client.cpp
	These files contain the definition of class 'client'. This class inherits class 'connection'(1).

5. server.hpp and server.cpp
	These files contain the definition of class 'server'. This class inherits class 'connection'(1).

6. main_server.cpp and main_client.cpp
	These files contain main functions for server and client, respectively.
--------------------------------------------