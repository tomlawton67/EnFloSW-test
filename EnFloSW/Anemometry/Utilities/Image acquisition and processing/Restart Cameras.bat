@ECHO OFF

REM Windows Developer Kit (WDK) must first be installed, with the directory containing devcon.exe added to the "Path" system variable (not the "PATH" environment variable!)
REM repeat the three lines below for each camera, with Hardware ID extracted from device manager-->details
REM Note cameras may share Hardware ID, but will have unique instance IDs. This is taken care of automatically, so for cameras that share Hardware IDs, simply have one Hardware ID to restart them all
REM the ID is typically of the form USB\VID_xxxx&PID_xxxxyyyy.... all the y's (and everything thereafter) must be excluded and replaced with a single asterisk. Use the existing lines in this file as an example
REM it is essential that there are no spaces inserted either side of the "=" in the "set" command
REM also the double quotation mark and asterisk must be placed at the start and end of the ID respectively
REM to preserve the output for inspection, change the /c to /k

REM TunnelCam, TurntableCam
set CamID="USB\VID_046D&PID_082D*
cmd /c devcon restart %CamID%

REM SharpieCam
set CamID="USB\VID_EB1A&PID_299F*
cmd /c devcon restart %CamID%