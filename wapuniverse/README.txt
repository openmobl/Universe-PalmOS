Universe version 3.0.0

http://www.openmobl.com/

Copyright (c) 2007 OpenMobl Systems
Copyright (c) 2006 Donald C. Kirker
Portions Copyright (c) 1999-2006 Filip Onkelinx

INTRODUCTION:
=============
Universe is the first PalmOS that is publically available that supports tabbed browsing. 
Universe supports compiled WML pages as well as HTML, XHTML and WML 2.0 pages. Universe
also supports GIF, JPEG, WBMP, PNG, BMP and PalmBMP images embedded in pages. Universe currently
operates over the WAP 1.3 and WAP 2.0 protocol set and communicates with a WAP gateway provided by
a public provider or wireless carrier. Universe also supports WAP 2.0 and HTTP over TCP/IP.

REQUIREMENTS:
=============
PalmOS version 5.0 or greater
Means of connecting to an IP network through the network panel
~1Mb of free memory (not including memory for bookmarks and connections or memory 
  Universe uses at runtime (up to 1M))

FILES:
=============
iScript.prc
	A PalmOS WMLScript library.

pnoJpegLib.prc
	The JPEG library. It also handles fast image resizing. This is required to display JPEG images and resize images.

GIFLib.prc
        A GIF image decoder.

Universe.prc
	The browser application.

INSTALLING:
=============
If you have a version older than 3.0 you MUST delete it and all connection settings.
Installation is simple. Unzip the prc-file and HotSync (tm) Universe.prc to your handheld
like any other Palm application. If you have a previous version (before 2.0) installed,
you MUST delete the old version !! I recommend deleting any older versions first anyway.

LIBRARIES:
=============
Universe makes use of functions from open source libraries. These libraries are:
iScript available at http://www.taptarget.com/

Copies of the source of these libraries are available upon request in the event that the 
above sources become no longer available.

SOURCE:
=============
The source code is available at:

http://www.wapuniverse.com/


HELP:
=============
Go to http://www.wapuniverse.com/ for help.
For questions email support@openmobl.com.

CHANGES:
=============
See Changelog.txt.

LICENSE:
=============
This program is free software; you can redistribute it and/or modify it under the terms 
of the GNU General Public License as published by the Free Software Foundation; either 
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
See the GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License along with this program; 
if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, 
MA 02111-1307, USA. 

See "LICENSE.TXT"
Please include unmodified copies of "LICENSE.TXT" and "README.TXT" with distributions.

CONTRIBUTING:
=============
To add a contribution just email it to support@wapuniverse.com.

SUBMIT A BUG:
=============
You found a bug? Please report it to support@wapuniverse.com.
You corrected a bug? Great, please submit the patch to support@wapuniverse.com.


Last Update: March 07, 2007 3.0.0 Revision c
