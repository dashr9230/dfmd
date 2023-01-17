# dfmd
A tool for deobfuscating Delphi 7 RCDATA

## Usage
1. Open an Delphi 7 compiled executable with a file archive software, like 7-Zip by right click on the .exe, then choose 'Open archive'.
2. Go to `.rsrc\0\RCDATA`
3. Extract files that are starts with `TFM`
4. Open up your Command Prompt, and type `dfmd.exe <your TFM file name> > <output file name>`, and press enter.
  Example: If you want to deobfuscate TFMMAIN into a `*.txt` file, then it will be `dfmd.exe TFMMAIN > TFMMAIN.export.txt`
  Or: You can simply pass the file name, and it will print it out in the console like this: `dfmd.exe TFMMAIN`
5. Done.

## Things that are not working
1. Floating numbers for charts and stuffs.
2. There maybe more issues that i'm not aware of, that's the reason why it has TODOs placed in some cases.
