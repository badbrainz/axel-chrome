This is a clone of Axel 2.4. It has been modifed to support Google Chrome
Native Host Messaging. It is not meant to be used from the command line.

Original author: 
    Wilmer van der Gaast. <wilmer@gaast.net>

Axel Home:
    See http://axel.alioth.debian.org/ for latest information on axel
    

### Supported architectures
Should compile on any decent Linux system. Additionaly, it should compile
(and run) on BSD, Solaris, Darwin (Mac OS X) and Win32 (Cygwin) systems.
## How to install/use
### Configure/build axel
Run the configure script (you can supply some options if you want, try
'./configure --help' for more info) and then run make. The program should
compile then. There are no special requirements for Axel. You can install
the program using 'make install' or you can just run it from the current
directory. You can copy the axelrc.example file to ~/.axelrc then, if you
want to change some of the settings.
### Build the Chrome extension
To build the extension, you'll need the extension ID that is generated
by Chrome. cd to the gui/chrome directory and run the configure and build
scripts. 
```
cd gui/chrome
./configure
./build
```
Next, install the extension, copy the ID from chrome://extensions and
paste it into the configure script, and then run the configure and build scripts
again. *This is the only method I know of that gives us the extension ID.*
### Configure native messaging host
The last step is to register the native host app by running the install script.
```
./install
```
