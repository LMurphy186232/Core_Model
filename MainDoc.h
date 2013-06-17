//The only purpose of this file is to be the main documentation page.  I put
//it here like this so Doxygen will catch it.

/**
* @mainpage SORTIE
*
* @section running Running the Code
* If you have the SORTIE.exe executable, you can run it directly in two ways.
* If you launch the executable directly, either on a command line with no
* arguments or by double-clicking it, it will launch a DOS interface.  To
* input a file, type "input " + the filename. This file can be a parameter file,
* a batch file, or a map file to be added to a parameter file you already
* entered.  If the file reads correctly, the following appears:
* <code>Model Status: Ready to run.</code>
* To run, type "run". If you want to specify a number of timesteps to run, you
* can type the number after the word "run." To enter another file, type
* "input " followed by the path and name of the parameter file.  For a list of
* all commands, type "help". To end, type "quit".
*
* You can also run SORTIE from the command line by adding as an argument the
* name of the parameter or batch file you wish it to run.  For example,
* <code>c:\\sortie\\sortie.exe myparamfile.xml</code>
*
* When running from either method, SORTIE will update its status via messages
* to <code>stdout</code>, which you will see in the DOS window.
*/
