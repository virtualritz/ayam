# tsurf.tcl: example script for Ayam Script object
# this script wants Script Object Type "Modify"
# and creates a translational surface from two NURBS curves

convOb -inplace NCurve

selOb 0
getProp
set w $NCurveAttrData(Length)
set uo $NCurveAttrData(Order)
set ukt $NCurveAttrData(Knot-Type)
set ukv $NCurveAttrData(Knots)
set c0 ""
getPnt -trafo -all c0

selOb 1
getProp
set h $NCurveAttrData(Length)
set vo $NCurveAttrData(Order)
set vkt $NCurveAttrData(Knot-Type)
set vkv $NCurveAttrData(Knots)
set c1 ""
getPnt -trafo -all c1

set cv ""
set i 0
while { $i < $w } {
    set j 0
    while { $j < $h } {
	set ii [expr {$i * 4}]
	set jj [expr {$j * 4}]
	for {set k 0} {$k < 3} {incr k} {
	    set p0 [lindex $c0 $ii]
	    set p1 [lindex $c1 $jj]
	    lappend cv [expr {$p0+$p1}]
	    incr ii
	    incr jj
	}
	incr ii
	# XXXX TODO: fix/improve weight handling
	lappend cv 1.0
	incr j
    }
    incr i
}

crtOb NPatch -uo $uo -vo $vo -wi $w -he $h -ukt $ukt -vkt $vkt\
    -un ukv -vn vkv -cn cv

selOb 0 1
delOb

# tsurf.tcl
