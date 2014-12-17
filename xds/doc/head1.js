<!-- 
//    -------------------------------------------------------------------------
//                OUTLINE STYLE TABLE OF CONTENTS in JAVASCRIPT
//                                Version 2.0
//                    by Danny Goodman (dannyg@dannyg.com)
//                    Analyzed and described at length in
//                             "JavaScript Bible"
//                              by Danny Goodman
//                      (IDG Books ISBN 0-7645-3022-4)
//    
//    This program is Copyright 1996 by Danny Goodman.  You may adapt
//    this outliner for your Web pages, provided these opening credit
//    lines (down to the lower dividing line) are in your outliner HTML document.
//    You may not reprint or redistribute this code without permission from 
//    the author.
//    -------------------------------------------------------------------------
//     21 February 1997  Added intelligence so that clicking on a fully nested
//                       widget icon does not cause the outline to reload. 
//                       
//     24 October 1997   Script speedup for Navigator 4, reduces access to cookie
// -->
<!--

// size of horizontal indent per level
var indentPixels = 20

// art files and sizes for three widget styles
// (all three widgets must have same height/width)
var collapsedWidget = "plus.gif"
var expandedWidget = "minus.gif"
var endpointWidget = "end.gif"
var widgetWidth = 12
var widgetHeight = 12

// Target for documents loaded when user clicks on a link.
// Specify your target frame name here.
var displayTarget = "body"

var db = new Array()


// object constructor for each outline entry
function dbRecord(mother,display,URL,indent,statusMsg){
	this.mother = mother   // is this item a parent?
	this.display = display // text to display
	this.URL = URL         // link tied to text; if empty string, item appears as straight text
	this.indent = indent   // how many levels nested?
	this.statusMsg = statusMsg  // descriptive text for status bar 
	return this
}

// end -->

