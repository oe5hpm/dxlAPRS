readme.txt               Copyright (c) 1999-2011 Excelsior, LLC.
----------------------------------------------------------------

                 Native XDS-x86 Linux edition
                 ----------------------------
                            v2.60

                        Read Me First!


1. Overview
-----------

Your XDS setup package includes three products:
  - Native XDS-x86,
  - XDS-C and
  - TopSpeed Compatibility Pack.

Native XDS-x86 for Linux is a highly optimising ISO Modula-2 and 
Oberon-2 compiler targeting Intel x86 systems running Linux.

XDS-C is a "via C" Modula-2/Oberon-2 cross-compiler. Its output is 
K&R C, ANSI C, or C++ source code, which may be further compiled by 
a third-party C/C++ compiler for any target platform. 

TopSpeed Compatibility Pack (TSCP) is an add-on for XDS compilers 
making them more compatible with the TopSpeed Modula-2 compiler, 
thus simplifying porting of legacy TopSpeed Modula-2 sources.


2. Installation
---------------

Under Unix, your XDS package is usually installed into the directory
"/usr/local/xds".  The location is not "hard-wired" into the executables,
so you are free to move the executables to other directories. These
notes assume the default setup.

After installation, edit your path environment variable to append path
to the "/usr/local/xds/bin" directory.

Example bash syntax:

    export XDSDIR=/usr/local/xds
    export PATH=$PATH:$XDSDIR/bin


2.1. Directory Structure
------------------------

The following directory structure is created during installation:

xds/
    bin             executable and system files
    C/               library source code:
        C               XDS-C
        x86             Native XDS-x86
    def/            definition modules:
        iso             ISO Modula-2 library
        ob2             Oberon-2 library pseudo-definition modules
        pim             PIM library
        POSIX           POSIX API
        ts              TopSpeed-like library
        X11             X11 API
        xds             XDS library
        Xm              Motif API
    doc             documentation (HTML)
    include/        library include files:
        C               XDS-C
        x86             Native XDS-x86
    lib/            libraries:
        C               XDS-C
        x86             Native XDS-x86
    licenses        license agreements
    pdf             printable documentation (PDF)
    readme          useful texts
    samples         Modula-2/Oberon-2 code samples
    sym/            symbol files:
        C               XDS-C
        x86             Native XDS-x86


2.2. Building the library
-------------------------

The Native XDS-x86 and XDS-C runtime libraries were built on top 
of glibc 2 and thus executables created by this product require 
glibc 2 shared libraries unless you force static linking of C 
libraries:

    -usestaticclibs:+

The runtime library may have to be rebuilt on Linux systems that
use C library other than glibc 2.

To build the Native XDS-x86 library, issue the following commands:

    cd /usr/local/xds/lib/x86
    rm libxds.a libts.a
    make -f lib.mkf
    make -f tslib.mkf


To rebuild the XDS-C runtime library, issue the following commands:

    cd /usr/local/xds/lib/C
    rm libxds.a libts.a
    make -f lib.mkf
    make -f tslib.mkf

To build the XDS runtime library with a C cross-compiler, copy
and edit the file /usr/local/xds/lib/C/lib.mkf.


2.3. Building API symbol files
------------------------------

Your XDS package contains definition modules for POSIX and X11/Motif
APIs. To build the respective symbol files for Native XDS-x86, issue 
the following commands:

    cd /usr/local/xds
    ./apisyms-x86

To build the respective symbol files for XDS-C, issue the following
commands:

    cd /usr/local/xds
    ./apisyms-c
     

3. Creating the Working Directory
---------------------------------

The "xcwork" shell script (/usr/local/xds/bin/xcwork) may
help you to create the working directory structure for your
Native XDS-x86 project, for example

    mkdir wrk
    cd wrk
    xcwork

The "xmwork" shell script (/usr/local/xds/bin/xmwork) may
help you to create the working directory structure for your
XDS-C project, for example

    mkdir wrk
    cd wrk
    xmwork


4. Creating X11/Motif applications
----------------------------------

4.1. Documentation
------------------

Your XDS package includes the X11/Motif API definition modules 
and support files, but no documentation on these APIs. You have
to use third-party books or the on-line documentation.
We regret the inconvenience.


4.2. Using the X11/Motif APIs in your programs
----------------------------------------------

X11 and Motif definition modules reside in the `def/X11' and
`def/Xm' subdirectories of your XDS installation, resspectively. 
Use the `apisyms-c' script in the root of your XDS installation to
create symbol files for these APIs.

If your program is an X11 application, toggle the XAPP option
ON in the project file or on the command line:

    +XAPP

If your program is a Motif application, toggle the XMAPP option
ON in the project file or on the command line:

    +XMAPP

Note: These options are only used to set up additional include 
paths and libraries during processing of the template file, 
`bin/xm.tem'. That file contains defaults for Solaris, HP-UX,
and Linux. If the X11/Motif include files and libraries reside
in different directories on your target system, you have to edit
the default template file or to use your own modified copy in your 
projects.


4.3. Declaring X callbacks
--------------------------

You have to specify the "C" calling and naming convention for
your X callback procedures:

PROCEDURE ["C"] QuitCB (
                    w: Xt.Widget;
                    client_data: Xt.XtPointer;
                    call_data: Xt.XtPointer);



5. Contact information
----------------------

Send your bug reports, questions, and comments to: 

support@excelsior-usa.com

For latest information about new products, releases, updates, 
and fixes, please visit our Web page at: 

http://www.excelsior-usa.com/


6. See also
-----------

    report.txt       bug reporting guidelines
    samples.txt      sample programs description
    whatsnew.txt     what is new in this release

                         [end of document]
