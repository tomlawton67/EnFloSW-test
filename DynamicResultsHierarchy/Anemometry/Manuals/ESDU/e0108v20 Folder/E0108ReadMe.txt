ESDU Program: Wind and turbulence properties. Excel module E0108v10.xls Version 1.0 December 2001
Version 1.1 April 2002
Version 1.2 May 2003

The program E0108, which supersedes program A9232 previously available, is written in Visual Basic and runs as a module in Microsoft Excel 97 (or later).  The code that runs the Excel module is hidden to ensure that only authorised changes can be made.  If access is to the code is required, contact ESDU International for a password.

The program is supplied as file E0108vxx.zip, where vxx indicates the program version (e.g., v1.0, v1.1, etc.).  It is supplied in a zipped format that will be automatically unzipped if installed using ESDUview, or the file can be unzipped and copied to an appropriate location.

The digits 0108 in the file name relate to  ESDU Data Item 01008 which provides details of the Excel module program and its use. Tables 3.1 and 3.2 in ESDU 01008 provide a summary guide to using the Excel module.

The program can be run by starting Excel and opening file E0108vxx.xls using File..Open, or by double-clicking the file name. The default ESDUview installation locates the file in C:\ESDU\ESDUpacs\E0108vxx.

It is strongly recommended that, each time the program is run, the file is saved (using File..Save As) under a new name reflecting the particular application or site location.  This will ensure that the original program file is kept unchanged for future use.  Input data and results from particular runs can be copied to other independent Excel Worksheets for future reference which also avoids saving large files. 

Alternatively, the original file can be saved as an Excel template file (E0108vxx.xlt) in the XLstart directory/folder (within the same directory as Excel.exe).  The program can then be run by starting Excel and using File..New to select the required template.  This procedure opens a separate instance of the program each time with 1, 2, 3, etc appended to the basic file name.

Amendments at version 1.1
Correction made to sort (if necessary) hill input data into correct order for calculation.
Iterative calculation corrected for cases where reference wind speed applies to site with hilly terrain.
Positive or negative latitudes now accepted.

Amendments at version 1.2
To cope with situations where hill height is significant compared to boundary-layer height (i.e. significant blockage), it is necessary to increase the vertical grid size for computation of speed-up factors over the hill. This is determined automatically by the program.

Amendments at version 2.0
The principal amendment provides a more rational procedure for dealing with more than one terrain roughness change upwind of a site. Starting with the roughness change furthest upwind from the site, values of the local parameters are calculated at the start of the next roughness change using the method for a single roughness change. These values are then used as the input parameters for the next change and so on until all changes have been accounted for.  Also, an anomaly in the basic method for wind properties over a roughness change was found to give spurious results at larger heights, and this has been corrected. The new calculation procedure will give the same results as the previous version for a single roughness change but differences of up to about 10 percent can accrue for multiple changes depending on the terrain characteristics. Also, some minor changes have been made to the module dealing with topographic effects.
