<!--

// pre-load all images into cache
var fillerImg = new Image(1,1)
fillerImg.src = "filler.gif"
var collapsedImg = new Image(widgetWidth,widgetHeight)
collapsedImg.src = collapsedWidget
var expandedImg = new Image(widgetWidth,widgetHeight)
expandedImg.src = expandedWidget
var endpointImg = new Image(widgetWidth,widgetHeight)
endpointImg.src = endpointWidget

// ** functions that get and set persistent cookie data **
// set cookie data
var mycookie = document.cookie
function setCurrState(setting) {
        mycookie = document.cookie = "currState=" + escape(setting)
}
// retrieve cookie data
function getCurrState() {
        var label = "currState="
        var labelLen = label.length
        var cLen = mycookie.length
        var i = 0
        while (i < cLen) {
                var j = i + labelLen
                if (mycookie.substring(i,j) == label) {
                        var cEnd = mycookie.indexOf(";",j)
                        if (cEnd ==     -1) {
                                cEnd = mycookie.length
                        }
                        return unescape(mycookie.substring(j,cEnd))
                }
                i++
        }
        return ""
}

// **function that updates persistent storage of state**
// toggles an outline mother entry, storing new value in the cookie
function toggle(n) {
	if (n != 0) {
		var newString = ""
		var currState = getCurrState() // of whole outline
		var expanded = currState.charAt(n-1) // of clicked item
		newString += currState.substring(0,n-1)
		newString += expanded ^ 1 // Bitwise XOR clicked item
		newString += currState.substring(n,currState.length)
		setCurrState(newString) // write new state back to cookie
	}
}

// **functions used in assembling updated outline**
// returns the proper GIF file name for each entry's control
function getGIF(n) {
	var mom = db[n].mother  // is entry a parent?
	var expanded = getCurrState().charAt(n-1) // of clicked item
	if (!mom) {
		return endpointWidget
	} else {
		if (expanded == 1) {
			return expandedWidget
		}
	}
	return collapsedWidget
}

// returns the proper status line text based on the icon style
function getGIFStatus(n) {
	var mom = db[n].mother  // is entry a parent
	var expanded = getCurrState().charAt(n-1) // of rolled item
	if (!mom) {
		return "No further items"
	} else {
		if (expanded == 1) {
			return "Click to collapse nested items"
		}
	}
	return "Click to expand nested items"
}

// initialize 'current state' storage field
if (getCurrState() == "" || getCurrState().length != (db.length-1)) {
	initState = ""
	for (i = 1; i < db.length; i++) {
		initState += "0"
	}
	setCurrState(initState)
}

// end -->

