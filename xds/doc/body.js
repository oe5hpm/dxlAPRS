<!-- start
// build new outline based on the values of the cookie
// and data points in the outline data array.
// This fires each time the user clicks on a control,
// because the HREF for each one reloads the current document.
var newOutline = ""
var prevIndentDisplayed = 0
var showMyDaughter = 0
// cycle through each entry in the outline array
for (var i = 1; i < db.length; i++) {
	var theGIF = getGIF(i)				// get the image
	var theGIFStatus = getGIFStatus(i)  // get the status message
	var currIndent = db[i].indent		// get the indent level
	var expanded = getCurrState().charAt(i-1) // current state
	// display entry only if it meets one of three criteria
	if (currIndent == 0 || currIndent <= prevIndentDisplayed || (showMyDaughter == 1 && (currIndent - prevIndentDisplayed == 1))) {
		newOutline += "<NOBR><IMG SRC=\"filler.gif\" HEIGHT = 1 WIDTH =" + (indentPixels * currIndent) + ">"
		newOutline += "<A HREF=\"javascript:history.go(0)\" onMouseOver=\"window.status=\'" + theGIFStatus + "\';return true;\" onClick=\"toggle(" + i + ");return " + (theGIF != endpointWidget) + "\">"
		newOutline += "<IMG SRC=\"" + theGIF + "\" HEIGHT=" + widgetHeight + " WIDTH=" + widgetWidth + " BORDER=0></A>"		
		if (db[i].URL == "" || db[i].URL == null) {
			newOutline += " " + db[i].display + "</NOBR><BR>"	// no link, just a listed item	
		} else {
			newOutline += " <A HREF=\"" + db[i].URL + "\" TARGET=\"" + displayTarget + "\" onMouseOver=\"window.status=\'" + db[i].statusMsg + "\';return true;\">" + db[i].display + "</A></NOBR><BR>"
		}
		prevIndentDisplayed = currIndent
		showMyDaughter = expanded
		if (db.length > 25) {
			document.write(newOutline)
			newOutline = ""
		}
	}
}
document.write(newOutline)

// end -->

