# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# action.tcl - interactive actions

#actionEnd:
# bound to variable trace for ay(action) that in turn
# designates whether modeling actions are going on;
# if ay(action) is written and 0 (an action ended),
# we enforce an additional notification, so that
# objects may adapt their notification (do low quality
# but fast work, when a modelling action is going on,
# do high quality but slow work, when the modelling
# action finished)
proc actionEnd { n1 n2 op } {
    global ay

    if { $ay(action) == 0 } {
	# only initiate notification for modified objects
	# this also avoids unneeded lengthy notification runs in
	# view actions (view-rotate, view-move etc.)
	notifyOb -mod
	$ay(currentView) setconf -a 0
    } else {
	$ay(currentView) setconf -a 1
    }

 return;
}
# actionEnd

trace variable ay(action) w actionEnd


#actionBindRelease:
# establish the standard release binding for modelling actions:
# normalize points or transformation attributes;
# force notification (via ay(action)/actionEnd above);
# redraw all views; update property GUI
proc actionBindRelease { w {normalize 1} } {
    if { $normalize } {
	bind $w <ButtonRelease-1> {
	    set sel ""
	    getSel sel
	    if { $sel != "" } {
		if { $ay(cVPnts) } {
		    if { $ayprefs(NormalizePoints) } {
			normPnts
		    }
		} else {
		    if { $ayprefs(NormalizeTrafos) } {
			normTrafos;getTrafo
		    }
		}
            }
        }
    }
    bind $w <ButtonRelease-1> {+
	set ay(action) 0
	update
	rV
	plb_update
	focus %W
    }
 return;
}
# actionBindRelease


#actionBindCenter:
# establish standard bindings for some modelling actions
# that switch to about cog immediately, without going
# the detour via actionSetMark below
proc actionBindCenter { w { nextaction "" } } {
    global ayviewshortcuts

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    # set mark from selected objects center of gravity
    bind $t $ayviewshortcuts(CenterO) {
	if { [string first ".view" %W] == 0 } {
	    set w [winfo toplevel %W]
	} else {
	    set w %W
	}
	$w.f3D.togl mc
	$w.f3D.togl setconf -cmark 0
    }

    # set mark from selected points center of gravity
    bind $t $ayviewshortcuts(CenterPC) {
	if { [string first ".view" %W] == 0 } {
	    set w [winfo toplevel %W]
	} else {
	    set w %W
	}
	$w.f3D.togl mc
	$w.f3D.togl setconf -cmark 2
    }

    # set mark from selected points bounding box center
    bind $t $ayviewshortcuts(CenterPB) {
	if { [string first ".view" %W] == 0 } {
	    set w [winfo toplevel %W]
	} else {
	    set w %W
	}
	$w.f3D.togl mc
	$w.f3D.togl setconf -cmark 1
    }

    if { $nextaction != "" } {
	bind $t $ayviewshortcuts(CenterO) "+ $nextaction \$w.f3D.togl;"
	bind $t $ayviewshortcuts(CenterPC) "+ $nextaction \$w.f3D.togl;"
	bind $t $ayviewshortcuts(CenterPB) "+ $nextaction \$w.f3D.togl;"
    }

 return;
}
# actionBindCenter


#actionSetMark:
# helper for all actions about a user specified point (the mark)
# e.g. rotate about and scale about
# but also a full fledged action to set the mark
proc actionSetMark { w { nextaction "" } } {
    global ayprefs

    viewTitle $w "" "Mark Point"
    viewSetMAIcon $w ay_Mark_img "Mark Point"

    if { $nextaction == "" } {
	actionClearB1 $w
    }

    # set mark from mouse click
    bind $w <ButtonPress-1> {
	%W mc
	%W setconf -mark %x %y
	set oldx %x
	set oldy %y
    }
    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0
	if { %x != $oldx || %y != $oldy } {
	    %W setconf -mark %x %y $oldx $oldy
	}
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    # until we can set the mark via kbd
    actionClearParamKbd $t

    # if nextaction is not empty, we are an intermediate
    # action, embedded into some other action, which we arrange
    # to re-start here (after setting the mark):
    if { $nextaction != "" } {
	bind $w <ButtonRelease-1> "+ $nextaction %W;"
	# take over old mark
	set ob [bind $t <Key-Return>]
	bind $t <Key-Return> "bind $t <Key-Return> \{$ob\};\
                              $nextaction $t.f3D.togl;"
    } else {
	bind $t <Key-Return> ""
    }
    # if nextaction

    actionBindCenter $w $nextaction

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -readonly -flash
	}

	bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"
	bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"

	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
              %W startpepac %x %y -readonly -flash -ignoreold"
	    bind $w <Shift-ButtonRelease-1> "+\
              %W startpepac %x %y -readonly -flash -ignoreold"
	}
    }
    # if flash

    bind $w <ButtonRelease-1> "+focus %W"

    $w setconf -drawh 1

 return;
}
# actionSetMark


#
proc actionRotView { w } {

    viewTitle $w "" "Rotate View"
    viewSetMAIcon $w ay_RotV_img "Rotate View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	%W mc
	if { $ay(cVUndo) } {
	    undo save RotView
	}
	set oldx %x
	set oldy %y
    }

    bind $w <B1-Motion> {
	%W setconf -drotx [expr {$oldx - %x}] -droty [expr {$oldy - %y}]
	set oldx %x
	set oldy %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

    $w setconf -drawh 0

 return;
}
# actionRotView


#
proc actionMoveView { w } {

    viewTitle $w "" "Move View"
    viewSetMAIcon $w ay_MoveV_img "Move View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	%W mc
	if { $ay(cVUndo) } {
	    undo save MovView
	}
	%W movevac -start %x %y
    }

    bind $w <B1-Motion> {
	%W movevac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

    $w setconf -drawh 0

 return;
}
# actionMoveView


#
proc actionZoomView { w } {

    viewTitle $w "" "Zoom View"
    viewSetMAIcon $w ay_ZoomV_img "Zoom View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	%W mc
	if { $ay(cVUndo) } {
	    undo save ZoomView
	}
	%W zoomvac -start %y
    }

    bind $w <B1-Motion> {
	%W zoomvac -winy %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

    $w setconf -drawh 0
}
# actionZoomView


#
proc actionMoveZView { w } {

    viewTitle $w "" "MoveZ View"
    viewSetMAIcon $w ay_MoveVZ_img "MoveZ View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	%W mc
	if { $ay(cVUndo) } {
	    undo save MovZView
	}
	%W movezvac -start %y
    }

    bind $w <B1-Motion> {
	%W movezvac -winy %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

    $w setconf -drawh 0

 return;
}
# actionMoveZView


#
proc actionMoveOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Move"
    viewSetMAIcon $w ay_Move_img "Move"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save MoveObj
	%W mc
	if { $ay(restrict) > 0 } {
	    %W moveoac -start %x %y $ay(restrict)
	} else {
	    %W moveoac -start %x %y
	}
    }

    bind $w <B1-Motion> {
	%W moveoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    # always start unrestricted
    set ay(restrict) 0

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    # allow restriction: x only
    bind $t $ayviewshortcuts(RestrictX) "\
	set ay(restrict) 1;\
	viewSetMAIcon $t.f3D.togl ay_MoveX_img \"MoveX\";\
 actionBindParamKbd $t ay(dx) \" DX: \" \"undo save MoveXObj; $w moveoac -dx \""

    # allow restriction: y only
    bind $t $ayviewshortcuts(RestrictY) "\
	set ay(restrict) 2;\
	viewSetMAIcon $t.f3D.togl ay_MoveY_img \"MoveY\";\
 actionBindParamKbd $t ay(dy) \" DY: \" \"undo save MoveYObj; $w moveoac -dy \""

    # allow restriction: z only
    bind $t $ayviewshortcuts(RestrictZ) "\
	set ay(restrict) 3;\
	viewSetMAIcon $t.f3D.togl ay_MoveZ_img \"MoveZ\";\
 actionBindParamKbd $t ay(dz) \" DZ: \" \"undo save MoveZObj; $w moveoac -dz \""

 return;
}
# actionMoveOb

# actionGetParamFromKbd:
# request a parameter from the user via a temporary entry widget that
# appears in the lower right corner of the view when one of the ParamKeys
# is pressed. Number keys are immediately re-routed to the entry.
proc actionGetParamFromKbd { w var txt cmd key } {
    global ay ayprefs
    if { [string first ".view" $w] == 0 } {
	catch {destroy $w.param}
	set f [frame $w.param]
    } else {
	catch {destroy .param}
	set f [frame .param]
    }
    pack [label $f.l -text $txt] -side left
    pack [entry $f.e -textvariable $var] -side left
    bind $f.e <Key-Escape> "destroy $f;focus $w;break;"
    if { $ayprefs(KeepParamGUI) } {
	bind $f.e <Key-Return> "$cmd \$$var;break;"
	bind $f.e <Shift-Key-Return> "$cmd \$$var;destroy $f;focus $w;break;"
    } else {
	bind $f.e <Key-Return> "$cmd \$$var;destroy $f;focus $w;break;"
	bind $f.e <Shift-Key-Return> "$cmd \$$var;break;"
    }
    bind $f.e <Key-KP_Enter> [bind $f.e <Key-Return>]
    bind $f.e <Shift-Key-KP_Enter> [bind $f.e <Shift-Key-Return>]
    place $f -in $w -anchor se -relx 1.0 -rely 1.0 -x -1 -y -1
    if { [string length $key] == 1 } {
	$f.e delete 0 end
	$f.e insert 0 $key
    } else {
	$f.e select range 0 end
    }
    bindtags $f.e [list $f.e Entry]
    focus $f.e
 return;
}
# actionGetParamFromKbd

proc actionBindParamKbd { w var txt cmd } {
    global ayviewshortcuts
    foreach key $ayviewshortcuts(ParamKeys) {
	bind $w <Key-$key> "actionGetParamFromKbd $w $var {$txt} {$cmd} %K"
    }
 return;
}

proc actionClearParamKbd { w } {
    global ayviewshortcuts
    foreach key $ayviewshortcuts(ParamKeys) {
	bind $w <Key-$key> ""
    }
 return;
}

#
proc actionRotOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Rotate"
    viewSetMAIcon $w ay_Rotate_img "Rotate"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save RotObj
	%W mc
	%W rotoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W rotoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionRotObA $w;\
	} else {\
	    actionSetMark $w actionRotObA;\
	}"

    actionBindCenter $w actionRotObA

    actionBindParamKbd $t ay(angle) " Angle: "\
	"undo save RotObj; $w rotoac -angle "

 return;
}
# actionRotOb


#
proc actionRotObA { w } {
    global ayviewshortcuts

    viewTitle $w "" "RotateA"
    viewSetMAIcon $w ay_RotateA_img "RotateA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save RotObjA
	%W mc
	%W rotoaac -start %x %y
    }
    bind $w <B1-Motion> {
	%W rotoaac -winxy %x %y
    }

    actionBindRelease $w

    bind $w <Motion> ""

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionRotObA"

    actionBindParamKbd $t ay(angle) " Angle: "\
	"undo save RotObjA; $w rotoaac -angle "

 return;
}
# actionRotObA


#
proc actionSc1DXOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale1DX"
    viewSetMAIcon $w ay_Scale1DX_img "Scale1DX"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DXObj
	%W mc
	%W sc1dxoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dxoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc1DXAOb $w;\
	} else {\
	    actionSetMark $w actionSc1DXAOb;\
	}"

    actionBindCenter $w actionSc1DXAOb

    actionBindParamKbd $t ay(angle) " Scale: "\
	"undo save Sc1DXObj; $w sc1dxoac -scale "

 return;
}
# actionSc1DXOb


#
proc actionSc1DXAOb { w } {
    global ayviewshortcuts

    viewTitle $w "" "Scale1DXA"
    viewSetMAIcon $w ay_Scale1DXA_img "Scale1DXA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DXAObj
	%W mc
	%W sc1dxaoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dxaoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc1DXAOb"

    actionBindParamKbd $t ay(angle) " Scale: "\
	"undo save Sc1DXAObj; $w sc1dxaoac -scale "

 return;
}
# actionSc1DXAOb


#
proc actionSc1DYOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale1DY"
    viewSetMAIcon $w ay_Scale1DY_img "Scale1DY"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DYObj
	%W mc
	%W sc1dyoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dyoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc1DYAOb $w;\
	} else {\
	    actionSetMark $w actionSc1DYAOb;\
	}"

    actionBindCenter $w actionSc1DYAOb

    actionBindParamKbd $t ay(angle) " Scale: "\
	"undo save Sc1DYObj; $w sc1dyoac -scale "

 return;
}
# actionSc1DYOb


#
proc actionSc1DYAOb { w } {
    global ayviewshortcuts

    viewTitle $w "" "Scale1DYA"
    viewSetMAIcon $w ay_Scale1DYA_img "Scale1DYA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DYAObj
	%W mc
	%W sc1dyaoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dyaoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc1DYAOb"

    actionBindParamKbd $t ay(angle) " Scale: "\
	"undo save Sc1DYAObj; $w sc1dyaoac -scale "

 return;
}
# actionSc1DYAOb


#
proc actionSc1DZOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale1DZ"
    viewSetMAIcon $w ay_Scale1DZ_img "Scale1DZ"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DZObj
	%W mc
	%W sc1dzoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dzoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc1DZAOb $w;\
	} else {\
	    actionSetMark $w actionSc1DZAOb;\
	}"

    actionBindCenter $w actionSc1DZAOb

    actionBindParamKbd $t ay(angle) " Scale: "\
	"undo save Sc1DZObj; $w sc1dzoac -scale "

 return;
}
# actionSc1DZOb


#
proc actionSc1DZAOb { w } {
    global ayviewshortcuts

    viewTitle $w "" "Scale1DZA"
    viewSetMAIcon $w ay_Scale1DZA_img "Scale1DZA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DZAObj
	%W mc
	%W sc1dzaoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dzaoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc1DZAOb"

    actionBindParamKbd $t ay(angle) " Scale: "\
	"undo save Sc1DZAObj; $w sc1dzaoac -scale "

 return;
}
# actionSc1DZAOb


#
proc actionSc2DOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale2D"
    viewSetMAIcon $w ay_Scale2D_img "Scale2D"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc2DObj
	%W mc
	%W sc2doac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc2doac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc2DAOb $w;\
	} else {\
	    actionSetMark $w actionSc2DAOb;\
	}"

    # allow restriction: x only
    bind $t $ayviewshortcuts(RestrictX) "actionSc1DXOb $w"

    # allow restriction: y only
    bind $t $ayviewshortcuts(RestrictY) "actionSc1DYOb $w"

    # allow restriction: z only
    bind $t $ayviewshortcuts(RestrictZ) "actionSc1DZOb $w"

    actionBindCenter $w actionSc2DAOb

    actionBindParamKbd $t ay(scale) " Scale: "\
	"undo save Sc2DObj; $w sc2doac -scale "

 return;
}
# actionSc2DOb


#
proc actionSc2DAOb { w } {
    global ayviewshortcuts

    viewTitle $w "" "Scale2D"
    viewSetMAIcon $w ay_Scale2DA_img "Scale2DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc2DAObj
	%W mc
	%W sc2daoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc2daoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc2DAOb"

    actionBindParamKbd $t ay(scale) " Scale: "\
	"undo save Sc2DAObj; $w sc2daoac -scale "

 return;
}
# actionSc2DAOb


#
proc actionSc3DOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale3D"
    viewSetMAIcon $w ay_Scale3D_img "Scale3D"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc3DObj
	%W mc
	%W sc3doac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc3doac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc3DAOb $w;\
	} else {\
	    actionSetMark $w actionSc3DAOb;\
	}"

    actionBindCenter $w actionSc3DAOb

    actionBindParamKbd $t ay(scale) " Scale: "\
	"undo save Sc3DObj; $w sc3doac -scale "

 return;
}
# actionSc3DOb


#
proc actionSc3DAOb { w } {
    global ayviewshortcuts

    viewTitle $w "" "Scale3DA"
    viewSetMAIcon $w ay_Scale3DA_img "Scale3DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc3DAObj
	%W mc
	%W sc3daoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc3daoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc3DAOb"

    actionBindParamKbd $t ay(scale) " Scale: "\
	"undo save Sc3DAObj; $w sc3daoac -scale "

 return;
}
# actionSc3DAOb


#
proc actionStr2DOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Stretch2D"
    viewSetMAIcon $w ay_Stretch2D_img "Stretch2D"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Str2DObj
	%W mc
	%W str2doac -start %x %y
    }

    bind $w <B1-Motion> {
	%W str2doac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionStr2DAOb $w;\
	} else {\
	    actionSetMark $w actionStr2DAOb;\
	}"

    actionBindCenter $w actionStr2DAOb

 return;
}
# actionStr2DOb


#
proc actionStr2DAOb { w } {
    global ayviewshortcuts

    viewTitle $w "" "Stretch2DA"
    viewSetMAIcon $w ay_Stretch2DA_img "Stretch2DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Str2DAObj
	%W mc
	%W str2daoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W str2daoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionStr2DAOb"

 return;
}
# actionStr2DAOb


#
proc actionTagP { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Select Points"
    viewSetMAIcon $w ay_Tag_img "Select Points"

    if { $ayprefs(TagResetTagged) == 1 } {
	selPnts; rV
    }

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	# undo save TagP
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W selpac %x %y $oldx $oldy
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	plb_update
	focus %W
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -readonly -flash -ignoreold
	    %W startpepac %x %y -readonly -flash -ignoreold
	}
    }

    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W selpac %x %y $oldx $oldy 1
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	plb_update
	focus %W
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -readonly -flash -ignoreold
	    %W startpepac %x %y -readonly -flash -ignoreold
	}
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -readonly -flash
	}

	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
              %W startpepac %x %y -readonly -flash -ignoreold"
	    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> "+\
              %W startpepac %x %y -readonly -flash -ignoreold"
	}
    }
    # if flash

    $w setconf -drawh 1

 return;
}
# actionTagP


#
proc actionDelTagP { w } {
    # undo save DelTagP
    $w deselpac
    rV

 return;
}
# actionDelTagP


#
proc actionTagB { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Select Bounds"
    viewSetMAIcon $w ay_TagB_img "Select Bounds"

    if { $ayprefs(TagResetTagged) == 1 } {
	selPnts; rV
    }

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	# undo save TagP
	%W mc
    }

    bind $w <ButtonRelease-1> {
	%W selbac %x %y
	rV
	update
	focus %W
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -readonly -flash
	}
	bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"

	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
              %W startpepac %x %y -readonly -flash -ignoreold"
	}
    }
    $w setconf -drawh 1

 return;
}
# actionTagB


# in this global array the numeric edit points
# action keeps its data
array set editPntArr {
    x 0.0
    y 0.0
    z 0.0
    w 0.0

    lx 0.0
    ly 0.0
    lz 0.0
    lw 0.0

    wx 0.0
    wy 0.0
    wz 0.0
    ww 0.0

    window ""
    valid 0
    local 1
}
# array

proc editPointDialogClear { ww } {
    foreach e {x y z w} {$ww.f1.f$e.e delete 0 end}
}

proc editPointDialogSet { ww arr } {
    if { $arr == "first" || $arr == "last" } {
	selPnts -get pnts
	if { [llength $pnts] > 0 } {
	    set pnts [lsort $pnts]
	    if { $arr == "first" } {
		getPnt [lindex $pnts 0] x y z w
	    } else {
		set l [llength $pnts]
		incr l -1
		getPnt [lindex $pnts $l] x y z w
	    }
	    editPointDialogClear $ww
	    foreach e {x y z w} {
		if { [info exists $e] } {
		    eval $ww.f1.f$e.e insert 0 \$$e
		}
	    }
	}
    } else {
	global $arr
	editPointDialogClear $ww
	foreach e {x y z w} {
	    if { [info exists ${arr}($e)] } {
		eval $ww.f1.f$e.e insert 0 \$${arr}($e)
	    }
	}
    }
 return;
}

proc editPointDialogUpdatePopup { ww } {
    global aymark
    set m $ww.popup
    set l "Fetch Mark"
    append l " ($aymark(x), $aymark(y), $aymark(z))"
    $m entryconfigure 2 -label $l
    set is 0
    if { [getSel] != "" } {
	foreach s [isSurface] {
	    if { $s } {
		set is 1
		break;
	    }
	}
    }
    selPnts -get pnts
    if { !$is && ([llength $pnts] > 0) } {
	set pnts [lsort $pnts]
	set len [llength $pnts]
	incr len -1
	foreach i {3 4} {
	    if { $i == 3 } {
		set l "Fetch First ("
		getPnt [lindex $pnts 0] x y z w
	    } else {
		set l "Fetch Last ("
		getPnt [lindex $pnts $len] x y z w
	    }
	    foreach e {x y z w} {
		if { [info exists $e] } {
		    eval append l \$$e
		    if { $e != "w" } { append l ", " }
		}
	    }
	    append l ")"
	    $m entryconfigure $i -state active -label $l
	}
    } else {
	$m entryconfigure 3 -state disabled -label "Fetch First"
	$m entryconfigure 4 -state disabled -label "Fetch Last"
    }
 return;
}

#editPointDialogUpdate:
# helper for editPointDialog
# updates the dialog entries
proc editPointDialogUpdate { w } {
    upvar #0 editPntArr array

    if { $array(local) == 1 } {
	set array(x) $array(lx)
	set array(y) $array(ly)
	set array(z) $array(lz)
	set array(w) $array(lw)
    } else {
	set array(x) $array(wx)
	set array(y) $array(wy)
	set array(z) $array(wz)
	set array(w) $array(ww)
    }

    update
    editPointDialogSet $w editPntArr
    update

 return;
}
# editPointDialogUpdate


#editPointDialogApply:
# helper for editPointDialog
# apply the changes from the dialog
proc editPointDialogApply { w } {
    global ay
    upvar #0 editPntArr array

    set array(x) [$w.f1.fx.e get]
    set array(y) [$w.f1.fy.e get]
    set array(z) [$w.f1.fz.e get]
    set array(w) [$w.f1.fw.e get]

    # if there are scripts or variable accesses in the
    # entry fields, execute / realize them ...
    if { [string match {*\[*} $array(x)] } {
	eval set array(x) $array(x)
    } else {
	if { [string match {*\$*} $array(x)] } {
	    eval [subst "set array(x) $array(x)"]
	}
    }

    if { [string match {*\[*} $array(y)] } {
	eval set array(y) $array(y)
    } else {
	if { [string match {*\$*} $array(y)] } {
	    eval [subst "set array(y) $array(y)"]
	}
    }

    if { [string match {*\[*} $array(z)] } {
	eval set array(z) $array(z)
    } else {
	if { [string match {*\$*} $array(z)] } {
	    eval [subst "set array(z) $array(z)"]
	}
    }

    if { [string match {*\[*} $array(w)] } {
	eval set array(w) $array(w)
    } else {
	if { [string match {*\$*} $array(w)] } {
	    eval [subst "set array(w) $array(w)"]
	}
    }

    # in floating window GUI mode our window
    # may be closed now, better check that...
    if { [winfo exists $array(window)] } {
	undo save EditPntNum
	set focusWin [focus]
	$array(window) penpac -apply
	rV
	plb_update
	after idle "catch \{focus $focusWin\}"
    } else {
	ayError 2 "editPointApply" "Lost window to apply changes to!"
    }

 return;
}
# editPointDialogApply


#editPointDialogBind:
# helper for editPointDialog
# establish misc keyboard bindings
proc editPointDialogBind { w } {
    global tcl_platform

    set ww .editPointDw

    bind $w <Key-Return> "editPointDialogApply $ww; break"
    bind $w <Shift-Key-Return>\
	"editPointDialogApply $ww; $ww.f2.bca invoke; break"
    catch {bind $w <Key-KP_Enter> "editPointDialogApply $ww; break"}
    catch {bind $w <Shift-Key-KP_Enter>\
	       "editPointDialogApply $ww; $ww.f2.bca invoke; break"}

    bind $w <Shift-Delete> "editPointDialogClear $ww"
    if { $tcl_platform(platform) == "windows" && $w != $ww } {
	bind $w <Tab> "focus \[tk_focusNext $w\];break"
	bind $w <Shift-Tab> "focus \[tk_focusPrev $w\];break"
    }

 return;
}
# editPointDialogBind


#editPointDialog:
# helper for actionEditNumP below
# open the numeric point edit dialog
proc editPointDialog { win {dragsel 0} } {
    #upvar #0 editPntArr array
    global editPntArr
    global ay aymark ayprefs aymainshortcuts tcl_platform AYWITHAQUA

    set w .editPointDw

    if { $dragsel == 1 } {
	if { [winfo exists $w] } {
	    return;
	}
	selPnts -get pnts
	if { [llength $pnts] == 0 } {
	    return;
	}
	$win setconf -cmark 1
	set editPntArr(x) $aymark(x)
	set editPntArr(y) $aymark(y)
	set editPntArr(z) $aymark(z)
	set editPntArr(w) 1

	if { [llength $pnts] == 1 } {
	    getPnt -sel pnt
	    if { [llength $pnt] == 4 } {
		set editPntArr(w) [lindex $pnt 3]
	    }
	}

	set editPntArr(valid) 1
    }

    if { [winfo exists $w] } {
	editPointDialogUpdate $w
	return;
    }

    catch {destroy $w}
    set t "Edit Point"
    winDialog $w $t

    if { $ayprefs(FixDialogTitles) == 1 } {
	pack [frame $w.fl] -in $w -side top
	pack [label $w.fl.l -text $t] -in $w.fl -side left -fill x -expand yes
    }

    set m $ay(editmenu)
    bind $w <[repctrl $aymainshortcuts(Undo)]> "$m invoke 12;break"
    bind $w <[repctrl $aymainshortcuts(Redo)]> "$m invoke 13;break"

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set f [frame $f.fm]
    menubutton $f.mb -text "Local" -menu $f.mb.m -relief raised\
	    -padx 0 -pady 1 -indicatoron 1
    if { $tcl_platform(platform) == "windows" } {
	$f.mb configure -pady 1
    }
    if { $AYWITHAQUA } {
	$f.mb configure -pady 2
    }
    set m [menu $f.mb.m -tearoff 0]
    $m add command -label "Local" -command {
	global editPntArr
	set editPntArr(local) 1
	.editPointDw.f1.fm.mb configure -text "Local"
	editPointDialogUpdate .editPointDw
    }
    $m add command -label "World" -command {
	global editPntArr
	set editPntArr(local) 0
	.editPointDw.f1.fm.mb configure -text "World"
	editPointDialogUpdate .editPointDw
    }
    pack $f.mb -in $f -side left -fill x -expand yes -pady 0
    pack $f -in $w.f1 -side top -fill x

    # separating space
    pack [label $w.f1.l0 -height 1 -image ay_Empty_img -pady 0]\
	-in $w.f1 -side top -fill x -pady 0

    # create four entries
    foreach e {X Y Z W} {
	set lce [string tolower $e]
	set f $w.f1
	set f [frame $f.f$lce]
	label $f.l -text $e -width 4
	entry $f.e -width 8
	pack $f.l -in $f -padx 2 -pady 2 -side left -fill x -expand no
	pack $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
	pack $f -in $w.f1 -side top -fill x

	editPointDialogBind $f.e
    }

    # separating space
    pack [label $w.f1.l2 -height 2 -image ay_Empty_img -pady 0]\
	-in $w.f1 -side top -fill x -pady 0

    update
    if { $dragsel == 0 } {
	editPointDialogUpdate $w
    } else {
	editPointDialogSet $w editPntArr
    }

    set f [frame $w.f2]
    button $f.bok -text "Apply" -width 5 -pady $ay(pady)\
	-command "editPointDialogApply $w"
    button $f.bca -text "Cancel" -width 5 -pady $ay(pady) -command "\
            global editPntArr;\
	    if { \[winfo exists \$editPntArr(window)\] } {\
	     focus \$editPntArr(window) } else { resetFocus };\
	    destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # create popup
    set m [menu $w.popup -tearoff 0]
    $m add command -label "Clear <Shift-Delete>"\
	-command "editPointDialogClear $w"
    $m add command -label "Reset" -command "editPointDialogUpdate $w"
    $m add command -label "Fetch Mark" -command "editPointDialogSet $w aymark"
    $m add command -label "Fetch First" -command "editPointDialogSet $w first"
    $m add command -label "Fetch Last" -command "editPointDialogSet $w last"

    # auto raise window, when obscured
    bind $w <Visibility> {
	# are we obscured?
	if { "%s" == "VisibilityPartiallyObscured" ||\
		"%s" == "VisibilityFullyObscured" } {
	    # yes: try to raise the window, but just one time
	    raise [winfo toplevel %W]
	    bind %W <Visibility> ""
	}
    }

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-3.html numeditac

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$w.f2.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$w.f2.bca invoke"

    # establish misc keyboard bindings
    editPointDialogBind $w

    foreach e {x y z w}	{
	bind $w <$e> "\
          if \{ \"\[winfo parent %W\]\" != \"$w.f1\" \} \{\
            focus $w.f1.f$e.e;\
	  \}"
    }

    focus $f.bok

    bind $w <ButtonPress-$aymainshortcuts(CMButton)> "\
          editPointDialogUpdatePopup $w;\
          winOpenPopup $w"

    if { [string first ".view" $win] != 0 } {
	# internal view
	winRestoreOrCenter $w $t
    } else {
	# external view => center dialog on view
	wm transient $w [winfo toplevel $win]
	winCenter $w [winfo toplevel $win]
    }

 return;
}
# editPointDialog


#
proc actionEditNumP { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Numeric Edit"
    viewSetMAIcon $w ay_EditN_img "Numeric Edit"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	set oldx %x
	set oldy %y
	set editPntArr(valid) 0
	%W penpac -start %x %y
	set editPntArr(window) %W
	if { $editPntArr(valid) == 1 } {
	    editPointDialog %W
	}
	update
    }

    # next bindings taken from tag action (actionTagP)
    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W selpac %x %y $oldx $oldy
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	focus %W
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash -ignoreold
	    %W startpepac %x %y -flash -ignoreold
	}
	editPointDialog %W 1;
    }

    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W selpac %x %y $oldx $oldy 1
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	focus %W
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash -ignoreold
	    %W startpepac %x %y -flash -ignoreold
	}
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
              %W startpepac %x %y -flash -ignoreold;"
	    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> "+\
              %W startpepac %x %y -flash -ignoreold;"
	}
    }

    $w setconf -drawh 1

    if { [selPnts -have] } {
	global editPntArr
	set editPntArr(window) $w
        $w penpac -start 0 0
	editPointDialog $w 1
    }

 return;
}
# actionEditNumP


#actionEditP:
# move/edit/drag picked points
proc actionEditP { w } {
    global ayprefs

    viewTitle $w "" "Edit Points"
    viewSetMAIcon $w ay_Edit_img "Edit Points"

    actionClearB1 $w

    bind $w <Motion> ""

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save EditPnt
	%W mc
	if { $ayprefs(FlashPoints) == 1 } {
	    %W pepac -start %x %y
	} else {
	    %W startpepac %x %y
	    %W pepac -start %x %y
	}
    }

    bind $w <B1-Motion> {
	%W pepac -winxy %x %y
    }

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;"

	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
              %W startpepac %x %y -flash -ignoreold"
	}
    }

    $w setconf -drawh 1

 return;
}
# actionEditP


#actionEditWP
# change the weigth of the picked points
proc actionEditWP { w } {
    global ayprefs ay

    viewTitle $w "" "Edit Weights"
    viewSetMAIcon $w ay_EditW_img "Edit Weights"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save EditWeight
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash
	} else {
	    %W startpepac %x %y
	}
	%W wepac -start %x
    }

    bind $w <Control-ButtonPress-1> {
	set ay(action) 1
	undo save EditWeight
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash
	} else {
	    %W startpepac %x %y
	}
	%W wepac -start %x 0
    }

    bind $w <B1-Motion> {
	%W wepac -winx %x
	%W render
    }

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;"

	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
              %W startpepac %x %y -flash -ignoreold"
	}
    }

    $w setconf -drawh 3

 return;
}
# actionEditWP


#actionResetWP:
# reset the weights of the picked points to 1.0
proc actionResetWP { w } {
    global ayprefs

    viewTitle $w "" "Reset Weights"
    viewSetMAIcon $w ay_ResetW_img "Reset Weights"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	undo save ResetWeights
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash
	} else {
	    %W startpepac %x %y
	}
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W wrpac -rect %x %y $oldx $oldy
	} else {
	    %W wrpac -selected
	}
	rV
	plb_update
	focus %W
    }

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;"
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold"
	}
    }

    $w setconf -drawh 3

 return;
}
# actionResetWP


#actionResetAllWP:
# reset the weights of all points to 1.0
proc actionResetAllWP { w } {
    undo save ResetAllWeights
    $w wrpac
    plb_update
    rV
 return;
}
# actionResetAllWP


#actionInsertAndEdit:
# insert points into curves and immediately edit them
proc actionInsertAndEdit { w x y } {
    global ayprefs
    undo save InsPnt
    $w mc
    $w insertpac $x $y -edit
    selPnts -get index
    if { [llength $index] > 0 } {
	getProp
	$w redraw
	$w setconf -pnts 1
	$w moveoac -start $x $y
	bind $w <B1-Motion> "$w moveoac -winxy %x %y; setProp -1"
	bind $w <ButtonRelease-1>\
	    "+bind %W <B1-Motion> \"\"; selPnts; %W redraw;"

	if { $ayprefs(FlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
              %W startpepac %x %y -flash -ignoreold;"
	    if { $ayprefs(FixFlashPoints) == 1 } {
		bind $w <ButtonRelease-1> "+\
                  %W startpepac %x %y -flash -ignoreold;"
	    }
	}

	# XXXX bind <Control-Release> to selPnts to allow continuous editing?
    }
 return;
}
# actionInsertAndEdit


#actionInsertP:
# insert points into curves
proc actionInsertP { w } {
    global ayprefs

    viewTitle $w "" "Insert Points"
    viewSetMAIcon $w ay_Insert_img "Insert Points"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	undo save InsPnt
	%W mc
	%W insertpac %x %y
    }

    bind $w <Control-ButtonPress-1> {
	actionInsertAndEdit %W %x %y
    }

    bind $w <B1-Motion> ""

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;"
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
                  %W startpepac %x %y -flash -ignoreold;"
	}
    }

    $w setconf -drawh 1

 return;
}
# actionInsertP


#actionDeleteP:
# delete points from curves
proc actionDeleteP { w } {
    global ayprefs

    viewTitle $w "" "Delete Points"
    viewSetMAIcon $w ay_Delete_img "Delete Points"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	undo save DelPnt
	%W mc
	%W deletepac %x %y
    }

    bind $w <B1-Motion> ""

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <ButtonRelease-1> "+%W startpepac %x %y -flash -ignoreold"
    }

    $w setconf -drawh 1

 return;
}
# actionDeleteP


#actionFindU:
# find parametric value u on a NURBS curve
proc actionFindU { w } {
    global ayprefs

    viewTitle $w "" "Find U"
    viewSetMAIcon $w ay_FindU_img "Find U"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -readonly -flash
	}
    }

    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W finduac %x %y $oldx $oldy
	} else {
	    %W finduac %x %y
	}
	%W finduac -end %x %y
	%W redraw
	if { ($oldx != %x) || ($oldy != %y) } {
	    plb_update
	}
	focus %W
    }

    bind $w <Control-ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W finduac %x %y $oldx $oldy -extend
	} else {
	    %W finduac %x %y -extend
	}
	%W finduac -end %x %y
	%W redraw
	if { ($oldx != %x) || ($oldy != %y) } {
	    plb_update
	}
	focus %W
    }

    $w setconf -drawh 2

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    actionBindParamKbd $t u " U: " "actionSetMarkFromU "

 return;
}
# actionFindU

proc actionSetMarkFromU { u } {
    withOb 0 {
	getPnt -eval $u x y z
	setMark $x $y $z
    }
 return;
}

#actionFindUV:
# find parametric values u and v on a NURBS surface
proc actionFindUV { w } {
    global ayprefs

    viewTitle $w "" "Find UV"
    viewSetMAIcon $w ay_FindUV_img "Find UV"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -readonly -flash
	}
    }

    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W finduvac %x %y $oldx $oldy
	} else {
	    %W finduvac %x %y
	}
	%W finduvac -end %x %y
	%W redraw
	focus %W
    }

    $w setconf -drawh 2

 return;
}
# actionFindUV

#actionSelBnd:
# 
proc actionSelBnd { w {c 0} } {
    global ayprefs ayviewshortcuts

    if { $c } {
	viewTitle $w "" "SelBndC"
	viewSetMAIcon $w ay_SelBndC_img "SelBndC"
    } else {
	viewTitle $w "" "SelBnd"
	viewSetMAIcon $w ay_SelBnd_img "SelBnd"
    }
    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $c } {
	bind $w <ButtonRelease-1> {
	    %W setconf -rect $oldx $oldy %x %y 0
	    if { ($oldx != %x) || ($oldy != %y) } {
		%W cselbndac %x %y $oldx $oldy
	    } else {
		%W cselbndac %x %y
	    }
	}

	bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> {
	    %W setconf -rect $oldx $oldy %x %y 0
	    if { ($oldx != %x) || ($oldy != %y) } {
		%W cselbndac %x %y $oldx $oldy 1
	    } else {
		%W cselbndac %x %y
	    }
	}
    } else {
	bind $w <ButtonRelease-1> {
	    %W setconf -rect $oldx $oldy %x %y 0
	    if { ($oldx != %x) || ($oldy != %y) } {
		%W selbndac %x %y $oldx $oldy
	    } else {
		%W selbndac %x %y
	    }
	}

	bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> {
	    %W setconf -rect $oldx $oldy %x %y 0
	    if { ($oldx != %x) || ($oldy != %y) } {
		%W selbndac %x %y $oldx $oldy 1
	    } else {
		%W selbndac %x %y
	    }
	}
    }

    bind $w <ButtonRelease-1> {+
	    #%W selbnd -end %x %y
	    %W redraw
	    if { ($oldx != %x) || ($oldy != %y) } {
		plb_update
	    }
	    focus %W
    }
    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> {+
	    #%W selbnd -end %x %y
	    %W redraw
	    if { ($oldx != %x) || ($oldy != %y) } {
		plb_update
	    }
	    focus %W
    }

    $w setconf -drawh 4

 return;
}
# actionSelBnd


#actionSplitNC:
# split NURBS curve at parametric value u
proc actionSplitNC { w } {
    global ayprefs

    viewTitle $w "" "Split Curve"
    viewSetMAIcon $w ay_Split_img "Split Curve"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay_error 0
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W finduac %x %y $oldx $oldy -silence
	} else {
	    %W finduac %x %y -silence
	}
	%W finduac -end %x %y -nomark
	update
	if { $ay_error == 0 } {
	    ncurve_split $u
	}
	focus %W
    }

    $w setconf -drawh 2

 return;
}
# actionSplitNC


#actionPick:
# establish object picking bindings
proc actionPick { w } {
    global ay ayprefs ayviewshortcuts

    viewTitle $w "" "Pick Objects"
    viewSetMAIcon $w ay_Pick_img "Pick Objects"

    $w setconf -drawh 0
    set ay(b1d) 0

    bind $w <ButtonPress-1> {
	set oldx %x
	set oldy %y
	set ay(b1d) 1
    }

    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0

	if { $ay(b1d) && [winfo exists .reconsider] == 0 } {
	    if { ($oldx == %x) || ($oldy == %y) } {
		%W processObjSel node %x %y
		singleObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		multipleObjSel $node
	    }
	    set ay(b1d) 0
	}
	focus %W
    }
    # bind

    bind $w <${ayviewshortcuts(PickMod)}-ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0

	if { $ay(b1d) && [winfo exists .reconsider] == 0 } {
	    if { ($oldx == %x) || ($oldy == %y) } {
		%W processObjSel node %x %y
		addObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		addMultipleObjSel $node
	    }
	    set ay(b1d) 0
	}
	focus %W
    }
    # bind

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    if { $ayprefs(FlashObjects) } {
	bind $w <Motion> {
	    if { [winfo exists .reconsider] == 0 } {
		%W processObjSel - %x %y
	    }
	}
	bind $w <Leave> {
	    %W processObjSel +
	}
    } else {
	bind $w <Motion> ""
    }

 return;
}
# actionPick


#actionSnapToGrid3D:
# snap selected points to the grid (all dimensions, regardless of view type)
proc actionSnapToGrid3D { w } {
    undo save SnapToGrid3D
    $w snapac
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionSnapToGrid3D


#actionSnapToGrid2D:
# snap selected points to the grid (only in the input plane defined by the
# current view type)
proc actionSnapToGrid2D { w } {
    undo save SnapToGrid2D
    $w snapac 1
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionSnapToGrid2D

#actionSnapToGrid3DO:
# snap selected objects to the grid (all dimensions, regardless of view type)
proc actionSnapToGrid3DO { w } {
    undo save SnapToGrid3DO
    $w snapac 2
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionSnapToGrid3DO


#actionSnapToGrid2DO:
# snap selected objects to the grid (only in the input plane defined by the
# current view type)
proc actionSnapToGrid2DO { w } {
    undo save SnapToGrid2DO
    $w snapac 3
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionSnapToGrid2DO


#actionSnapToMark:
# snap selected points to the mark
proc actionSnapToMark { w } {
    undo save SnapToMark
    $w snapmac 0
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionSnapToMark


#actionSnapToMarkO:
# snap selected objects to the mark
proc actionSnapToMarkO { w } {
    undo save SnapToMarkO
    $w snapmac 1
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionSnapToMarkO


#actionIncMultP:
# increase multiplicity of selected points
proc actionIncMultP { w } {
    undo save IncMultP
    $w multpac 0
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionIncMultP


#actionDecMultP:
# decrease multiplicity of selected points
proc actionDecMultP { w } {
    undo save DecMultP
    $w multpac 1
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionDecMultP


#actionClearB1:
# helper procedure to clear all bindings to mouse button 1;
# all modeling actions call this before setting their bindings
proc actionClearB1 { w } {
    global ayviewshortcuts

    bind $w <B1-Motion> ""
    bind $w <ButtonPress-1> ""
    bind $w <ButtonRelease-1> ""
    bind $w <Control-ButtonPress-1> ""
    bind $w <${ayviewshortcuts(PickMod)}-ButtonRelease-1> ""
    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> ""

 return;
}
# actionClearB1


#actionClearKbd:
# helper procedure to clear all left over keyboard bindings
proc actionClearKbd { w } {
    global ayviewshortcuts

    bind $w $ayviewshortcuts(CenterO) ""
    bind $w $ayviewshortcuts(CenterPC) ""
    bind $w $ayviewshortcuts(CenterPB) ""
    bind $w <Key-Return> ""

 return;
}
# actionClearKbd


#actionClear:
# not really an action, clears all bindings, establishes picking bindings
# when requested via preferences and is normally bound to the Esc-key
proc actionClear { w {only_clear 0} } {
    global ayprefs ayviewshortcuts

    actionClearB1 $w
    actionClearKbd $w

    if { [string first ".view" $w] != 0 } {
	set t [winfo parent [winfo parent $w]]
	bind $w <ButtonPress-1> "focus \$w"
    } else {
	set t [winfo toplevel $w]
	bind $w <ButtonPress-1> ""
    }

    actionClearParamKbd $t

    shortcut_view $t

    bind $w <Motion> ""

    if { $only_clear || ($ayprefs(DefaultAction) == 0) } {
	viewTitle $w "" "None"
	viewSetMAIcon $w ay_EmptyG_img ""
    } else {
	switch $ayprefs(DefaultAction) {
	    1 { actionPick $w }
	    2 {	actionEditP $w }
	}
    }

    # do not draw points in none/pick action
    if { $only_clear || ($ayprefs(DefaultAction) < 2) } {
	$w setconf -drawh 0
    }

    # the following after scripts arrange for a short period
    # 0.1 - 1s after the first press of the <Esc> key, that a second
    # press of the <Esc> key also resets the mark and the focus
    after 100 "bind $t <$ayviewshortcuts(Break)> \"_doubleEsc $t\""
    # after 1s, the old binding is in effect
    after 1000 "bind $t <$ayviewshortcuts(Break)> \"actionClear $t.f3D.togl\""

    # re-establish standard set mark binding
    bind $t <$ayviewshortcuts(About)> "actionSetMark $t.f3D.togl"

 return;
}
# actionClear

# _doubleEsc:
# internal helper, bound to <Esc-Esc>
# clears the mark and resets the focus
proc _doubleEsc { t } {
    $t.f3D.togl setconf -mark n
    #ayError 4 "<Esc-Esc>" "Mark cleared."
    puts "Mark cleared."
    resetFocus
 return;
}
# _doubleEsc
