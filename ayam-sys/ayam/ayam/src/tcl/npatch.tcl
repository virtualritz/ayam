# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# npatch.tcl - NURBS patchs objects Tcl code

set NPatch_props { Transformations Attributes Material Tags Bevels Caps NPatchAttr }

array set NPatchAttr {
arr   NPatchAttrData
sproc ""
gproc ""
w     fNPatchAttr
}

array set NPatchAttrData {
Knot-Type_U 1
Knot-Type_V 1
DisplayMode 1
Knots_U ""
Knots_U-Modified 0
Knots_V ""
Knots_V-Modified 0
BoundaryNames { "U0" "U1" "V0" "V1" }
BevelsChanged 0
CapsChanged 0
}

# create NPatchAttr-UI
set w [frame $ay(pca).$NPatchAttr(w)]
addVSpace $w s1 2
addParam $w NPatchAttrData Width
addParam $w NPatchAttrData Height
addParam $w NPatchAttrData Order_U
addParam $w NPatchAttrData Order_V

addMenu $w NPatchAttrData Knot-Type_U [list Bezier B-Spline NURB Custom\
					 Chordal Centripetal]
addString $w NPatchAttrData Knots_U
addMenu $w NPatchAttrData Knot-Type_V [list Bezier B-Spline NURB Custom\
					 Chordal Centripetal]
addString $w NPatchAttrData Knots_V

addCheck $w NPatchAttrData CreateMP

addInfo $w NPatchAttrData IsRat

addParam $w NPatchAttrData Tolerance
addMenu $w NPatchAttrData DisplayMode $ay(npdisplaymodes)

trace variable NPatchAttrData(Knots_U) w markPropModified
trace variable NPatchAttrData(Knots_V) w markPropModified

# npatch_break:
#
#
proc npatch_break { } {
    global ay ayprefs ay_error npatchbrk_options

    if { ! [info exists npatchbrk_options] } {
	set npatchbrk_options(Direction) 0
    }

    winAutoFocusOff

    set npatchbrk_options(oldfocus) [focus]

    set w .npatchbrk
    set t "Break NPatch"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f e1 $t
    }

    addMenu $f npatchbrk_options Direction {"U" "V"}
    addCheck $f npatchbrk_options ApplyTrafo
    addCheck $f npatchbrk_options ReplaceOriginal

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay_error npatchbrk_options
	set cmd "breakNP"
	if { $npatchbrk_options(ApplyTrafo) == 1 } {
	    append cmd " -a"
	}
	if { $npatchbrk_options(ReplaceOriginal) == 1 } {
	    append cmd " -r"
	}
	if { $npatchbrk_options(Direction) == 0 } {
	    append cmd " -u"
	}
	set ay_error ""
	eval $cmd
	if { $ay_error > 1 } {
	    ayError 2 "Break" "There were errors while breaking!"
	} else {
	    if { $npatchbrk_options(ReplaceOriginal) == 1 } {
		uS
		foreach sel [getSel] {
		    if { $ay(lb) == 0 } {
			$ay(tree) selection add $ay(CurrentLevel):$sel
		    } else {
			$ay(olb) selection set $sel
		    }
		}
	    } else {
		uCR
	    }
	    rV; set ay(sc) 1
	}

	grab release .npatchbrk
	restoreFocus $npatchbrk_options(oldfocus)
	destroy .npatchbrk
    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    grab release .npatchbrk;\
	    restoreFocus $npatchbrk_options(oldfocus);\
	    destroy .npatchbrk"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    shortcut_addcshelp $w ayam-5.html breaknpt

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# npatch_break

# npatch_build:
#
#
proc npatch_build { } {
    global ay ayprefs ay_error npatchbld_options

    if { ! [info exists npatchbld_options] } {
	set npatchbld_options(Order_U) 0
	set npatchbld_options(Knot-Type_U) 2
	set npatchbld_options(ApplyTrafo) 1
    }

    winAutoFocusOff

    set npatchbld_options(oldfocus) [focus]

    set w .npatchbld
    set t "Build NPatch"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f e1 $t
    }

    addParam $f npatchbld_options Order_U
    addMenu $f npatchbld_options Knot-Type_U { Bezier B-Spline NURB\
						  Chordal Centripetal }
    addCheck $f npatchbld_options ApplyTrafo
    addCheck $f npatchbld_options ReplaceOriginal

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay_error npatchbld_options
	set cmd "buildNP"
	if { $npatchbld_options(Order_U) != 0 } {
	    append cmd " -o $npatchbld_options(Order_U)"
	}
	set kt $npatchbld_options(Knot-Type_U)
	if { $kt > 2 } {
	    incr kt
	}
	append cmd " -k $kt"
	if { $npatchbld_options(ApplyTrafo) == 1 } {
	    append cmd " -a 1"
	}
	if { $npatchbld_options(ReplaceOriginal) == 1 } {
	    append cmd " -r"
	}

	set ay_error ""
	eval $cmd
	if { $ay_error > 1 } {
	    ayError 2 "Build" "There were errors while building!"
	} else {
	    if { $npatchbld_options(ReplaceOriginal) == 1 } {
		uS
		foreach sel [getSel] {
		    if { $ay(lb) == 0 } {
			$ay(tree) selection add $ay(CurrentLevel):$sel
		    } else {
			$ay(olb) selection set $sel
		    }
		}
	    } else {
		uCR
	    }
	    rV; set ay(sc) 1
	}

	grab release .npatchbld
	restoreFocus $npatchbld_options(oldfocus)
	destroy .npatchbld
    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    grab release .npatchbld;\
	    restoreFocus $npatchbld_options(oldfocus);\
	    destroy .npatchbld"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    shortcut_addcshelp $w ayam-5.html buildnpt

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# npatch_build


# compute suggested new knots for refine knots with operation
proc npatch_getrknotsu { } {
    global ay NPatchAttrData
    set ay(refineknu) ""
    set ui [expr $NPatchAttrData(Order_U) - 1]
    while { $ui < [expr $NPatchAttrData(Width)] } {
	set u1 [lindex $NPatchAttrData(Knots_U) $ui]
	incr ui
	set u2 [lindex $NPatchAttrData(Knots_U) $ui]
	if { $u1 != $u2 } {
	    lappend ay(refineknu) [expr $u1 + ($u2-$u1)*0.5]
	}
    }
    if { [llength $ay(refineknu)] == 1 } {
	lappend ay(refineknu) [lindex $ay(refineknu) 0]
    }
 return;
}
# npatch_getrknotsu


# compute suggested new knots for refine knots with operation
proc npatch_getrknotsv { } {
    global ay NPatchAttrData
    set ay(refineknv) ""
    set vi [expr $NPatchAttrData(Order_V) - 1]
    while { $vi < [expr $NPatchAttrData(Height)] } {
	set v1 [lindex $NPatchAttrData(Knots_V) $vi]
	incr vi
	set v2 [lindex $NPatchAttrData(Knots_V) $vi]
	if { $v1 != $v2 } {
	    lappend ay(refineknv) [expr $v1 + ($v2-$v1)*0.5]
	}
    }
    if { [llength $ay(refineknv)] == 1 } {
	lappend ay(refineknv) [lindex $ay(refineknv) 0]
    }
 return;
}
# npatch_getrknotsv


# split patch at parametric value
proc npatch_split { cmd t r a {scmd ""}} {
    global ay ay_error
    getSel sel
    if { $r } {
	append cmd " -r"
    }
    if { $a } {
	append cmd " -a"
    }
    append cmd " $t"
    set ay_error 0
    eval $cmd
    if { $ay_error == 0 } {
	cS
	if { $a } {
	    uCR; sL [llength sel]; getSel lsel
	    lappend sel $lsel
	} else {
	    set ay(ul) $ay(CurrentLevel); uS
	    if { $scmd != "" } {
		set nsel [$scmd $sel]
	    } else {
		set inc 0
		foreach s $sel {
		    lappend nsel [expr $s + $inc] [expr $s + $inc + 1]
		    incr inc
		}
	    }
	    set sel $nsel
	}
	eval "selOb $sel"
	if { $ay(lb) == 0 } {
	    tree_sync $ay(tree)
	} else {
	    olb_sync $ay(lb)
	}
    } else {
	uS 0 1
    }
    rV
    plb_update
 return;
}
# npatch_split

proc npatch_tweensel { sel } {
    return [expr [lindex $sel 0] + 1 ]
}