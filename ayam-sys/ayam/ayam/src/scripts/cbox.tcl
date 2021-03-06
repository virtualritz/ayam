# Ayam, save array: CBoxAttrData
# cbox.tcl: example script for an Ayam Script object;
# this script wants Script object type "Create" and creates
# a cylindrical box; it also has a property GUI, just
# add a tag "NP CBoxAttr" to the Script object to see it
if { ![info exists ::CBoxAttrData] } {
    array set ::CBoxAttrData {
	Width 1.0
	Depth 1.0
	Height 1.0
	SP {Width Depth Height}
    }
}
if { ![info exists ::CBoxAttrGUI] } {
    set w [addPropertyGUI CBoxAttr "" ""]
    addVSpace $w s1 2
    addParam $w CBoxAttrData Width
    addParam $w CBoxAttrData Depth
    addParam $w CBoxAttrData Height
}

set wh [expr $CBoxAttrData(Width) * 0.5]
set wm [expr -$wh]
set lh [expr $CBoxAttrData(Depth) * 0.5]
set lm [expr -$lh]
set hh [expr $CBoxAttrData(Height) * 0.5]
set hm [expr -$hh]

set cv ""
lappend cv 0.0 $hm 0.0 1.0
lappend cv $wm $hm $lm 1.0
lappend cv $wm $hh $lm 1.0
lappend cv 0.0 $hh 0.0 1.0

lappend cv 0.0 $hm 0.0 1.0
lappend cv $wh $hm $lm 1.0
lappend cv $wh $hh $lm 1.0
lappend cv 0.0 $hh 0.0 1.0

lappend cv 0.0 $hm 0.0 1.0
lappend cv $wh $hm $lh 1.0
lappend cv $wh $hh $lh 1.0
lappend cv 0.0 $hh 0.0 1.0

lappend cv 0.0 $hm 0.0 1.0
lappend cv $wm $hm $lh 1.0
lappend cv $wm $hh $lh 1.0
lappend cv 0.0 $hh 0.0 1.0

lappend cv 0.0 $hm 0.0 1.0
lappend cv $wm $hm $lm 1.0
lappend cv $wm $hh $lm 1.0
lappend cv 0.0 $hh 0.0 1.0

crtOb NPatch -width 5 -height 4 -uorder 2 -vorder 2 -cn cv

# cbox.tcl
