#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# aytest.tcl - test Ayam

array set aytestprefs {
    KeepObjects 0
    KeepFiles 0
    ShowProgress 0
    Progress 0
    Breadth 0
    Status "Select Item"
    TestSet "n/a"
    TestItem "n/a"
    TestVariant "n/a"
    DebugCreate 0
    InitTypes 0
    RandomOrder 0
    AdvancedOptions 0
    TestProcs { testDefaultCallbacks testValidSolidVariations testValidNURBS testValidToolObjects testModellingTools testAllSolidVariations testCustomObjects testScriptObjects }
}

# aytest_handleLBS:
#  handle selection of tests in select GUI listbox
#  (fill and enable/disable second listbox accordingly)
proc aytest_handleLBS { w } {
    global aytestprefs
    set sel [$w curselection]
    set tw [winfo toplevel $w]
    if { [llength $sel] == 1 } {
	# allow selection and item management
	$tw.fu.fr.l1 conf -state normal
	# fill listbox with items from test
	$tw.fu.fr.l1 delete 0 end
	$tw.fu.fr.l1 insert end "All"
	set testproc [lindex $aytestprefs(TestProcs) $sel]
	eval set items \$::${testproc}Items
	foreach item $items {
	    $tw.fu.fr.l1 insert end $item
	}
	$tw.fu.fr.l1 selection set 0
	bind $tw.fu.fr.l1 <<ListboxSelect>> {
	    if {[llength [%W curselection]] > 1 } {
		%W selection clear 0
	    }
	}
    } else {
	# clear listbox
	$tw.fu.fr.l1 delete 0 end
	$tw.fu.fr.l1 insert end "All"
	$tw.fu.fr.l1 selection set 0
	# disallow further selection
	$tw.fu.fr.l1 conf -state disabled
	bind $tw.fu.fr.l1 <<ListboxSelect>> ""
    }
 return;
}
# aytest_handleLBS


# aytest_fillGUI:
# (re-)create the option GUI components
proc aytest_fillGUI { } {
    global aytestprefs
    set w .testGUI.fp

    foreach c [winfo children $w] {
	destroy $c
    }

    addCheck $w aytestprefs RandomOrder

    addOptionToggle $w aytestprefs AdvancedOptions "Advanced Options"\
	aytest_fillGUI

    if { $aytestprefs(AdvancedOptions) == 1 } {
	addCheck $w aytestprefs KeepObjects
	addCheck $w aytestprefs KeepFiles
    }

    addOptionToggle $w aytestprefs ShowProgress "Show Progress     "\
	aytest_fillGUI

    if { $aytestprefs(ShowProgress) == 1 } {
	addProgress $w aytestprefs Progress
    }

 return;
}
# aytest_fillGUI


# aytest_selectGUI:
#  create the aytest user interface
#
proc aytest_selectGUI { } {
    global ay aytestprefs

    winAutoFocusOff

    set w .testGUI
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Test Ayam"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }

    set f [frame $w.fu]
    pack $f -in $w -side top -fill both -expand yes

    set ay(bca) $w.fl.bclose
    set ay(bok) $w.fl.bstart

    # test management widgets
    set f [frame $w.fu.fl -relief sunken -border 1]

    # listbox for test selection
    addText $f e0 "Select Test:"

    set lb [listbox $f.l1 -exportselection 0\
		-selectmode multiple -activestyle none]

    $lb insert end "Test 1 - Default Object Callbacks"
    $lb insert end "Test 2 - Valid Solid Object Variations"
    $lb insert end "Test 3 - Valid NURBS Object Variations"
    $lb insert end "Test 4 - Valid Tool Object Variations"
    $lb insert end "Test 5 - Modelling Tools"
    $lb insert end "Test 6 - All Solid Object Variations"
    $lb insert end "Test 7 - Custom Objects"
    $lb insert end "Test 8 - Script Objects"

    bind $lb <<ListboxSelect>> "aytest_handleLBS %W"

    pack $f.l1 -side top -fill both -expand yes

    addInfo $w aytestprefs TestSet
    addInfo $w aytestprefs TestItem
    addInfo $w aytestprefs TestVariant
    addInfo $w aytestprefs Status

    # test preferences
    set f [frame $w.fp]

    aytest_fillGUI

    pack $w.fp -side top -fill x -expand no
    pack $w.fu.fl -side left -fill both -expand yes

    set f [frame $w.fu.fr -relief sunken -border 1]

    # listbox for test item selection
    addText $f e0 "Select Item:"
    set lb [listbox $f.l1 -exportselection 0 -yscrollcommand "$f.sc set"\
		-selectmode multiple -activestyle none]
    $lb insert end "All"
    $lb selection set 0
    $lb conf -state disabled
    pack $f.l1 -side left -fill both -expand yes

    scrollbar $f.sc -command "$lb yview" -takefocus 0
    pack $f.sc -side right -fill y -expand no

    pack $w.fu.fr -side left -fill both -expand yes

    # ok/cancel buttons
    set f [frame $w.fl]

    button $f.brun -text "Run" -width 5 -command "\
   aytest_runTests \[$w.fu.fl.l1 curselection\] \[$w.fu.fr.l1 curselection\];"

    button $f.bstop -text "Stop" -width 5 -command "set ::cancelled 1"

    button $f.bclose -text "Close" -width 5 -command "focus .;destroy $w;"

    pack $f.brun $f.bstop $f.bclose -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    set aytestprefs(closebutton) $f.bclose

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bclose invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bclose invoke"

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-6.html aytesttcl

    # emergency exit
    bind $w <Control-KeyPress-C> {set cancelled 1;break}

    winCenter $w
    grab $w
    focus $w.fu.fl.l1
    tkwait window $w

    winAutoFocusOn

    # cleanup
    after idle {set ::cancelled 0}
    . configure -cursor {}

 return;
}
# aytest_selectGUI


#
# Test Default (Object) Callbacks
#
proc testDefaultCallbacks { types } {
set ::types $types
uplevel #0 {
puts $log "Testing default object callbacks...\n"

# these types do not support getPnt (when empty):
set nopnttypes ""
lappend nopnttypes Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon
lappend nopnttypes Cap Bevel ExtrNC ExtrNP OffsetNC OffsetNP ConcatNC ConcatNP
lappend nopnttypes Clone Mirror Script Text DSkin

# these types do not support conversion (or flag errors upon conversion
# attempts, when empty):
set noconvtypes ""
lappend noconvtypes Camera Light Material RiInc RiProc Select
lappend noconvtypes Cap Bevel Birail1 Birail2 ConcatNP Extrude OffsetNP Gordon
lappend noconvtypes Revolve Skin Sweep Swing Trim Text ExtrNP OffsetNC DSkin

set view1 ""
if { [winfo exists .fv.fViews.fview1.f3D.togl] } {
    set view1 .fv.fViews.fview1.f3D.togl
}
set view2 ""
if { [winfo exists .fv.fViews.fview2.f3D.togl] } {
    set view2 .fv.fViews.fview2.f3D.togl
}

puts -nonewline "Testing "
foreach type $types {
    set aytestprefs(TestItem) "${type}"
    puts -nonewline "${type}, "

    if { $aytestprefs(InitTypes) && ![info exists ${type}_props] } {
	eval init_${type}
    }

    puts $log "Creating a $type ...\n"
    crtOb $type
    hSL

    puts $log "Copying a $type ...\n"
    copOb
    pasOb
    hSL

    puts $log "Deleting a $type ...\n"
    delOb
    hSL

    puts $log "Get properties of $type ...\n"
    getProp

    puts $log "Set properties of $type ...\n"
    setProp

    puts $log "Saving a $type ...\n"
    saveScene $scratchfile 1

    puts $log "Reading a $type ...\n"
    insertScene $scratchfile
    catch {file delete $scratchfile}

    puts $log "Exporting a $type ...\n"
    wrib $scratchfile -selonly
    catch {file delete $scratchfile}

    if { [winfo exists $view1] } {
	puts $log "Drawing a $type ...\n"
	$view1 mc
	$view1 redraw
    }

    if { [winfo exists $view2] } {
	puts $log "Shading a $type ...\n"
	$view2 mc
	$view2 redraw
    }

    if { [winfo exists $view1] } {
	puts $log "Bounding box calculation of $type ...\n"
	$view1 mc
	$view1 zoomob
    }

    puts $log "Notifying a $type ...\n"
    notifyOb

    puts $log "Selecting points of $type ...\n"
    selPnts -all
    selPnts

    if { [lsearch -exact $nopnttypes $type] == -1 } {
	selPnts 0 2
	selPnts
    }

    if { [lsearch -exact $noconvtypes $type] == -1 } {
	puts $log "Converting a $type ...\n"
	convOb
	convOb -inplace
    }

    # missing tests: select points via action callback,
    # draw points/handles, (comparison - AI?), export?

    selOb
    goTop

    catch {viewCloseAll}

    if { $::cancelled } {
	break;
    }
}
# foreach

}
}
# testDefaultCallbacks


#
# Test Valid Solid Object Variations
#
proc testValidSolidVariations { types } {
set ::types $types
uplevel #0 {

puts $log "Testing valid solid object variations ...\n"

# standard angles to test for the ThetaMax
# parameter found in many objects
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 1 89 90 91 179 180 181 269 270 271 359 360}

# also some harmless float values
set floatvals {-1000 -100 -20 -2.5 -2 -1.5 -1.0 -0.9 -0.1 0.1 0.9 1.0 1.5 2 2.5 20 100 1000}

# Every object variation array contains the following components:
#  precmd - commands to run after object creation but before variation
#  valcmd - command to rule out invalid combinations of parameters
#  tstcmd - actual commands to test the implementation
#  arr - array to put all variable data into
#  fixedvars - list of fixed variables in array arr
#  fixedvals - list of value sets for all variables in fixedvars
#  freevars - list of free variables in array arr
#             (their values will be automatically
#              varied by a cartesian product!)
# for every free variable, another entry (named like the variable itself)
# exists, that contains the valid variable values, from which the cartesian
# product is built

#############
# Box
#############

# Box Variation #1
array set Box_1 {
    arr BoxAttrData
    freevars {Width Length Height}
    fixedvars {dummy}
    fixedvals { {0} }
}
set Box_1(Width) $floatvals
set Box_1(Length) $floatvals
set Box_1(Height) $floatvals
#set Box_1(tstcmd) {aytest_varcmds}


#############
# Sphere
#############

# Sphere Variation #1
array set Sphere_1 {
    arr SphereAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {Radius ZMin ZMax}
}
set Sphere_1(ThetaMax) $angles
#set Sphere_1(ThetaMax) {-180 -1 1 90}
#set Sphere_1(tstcmd) {aytest_varcmds}

lappend Sphere_1(fixedvals) { 1.0 -1.0 1.0 }
lappend Sphere_1(fixedvals) { 1.0 -0.1 0.1 }
lappend Sphere_1(fixedvals) { 1.0 -0.2 0.2 }
lappend Sphere_1(fixedvals) { 1.0 -0.5 0.5 }

lappend Sphere_1(fixedvals) { 1.0 -0.5 1.0 }
lappend Sphere_1(fixedvals) { 1.0 0.0 1.0 }
lappend Sphere_1(fixedvals) { 1.0 0.5 1.0 }
lappend Sphere_1(fixedvals) { 1.0 -1.0 -0.5 }
lappend Sphere_1(fixedvals) { 1.0 -1.0 0.0 }
lappend Sphere_1(fixedvals) { 1.0 -1.0 0.5 }


# Sphere Variation #2
array set Sphere_2 {
    arr SphereAttrData
    precmd {scalOb 0.5 0.5 0.5}
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {Radius ZMin ZMax}
}
set Sphere_2(ThetaMax) $angles

# take all valsets from Sphere_1 but adapt the radius
foreach valset $Sphere_1(fixedvals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 2.0]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Sphere_2(fixedvals) $val
}
# foreach


# Sphere Variation #3
array set Sphere_3 {
    arr SphereAttrData
    precmd {scalOb 2.0 2.0 2.0}
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {Radius ZMin ZMax}
}
set Sphere_3(ThetaMax) $angles

# take all valsets from Sphere_1 but adapt the radius
foreach valset $Sphere_1(fixedvals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 0.5]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Sphere_3(fixedvals) $val
}
# foreach


#############
# Cylinder
#############

# Cylinder Variation #1
array set Cylinder_1 {
    arr CylinderAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {Radius ZMin ZMax}
}
set Cylinder_1(ThetaMax) $angles

lappend Cylinder_1(fixedvals) { 1.0 -1.0 1.0 }
lappend Cylinder_1(fixedvals) { 1.0 -0.1 0.1 }
lappend Cylinder_1(fixedvals) { 1.0 -0.5 0.5 }
lappend Cylinder_1(fixedvals) { 1.0 -2.0 2.0 }
lappend Cylinder_1(fixedvals) { 1.0 0.0 1.0 }
lappend Cylinder_1(fixedvals) { 1.0 -1.0 0.0 }
lappend Cylinder_1(fixedvals) { 1.0 -2.0 -1.0 }
lappend Cylinder_1(fixedvals) { 1.0 1.0 2.0 }
lappend Cylinder_1(fixedvals) { 1.0 -2.1 -0.1 }
lappend Cylinder_1(fixedvals) { 1.0 0.1 2.1 }


# Cylinder Variation #2
array set Cylinder_2 {
    arr CylinderAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {Radius ZMin ZMax}
}
set Cylinder_2(ThetaMax) $angles

# take all valsets from Cylinder_1 but adapt the radius
foreach valset $Cylinder_1(fixedvals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 0.5]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Cylinder_2(fixedvals) $val
}
# foreach


# Cylinder Variation #3
array set Cylinder_3 {
    arr CylinderAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {Radius ZMin ZMax}
}
set Cylinder_3(ThetaMax) $angles

# take all valsets from Cylinder_1 but adapt the radius
foreach valset $Cylinder_1(fixedvals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 2.0]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Cylinder_3(fixedvals) $val
}
# foreach


#############
# Cone
#############

# Cone Variation #1
array set Cone_1 {
    arr ConeAttrData
    freevars {Closed ThetaMax Radius Height}
    Closed {0 1}
    Radius {0.1 0.5 1.0 1.5 2.0}
    Height {-2.0 -1.5 -1.0 -0.5 -0.1 0.1 0.5 1.0 1.5 2.0}
    fixedvars {}
    fixedvals {1}
}
set Cone_1(ThetaMax) $angles


#############
# Disk
#############

# Disk Variation #1
array set Disk_1 {
    arr DiskAttrData
    freevars {Radius Height ThetaMax}
    Radius {0.1 0.5 1.0 1.5 2.0}
    Height {-2.0 -1.5 -1.0 -0.5 -0.1 0.1 0.5 1.0 1.5 2.0}
    fixedvars {}
    fixedvals {1}
}
set Disk_1(ThetaMax) $angles


#############
# Hyperboloid
#############

# Hyperboloid Variation #1
array set Hyperboloid_1 {
    arr HyperboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {P1_X P1_Y P1_Z P2_X P2_Y P2_Z}
}
set Hyperboloid_1(ThetaMax) $angles

# a hyperboloid as you know it
lappend Hyperboloid_1(fixedvals) { 0.0 -1.0 -0.5  1.0 0.0 0.5 }
lappend Hyperboloid_1(fixedvals) { 1.0 0.0 0.5  0.0 -1.0 -0.5 }

# cylinder
lappend Hyperboloid_1(fixedvals) { 0.0 1.0 -0.5  0.0 1.0 0.5 }
lappend Hyperboloid_1(fixedvals) { 0.0 1.0 0.5  0.0 1.0 -0.5 }
lappend Hyperboloid_1(fixedvals) { 1.0 0.0 -0.5  1.0 0.0 0.5 }
lappend Hyperboloid_1(fixedvals) { 1.0 0.0 0.5  1.0 0.0 -0.5 }

# ring (disk with hole)
lappend Hyperboloid_1(fixedvals) { 0.5 0.0 0.0  1.0 0.0 0.0 }

# frustum (cone without apex)
lappend Hyperboloid_1(fixedvals) { 0.0 1.0 -0.5  0.0 0.5 0.5 }


#############
# Paraboloid
#############

# Paraboloid Variation #1
array set Paraboloid_1 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {RMax ZMin ZMax}
}
set Paraboloid_1(ThetaMax) $angles


lappend Paraboloid_1(fixedvals) { 1.0 0.0 0.5 }
lappend Paraboloid_1(fixedvals) { 1.0 0.0 1.0 }
lappend Paraboloid_1(fixedvals) { 1.0 0.0 1.5 }
lappend Paraboloid_1(fixedvals) { 1.0 0.0 2.0 }

lappend Paraboloid_1(fixedvals) { 1.0 0.1 0.5 }
lappend Paraboloid_1(fixedvals) { 1.0 0.1 1.0 }
lappend Paraboloid_1(fixedvals) { 1.0 0.1 1.5 }
lappend Paraboloid_1(fixedvals) { 1.0 0.1 2.0 }

lappend Paraboloid_1(fixedvals) { 1.0 1.0 0.1 }
lappend Paraboloid_1(fixedvals) { 1.0 1.0 0.5 }
lappend Paraboloid_1(fixedvals) { 1.0 1.0 0.9 }

lappend Paraboloid_1(fixedvals) { 1.0 2.0 0.1 }
lappend Paraboloid_1(fixedvals) { 1.0 2.0 0.5 }
lappend Paraboloid_1(fixedvals) { 1.0 2.0 0.9 }


# Paraboloid Variation #2
array set Paraboloid_2 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {RMax ZMin ZMax}
}
set Paraboloid_2(ThetaMax) $angles


# take all valsets from Paraboloid_1 but adapt RMax
foreach valset $Paraboloid_1(fixedvals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 0.5]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Paraboloid_2(fixedvals) $val
}
# foreach


# Paraboloid Variation #3
array set Paraboloid_3 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {RMax ZMin ZMax}
}
set Paraboloid_3(ThetaMax) $angles

# take all valsets from Paraboloid_1 but adapt RMax
foreach valset $Paraboloid_1(fixedvals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 2.0]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Paraboloid_3(fixedvals) $val
}
# foreach


#############
# Torus
#############

# Torus Variation #1
array set Torus_1 {
    arr TorusAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    fixedvars {MajorRad MinorRad PhiMin PhiMax}
}
set Torus_1(ThetaMax) $angles

lappend Torus_1(fixedvals) { 0.25 0.25 0.0 360.0 }
lappend Torus_1(fixedvals) { 0.75 0.25 0.0 360.0 }
lappend Torus_1(fixedvals) { 1.0 0.25 0.0 360.0 }
lappend Torus_1(fixedvals) { 1.0 0.5 0.0 360.0 }

lappend Torus_1(fixedvals) { 0.25 0.25 90.0 360.0 }
lappend Torus_1(fixedvals) { 0.75 0.25 90.0 360.0 }
lappend Torus_1(fixedvals) { 1.0 0.25 90.0 360.0 }
lappend Torus_1(fixedvals) { 1.0 0.5 90.0 360.0 }

lappend Torus_1(fixedvals) { 0.25 0.25 180.0 360.0 }
lappend Torus_1(fixedvals) { 0.75 0.25 180.0 360.0 }
lappend Torus_1(fixedvals) { 1.0 0.25 180.0 360.0 }
lappend Torus_1(fixedvals) { 1.0 0.5 180.0 360.0 }


# XXXX TODO: add torus variations for phimin/phimax

#set types Sphere
foreach type $types {
    set aytestprefs(TestItem) "${type}"

    puts $log "Testing $type ...\n"

    puts -nonewline "${type}, "

    aytest_var $type

    if { $::cancelled } {
	break;
    }
}
# foreach
puts -nonewline "\n"
}
}
# testValidSolidVariations

#
# Test 3 - Valid NURBS Object Variations
#

# aytest_crtnvars:
# helper to create variation arrays
# (also in use by modelling tools test)
proc aytest_crtnvars { { breadth 0 } } {
global NCurve_1 NCurve_2 NCurve_3 NCurve_4 NCurve_5 NCurve_6 ICurve_1 ACurve_1
global NPatch_1 NPatch_2 NPatch_3 NPatch_4 NPatch_5 NPatch_6 NPatch_7
global IPatch_1 APatch_1 PatchMesh_1 NCircle_1

# lengths/widths/heights to test
set lengthvals { 2 3 4 5 6 7 8 9 10 100 }

if { $breadth } {
    set lengthvals { 2 3 4 5 }
}

# orders to test
set ordervals { 2 3 4 5 6 7 8 9 10 }

if { $breadth } {
    set ordervals $lengthvals
}

# knot types to test
#  (deliberately omitting: 0 - Bezier, and 3 - Custom)
#  these will be handled by variations below
set ktvals { 1 2 4 5 }

# also some harmless float values
set floatvals {-1000 -100 -20 -2.5 -2 -1.5 -1.0 -0.9 -0.1 0.1 0.9 1.0 1.5 2 2.5 20 100 1000}


#############
# NCurve
#############

# NCurve Variation #1
array set NCurve_1 {
    arr NCurveAttrData
    freevars {Length Order Knot-Type}
    fixedvars {rational}
    fixedvals {0 1}
}
set NCurve_1(Length) $lengthvals
set NCurve_1(Order) $ordervals
set NCurve_1(Knot-Type) $ktvals
set NCurve_1(valcmd) {
    [expr {$::NCurveAttrData(Order) <= $::NCurveAttrData(Length)}]
}
set NCurve_1(precmd) {
    if { $l == 1 } {
	setPnt 0 0 0 0 0.5
    }
}

# NCurve Variation #2 (Bezier knots)
array set NCurve_2 {
    arr NCurveAttrData
    freevars {Length Order}
    fixedvars {Knot-Type}
    fixedvals { {0} }
}
set NCurve_2(Length) $lengthvals
set NCurve_2(Order) $ordervals
set NCurve_2(valcmd) {
    [expr {$::NCurveAttrData(Order) == $::NCurveAttrData(Length)}]
}


# NCurve Variation #3 (Custom knots)
array set NCurve_3 {
    arr NCurveAttrData
    freevars {Length Order}
    fixedvars {Knot-Type}
    fixedvals { {3} }
}
set NCurve_3(Length) $lengthvals
set NCurve_3(Order) $ordervals
set NCurve_3(valcmd) {
    [set ::NCurveAttrData(Knots-Modified) 1; set ::NCurveAttrData(Knots) [aytest_crtknots $::NCurveAttrData(Length) $::NCurveAttrData(Order)]] != ""
}

# rational variants of #2 and #3
array set NCurve_4 [array get NCurve_2]
set NCurve_4(precmd) {setPnt 0 0 0 0 0.5;}
array set NCurve_5 [array get NCurve_3]
set NCurve_5(precmd) {setPnt 0 0 0 0 0.5;}

# NCurve Variation #6 (Periodic curves)
array set NCurve_6 {
    arr NCurveAttrData
    freevars {Length Order Knot-Type}
    fixedvars {dummy}
    fixedvals { {0} }
}
set NCurve_6(Length) $lengthvals
set NCurve_6(Order) $ordervals
set NCurve_6(Knot-Type) $ktvals
set NCurve_6(valcmd) {
    [expr ($::NCurveAttrData(Length) - $::NCurveAttrData(Order)) > $::NCurveAttrData(Order)]
}
set NCurve_6(tstcmd) {set ::NCurveAttrData(Type) 2; setProp;}


#############
# ICurve
#############

# ICurve Variation #1
array set ICurve_1 {
    arr ICurveAttrData
    freevars {Type Length Order ParamType Derivatives SDLen EDLen}
    fixedvars {dummy}
    fixedvals { {0} }
}
set ICurve_1(Type) {0 1}
set ICurve_1(Length) $lengthvals
set ICurve_1(Order) [lrange $ordervals 1 end]
set ICurve_1(ParamType) {0 1 2}
set ICurve_1(Derivatives) {0 1}
set ICurve_1(SDLen) $floatvals
set ICurve_1(EDLen) $floatvals
set ICurve_1(valcmd) {
    [expr {$::ICurveAttrData(Order) <= $::ICurveAttrData(Length)}]
}


#############
# ACurve
#############

# ACurve Variation #1
array set ACurve_1 {
    arr ACurveAttrData
    freevars {Length ALength Order Closed Symmetric}
    fixedvars {dummy}
    fixedvals { {0} }
}
set ACurve_1(Length) [lrange $lengthvals 1 end]
set ACurve_1(ALength) $lengthvals
set ACurve_1(Order) $ordervals
set ACurve_1(Closed) {0 1}
set ACurve_1(Symmetric) {0 1}
set ACurve_1(valcmd) {
    [expr {($::ACurveAttrData(ALength) <= $::ACurveAttrData(Length)) &&
     ($::ACurveAttrData(Order) <= $::ACurveAttrData(ALength))}]
}


#############
# NPatch
#############

# NPatch Variation #1
array set NPatch_1 {
    arr NPatchAttrData
    freevars {Width Height Order_U Order_V Knot-Type_U Knot-Type_V}
    fixedvars {rational}
    fixedvals {0 1}
}
set NPatch_1(Width) $lengthvals
set NPatch_1(Height) $lengthvals
set NPatch_1(Order_U) $ordervals
set NPatch_1(Order_V) $ordervals
set NPatch_1(Knot-Type_U) $ktvals
set NPatch_1(Knot-Type_V) $ktvals
set NPatch_1(valcmd) {
    [expr {($::NPatchAttrData(Order_U) <= $::NPatchAttrData(Width)) &&
	   ($::NPatchAttrData(Order_V) <= $::NPatchAttrData(Height))}]
}
set NPatch_1(precmd) {
    if { $l == 1 } {
	setPnt 0 0 0 0 0 0.5
    }
}

# NPatch Variation #2 (Bezier knots U & V)
array set NPatch_2 {
    arr NPatchAttrData
    freevars {Width Height Order_U Order_V}
    fixedvars {Knot-Type_U Knot-Type_V}
    fixedvals { {0 0} }
}
set NPatch_2(Width) $lengthvals
set NPatch_2(Height) $lengthvals
set NPatch_2(Order_U) $ordervals
set NPatch_2(Order_V) $ordervals
set NPatch_2(valcmd) {
    [expr {($::NPatchAttrData(Order_U) == $::NPatchAttrData(Width)) &&
	   ($::NPatchAttrData(Order_V) == $::NPatchAttrData(Height))}]
}

# NPatch Variation #3 (Bezier knots U)
array set NPatch_3 {
    arr NPatchAttrData
    freevars {Width Height Order_U Order_V Knot-Type_V}
    fixedvars {Knot-Type_U}
    fixedvals { {0} }
}
set NPatch_3(Width) $lengthvals
set NPatch_3(Height) $lengthvals
set NPatch_3(Order_U) $ordervals
set NPatch_3(Order_V) $ordervals
set NPatch_3(Knot-Type_V) $ktvals
set NPatch_3(valcmd) {
    [expr {($::NPatchAttrData(Order_U) == $::NPatchAttrData(Width)) &&
	   ($::NPatchAttrData(Order_V) <= $::NPatchAttrData(Height))}]
}

# NPatch Variation #4 (Bezier knots V)
array set NPatch_4 {
    arr NPatchAttrData
    freevars {Width Height Order_U Order_V Knot-Type_U}
    fixedvars {Knot-Type_V}
    fixedvals { {0} }
}
set NPatch_4(Width) $lengthvals
set NPatch_4(Height) $lengthvals
set NPatch_4(Order_U) $ordervals
set NPatch_4(Order_V) $ordervals
set NPatch_4(Knot-Type_U) $ktvals
set NPatch_4(valcmd) {
    [expr {($::NPatchAttrData(Order_U) <= $::NPatchAttrData(Width)) &&
	   ($::NPatchAttrData(Order_V) == $::NPatchAttrData(Height))}]
}

# NPatch Variation #5 (Custom knots)
array set NPatch_5 {
    arr NPatchAttrData
    freevars {Width Height Order_U Order_V}
    fixedvars {Knot-Type_U Knot-Type_V}
    fixedvals { {3 3} }
}
set NPatch_5(Width) $lengthvals
set NPatch_5(Height) $lengthvals
set NPatch_5(Order_U) $ordervals
set NPatch_5(Order_V) $ordervals
set NPatch_5(valcmd) {
    ([set ::NPatchAttrData(Knots-Modified_U) 1;set ::NPatchAttrData(Knots_U) [aytest_crtknots $::NPatchAttrData(Width) $::NPatchAttrData(Order_U)]] != "") && ([set ::NPatchAttrData(Knots-Modified_V) 1;set ::NPatchAttrData(Knots_V) [aytest_crtknots $::NPatchAttrData(Height) $::NPatchAttrData(Order_V)]] != "")
}

# NPatch Variation #6 (Bezier knots U, Custom knots V)
array set NPatch_6 {
    arr NPatchAttrData
    freevars {Width Height Order_U Order_V}
    fixedvars {Knot-Type_U Knot-Type_V}
    fixedvals { {0 3} }
}
set NPatch_6(Width) $lengthvals
set NPatch_6(Height) $lengthvals
set NPatch_6(Order_U) $ordervals
set NPatch_6(Order_V) $ordervals
set NPatch_6(valcmd) {
    ($::NPatchAttrData(Order_U) == $::NPatchAttrData(Width)) && ([set ::NPatchAttrData(Knots-Modified_V) 1;set ::NPatchAttrData(Knots_V) [aytest_crtknots $::NPatchAttrData(Height) $::NPatchAttrData(Order_V)]] != "")
}

# NPatch Variation #7 (Custom knots U, Bezier knots V)
array set NPatch_7 {
    arr NPatchAttrData
    freevars {Width Height Order_U Order_V}
    fixedvars {Knot-Type_U Knot-Type_V}
    fixedvals { {3 0} }
}
set NPatch_7(Width) $lengthvals
set NPatch_7(Height) $lengthvals
set NPatch_7(Order_U) $ordervals
set NPatch_7(Order_V) $ordervals
set NPatch_7(valcmd) {
    ($::NPatchAttrData(Order_V) == $::NPatchAttrData(Height)) && ([set ::NPatchAttrData(Knots-Modified_U) 1;set ::NPatchAttrData(Knots_U) [aytest_crtknots $::NPatchAttrData(Width) $::NPatchAttrData(Order_U)]] != "")
}


#############
# IPatch
#############

# IPatch Variation #1
array set IPatch_1 {
    arr IPatchAttrData
    freevars {Width Height Order_U Order_V Close_U Close_V Knot-Type_U Knot-Type_V Derivatives_U Derivatives_V}
    fixedvars {dummy}
    fixedvals { {0} }
}
set IPatch_1(Width) $lengthvals
set IPatch_1(Height) $lengthvals
set IPatch_1(Order_U) $ordervals
set IPatch_1(Order_V) $ordervals
set IPatch_1(Close_U) {0 1}
set IPatch_1(Close_V) {0 1}
set IPatch_1(Knot-Type_U) {0 1 2}
set IPatch_1(Knot-Type_V) {0 1 2}
set IPatch_1(Derivatives_U) {0 1 2}
set IPatch_1(Derivatives_V) {0 1 2}
set IPatch_1(valcmd) {
    [expr {($::IPatchAttrData(Order_U) <= $::IPatchAttrData(Width)) &&
	   ($::IPatchAttrData(Order_V) <= $::IPatchAttrData(Height))}]
}


#############
# APatch
#############

# APatch Variation #1
array set APatch_1 {
    arr APatchAttrData
    freevars {Mode Width Height AWidth AHeight Order_U Order_V Close_U Close_V Knot-Type_U Knot-Type_V}
    fixedvars {dummy}
    fixedvals { {0} }
}
set APatch_1(Mode) {0 1}
set APatch_1(Width) $lengthvals
set APatch_1(Height) $lengthvals
set APatch_1(AWidth) $lengthvals
set APatch_1(AHeight) $lengthvals
set APatch_1(Order_U) $ordervals
set APatch_1(Order_V) $ordervals
set APatch_1(Close_U) {0 1}
set APatch_1(Close_V) {0 1}
set APatch_1(Knot-Type_U) {0 1}
set APatch_1(Knot-Type_V) {0 1}

set APatch_1(valcmd) {
    [expr {($::APatchAttrData(AWidth) <= $::APatchAttrData(Width)) &&
	   ($::APatchAttrData(AHeight) <= $::APatchAttrData(Height)) &&
	   ($::APatchAttrData(Order_U) <= $::APatchAttrData(AWidth)) &&
	   ($::APatchAttrData(Order_V) <= $::APatchAttrData(AHeight))}]
}


#############
# PatchMesh
#############

# PatchMesh Variation #1 (bilinear)
array set PatchMesh_1 {
    arr PatchMeshAttrData
    freevars {Width Height Close_U Close_V}
    fixedvars { Type }
    fixedvals { {1} }
}
set PatchMesh_1(Width) $lengthvals
set PatchMesh_1(Height) $lengthvals
set PatchMesh_1(Close_U) {0 1}
set PatchMesh_1(Close_V) {0 1}


# PatchMesh Variation #2 (bicubic)
set l 4
while { $l < 50 } {
    lappend pmlengths $l
    incr l
}

array set PatchMesh_2 {
    arr PatchMeshAttrData
    freevars {Width Height Close_U Close_V BType_U BType_V}
    fixedvars {dummy}
    fixedvals { {0} }
}
set PatchMesh_2(Width) $pmlengths
set PatchMesh_2(Height) $pmlengths
set PatchMesh_2(Close_U) {0 1}
set PatchMesh_2(Close_V) {0 1}
set PatchMesh_2(BType_U) {0 1 2 3 4}
set PatchMesh_2(BType_V) {0 1 2 3 4}

set PatchMesh_2(valcmd) {
    [aytest_checkpml $::PatchMeshAttrData(Width) $::PatchMeshAttrData(BType_U) $::PatchMeshAttrData(Close_U) $::PatchMeshAttrData(Height) $::PatchMeshAttrData(BType_V) $::PatchMeshAttrData(Close_V)]
}


#############
# NCircle
#############

# NCircle Variation #1
array set NCircle_1 {
    arr NCircleAttrData
    freevars {TMin TMax}
    fixedvars { dummy }
    fixedvals { {0} }
}
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 1 89 90 91 179 180 181 269 270 271 359 360}
set NCircle_1(TMin) $angles
set NCircle_1(TMax) $angles
set NCircle_1(valcmd) {
    [expr {$::NCircleAttrData(TMin) != $::NCircleAttrData(TMax)}]
}

}
# aytest_crtnvars

proc testValidNURBS { types } {
set ::types $types
uplevel #0 {

puts $log "Testing valid NURBS object variations ...\n"

aytest_crtnvars

###
foreach type $types {
    set aytestprefs(TestItem) "${type}"

    puts $log "Testing $type ...\n"

    puts -nonewline "${type}, "

    aytest_var $type

    if { $::cancelled } {
	break;
    }
}
# foreach
puts -nonewline "\n"
}
}
# testValidNURBS

# aytest_crtknots:
# create a custom knot vector
proc aytest_crtknots { l o } {
    global aytestknots

    if { $l < $o } {
	return "";
    }

    set vname "aytestknots("
    append vname $l "+" $o ")"

    if { [info exists $vname] } {
	return [subst "\$$vname"];
    }

    set i 0
    while { $i < $o } {
	lappend v 0.0
	incr i
    }
    set d 0.5
    set k $d
    while { $i < $l } {
	lappend v $k
	set k [expr {$k+$d}]
	if { $i < 3 } {
	    set d [expr {$d*0.5}]
	}
	incr i
    }
    set l [expr {$l+$o}]
    while { $i < $l } {
	lappend v $k
	incr i
    }

    set $vname $v

 return $v;
}
# aytest_crtknots


# aytest_checkpml:
# check PatchMesh parameter configuration for validity wrt. implied
# basis step size
proc aytest_checkpml { w tu cu h tv cv } {
    set su [lindex { 3 1 3 2 4 1 } $tu]
    set sv [lindex { 3 1 3 2 4 1 } $tv]
    if { $w == 4 } {
	set w $su
    }
    if { $h == 4 } {
	set h $sv
    }
    # closed?
    if { $cu == 1 } {
	incr w -1
    }
    if { $cv == 1 } {
	incr h -1
    }
    if { [expr {(fmod($w,$su) < 1e-06) && (fmod($h,$sv) < 1e-06)}] == 1 } {
	return 1;
    }
 return 0;
}
# aytest_checkpml


#
# Test Valid Tool Object Variations
#
proc testValidToolObjects { types } {
set ::types $types
uplevel #0 {

puts $log "Testing valid tool object variations ...\n"

# standard angles to test for the ThetaMax
# parameter found in many objects
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 1 89 90 91 179 180 181 269 270 271 359 360}

# also some harmless float values
set floatvals {-1000 -100 -20 -2.5 -2 -1.5 -1.0 -0.9 -0.1 0.1 0.9 1.0 1.5 2 2.5 20 100 1000}

# Revolve Variation #1
array set Revolve_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {crtOb NCurve}
	    1 {crtOb NCurve -length 2}
	    2 {crtOb NCurve -length 3}
	    3 {
		crtOb NCurve; hSL;
		getProp; set ::NCurveAttrData(Knot-Type) 1; setProp
	    }
	    4 {
		crtClosedBS -s 4; hSL; movOb 1.25 0 0;
	    }
	};
	notifyOb;
	goUp;
	hSL
    }
    arr RevolveAttrData
    freevars {ThetaMax}
    fixedvars {Sections Order}
}
set Revolve_1(ThetaMax) $angles

# repeating the same value set leads to multiple calls
# of the precmd, thus enabling it to check the value of
# <l> (set by aytest_var) and creating a different parameter
# curve for each iteration
lappend Revolve_1(fixedvals) { 0 0 }
lappend Revolve_1(fixedvals) { 0 0 }
lappend Revolve_1(fixedvals) { 0 0 }
lappend Revolve_1(fixedvals) { 0 0 }
lappend Revolve_1(fixedvals) { 0 0 }


# Revolve Variation #2
array set Revolve_2 {
    precmd {goDown -1;crtOb NCurve;hSL;movOb 1 0 0;rotOb 0 0 90;
	notifyOb;goUp;hSL}
    arr RevolveAttrData
    freevars {UpperCap LowerCap StartCap EndCap ThetaMax}
    UpperCap {0 1}
    LowerCap {0 1}
    StartCap {0 1}
    EndCap {0 1}
    fixedvars {Sections Order}
}
set Revolve_2(ThetaMax) $angles

lappend Revolve_2(fixedvals) { 0 0 }


# Revolve Variation #3
array set Revolve_3 {
    precmd {goDown -1;crtOb NCurve;hSL;movOb 1 0 0;rotOb 0 0 90;
	notifyOb;goUp;hSL}
    arr RevolveAttrData
    freevars {UpperCap LowerCap StartCap EndCap ThetaMax Sections Order}
    UpperCap {0 1}
    LowerCap {0 1}
    StartCap {0 1}
    EndCap {0 1}
    Sections { 2 5 10 100 }
    Order { 2 3 4 5 6 }
    fixedvars { dummy }
    fixedvals { {0} }
}
set Revolve_3(ThetaMax) $angles


# Extrude Variation #1
array set Extrude_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {crtOb NCurve}
	    1 {crtOb NCurve -length 2}
	    2 {crtOb NCurve -length 3}
	    3 {
		crtOb NCurve; hSL;
		getProp; set ::NCurveAttrData(Knot-Type) 1; setProp
	    }
	    4 {
		crtClosedBS -s 4; hSL; movOb 1.25 0 0;
	    }
	    5 {	crtOb NCircle }
	    6 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	notifyOb;
	goUp;
	hSL
    }
    arr ExtrudeAttrData
    fixedvars {Height}
}

lappend Extrude_1(fixedvals) { 1.0 }
lappend Extrude_1(fixedvals) { 1.0 }
lappend Extrude_1(fixedvals) { 1.0 }
lappend Extrude_1(fixedvals) { 1.0 }
lappend Extrude_1(fixedvals) { 1.0 }
lappend Extrude_1(fixedvals) { 1.0 }
lappend Extrude_1(fixedvals) { 1.0 }


# Extrude Variation #2
array set Extrude_2 {
    precmd {goDown -1;crtClosedBS -s 4;hSL;movOb 1.25 0.0 0.0;notifyOb;goUp;hSL}
    arr ExtrudeAttrData
    freevars {StartCap EndCap}
    StartCap {0 1}
    EndCap {0 1}
    fixedvars {Height}
}

lappend Extrude_2(fixedvals) { 0.1 }
lappend Extrude_2(fixedvals) { 0.5 }
lappend Extrude_2(fixedvals) { 1.0 }
lappend Extrude_2(fixedvals) { 2.0 }
lappend Extrude_2(fixedvals) { 10.0 }


# Extrude Variation #3
array set Extrude_3 {
    precmd {goDown -1;crtOb NCircle;hSL;movOb 1.25 0.0 0.0;notifyOb;goUp;hSL}
    arr ExtrudeAttrData
    freevars {StartCap EndCap}
    StartCap {0 1}
    EndCap {0 1}
    fixedvars {Height}
}

lappend Extrude_3(fixedvals) { 0.1 }
lappend Extrude_3(fixedvals) { 0.5 }
lappend Extrude_3(fixedvals) { 1.0 }
lappend Extrude_3(fixedvals) { 2.0 }
lappend Extrude_3(fixedvals) { 10.0 }


# XXXX TODO: add bevels, add holes


# Sweep Variation #1
array set Sweep_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; rotOb 0 0 90 ;
	crtOb NCurve; notifyOb; goUp; hSL
    }
    arr SweepAttrData
    freevars {Rotate Interpolate}
    Rotate {0 1}
    Interpolate {0 1}
    fixedvars { dummy }
    fixedvals { {0} }
}


# Sweep Variation #2
array set Sweep_2 {
    precmd {
	goDown -1;
	crtClosedBS -s 4; hSL; rotOb 0 90 0;
	crtOb NCurve; notifyOb; goUp; hSL
    }
    arr SweepAttrData
    freevars {Rotate Interpolate StartCap EndCap}
    Rotate {0 1}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    fixedvars { dummy }
    fixedvals { {0} }
}


# Sweep Variation #3
array set Sweep_3 {
    precmd {
	goDown -1;
	crtOb NCircle; hSL; rotOb 0 90 0;
	crtOb NCurve; notifyOb; goUp; hSL
    }
    arr SweepAttrData
    freevars {Rotate Interpolate StartCap EndCap}
    Rotate {0 1}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    fixedvars { dummy }
    fixedvals { {0} }
}

# XXXX TODO: add bevels


# Birail1 Variation #1
array set Birail1_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; rotOb 0 0 -90;
	crtOb NCurve; crtOb NCurve; hSL; movOb 0 0.75 0;
	notifyOb; goUp; hSL
    }
    arr Birail1AttrData
    freevars {Sections}
    Sections {0 1 2 3 4 5 50}
    fixedvars { dummy }
    fixedvals { {0} }
}


# Birail2 Variation #1
array set Birail2_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; rotOb 0 0 -90;
	crtOb NCurve; crtOb NCurve; hSL; movOb 0 0.75 0;
	crtOb NCurve; hSL; rotOb 0 0 -90; movOb 0.75 0 0;
	notifyOb; goUp; hSL
    }
    arr Birail2AttrData
    freevars {Sections InterpolCtrl}
    Sections {0 1 2 3 4 5 50}
    InterpolCtrl {0 1}
    fixedvars { dummy }
    fixedvals { {0} }
}


# Gordon Variation #1
array set Gordon_1 {
    precmd {
	crtOb NCurve;
	crtOb NCurve; hSL; movOb 0 0.75 0;
	crtOb Level 0;
	crtOb NCurve; hSL; rotOb 0 0 -90;
	crtOb NCurve; hSL; rotOb 0 0 -90; movOb 0.75 0 0;
	hSL 5; cutOb;
	goDown -1;
	pasOb -move
	notifyOb; goUp; hSL
    }
    arr GordonAttrData
    freevars {Order_U Order_V}
    Order_U {2 3 4 5}
    Order_V {2 3 4 5}
    fixedvars { dummy }
    fixedvals { {0} }
}


# DSkin Variation #1
array set DSkin_1 {
    arr DSkinAttrData
    freevars {Order_U Order_V}
    Order_U {2 3 4 5}
    Order_V {2 3 4 5}
    fixedvars { dummy }
    fixedvals { {0} }
}

set DSkin_1(precmd) $Gordon_1(precmd)


# Skin Variation #1
array set Skin_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; movOb 0 1 0;
	crtOb NCurve; notifyOb; goUp; hSL
    }
    arr SkinAttrData
    fixedvars { dummy }
    fixedvals { {0} }
}

# Skin Variation #2
array set Skin_2 {
    precmd {
	goDown -1;
	crtClosedBS -s 4; hSL; movOb 0 1 1;
	crtClosedBS -s 4; notifyOb; goUp; hSL
    }
    arr SkinAttrData
    freevars {StartCap EndCap}
    StartCap {0 1}
    EndCap {0 1}
    fixedvars { dummy }
    fixedvals { {0} }
}

# Skin Variation #3
array set Skin_3 {
    precmd {
	goDown -1;
	crtClosedBS -s 4; hSL; movOb 0 4 4;
	crtClosedBS -s 4; hSL; movOb 0 3 3;
	crtClosedBS -s 4; hSL; movOb 0 1 2;
	crtClosedBS -s 4; hSL; movOb 0 1 0;
	crtClosedBS -s 4; notifyOb; goUp; hSL
    }
    arr SkinAttrData
    freevars {Interpolate StartCap EndCap Knot-Type_U Order_U}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    Knot-Type_U { 1 2 3 }
    Order_U { 2 3 4 5 }
    fixedvars { dummy }
    fixedvals { 0 }
}

# Skin Variation #4
array set Skin_4 {
    precmd {
	goDown -1;
	switch $l {
	    0 {
		crtClosedBS -s 4; hSL; movOb 0 4 4;
		crtClosedBS -s 4; hSL; movOb 0 3 3;
		crtClosedBS -s 4; hSL; movOb 0 1 2;
		crtClosedBS -s 4; hSL; movOb 0 1 0;
		crtClosedBS -s 4;
	    }
	    1 {
		crtClosedBS -s 4; hSL; movOb 0 4 4;
		crtClosedBS -s 4; hSL; movOb 0 3 3;
		crtClosedBS -s 4; hSL; movOb 0 1 0;
		crtClosedBS -s 4;
	    }
	    2 {
		crtClosedBS -s 4; hSL; movOb 0 4 4;
		crtClosedBS -s 4; hSL; movOb 0 1 0;
		crtClosedBS -s 4;
	    }
	}
	notifyOb; goUp; hSL
    }
    arr SkinAttrData
    freevars {StartCap EndCap}
    StartCap {0 1}
    EndCap {0 1}
    fixedvars { Knot-Type_U Order_U }
}

lappend Skin_4(fixedvals) {0 5}
lappend Skin_4(fixedvals) {0 4}
lappend Skin_4(fixedvals) {0 3}

# Skin Variation #5
array set Skin_5 {
    precmd {
	goDown -1;
	switch $l {
	    0 {
		crtClosedBS -s 4; hSL; movOb 0 3 3;
		crtClosedBS -s 5; hSL; movOb 0 1.5 0;
		crtClosedBS -s 4;
	    }
	    1 {
		crtClosedBS -s 4; hSL; movOb 0 3 3;
		crtOb NCurve; hSL; movOb 0 1.5 0;
		crtClosedBS -s 4;
	    }
	    2 {
		crtClosedBS -s 4; hSL; movOb 0 3 3;
		crtOb NCircle; hSL; movOb 0 1.5 0;
		crtClosedBS -s 4;
	    }
	    3 {
		crtOb NCircle; hSL; movOb 0 3 3; rotOb 45 0 0;
		crtOb NCircle; hSL; movOb 0 1.5 0;
		crtClosedBS -s 4;hSL; rotOb -45 0 0;
	    }
	}
	notifyOb; goUp; hSL
    }
    arr SkinAttrData
    freevars {Interpolate StartCap EndCap Knot-Type_U Order_U}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    Knot-Type_U { 1 2 3 }
    Order_U { 2 3 4 5 }
    fixedvars { dummy }
}

lappend Skin_5(fixedvals) { 0 }
lappend Skin_5(fixedvals) { 0 }
lappend Skin_5(fixedvals) { 0 }
lappend Skin_5(fixedvals) { 0 }


# XXXX TODO: add bevels


# Cap Variation #1
array set Cap_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {	crtClosedBS -s 4; }
	    1 {	crtOb NCircle }
	    2 {
		crtClosedBS -s 4; hSL; movOb 1.25 0 0;
	    }
	    3 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	notifyOb;
	goUp;
	hSL
    }
    arr CapAttrData
    freevars {Type Fraction}
    Type {0 1 2 3}
    fixedvars {dummy}
}
set Cap_1(Fraction) $floatvals

lappend Cap_1(fixedvals) { 0 }
lappend Cap_1(fixedvals) { 0 }
lappend Cap_1(fixedvals) { 0 }
lappend Cap_1(fixedvals) { 0 }


# Bevel Variation #1
array set Bevel_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {	crtClosedBS -s 4; }
	    1 {	crtOb NCircle }
	    2 {
		crtClosedBS -s 4; hSL; movOb 1.25 0 0;
	    }
	    3 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	notifyOb;
	goUp;
	hSL
    }
    arr BevelAttrData
    fixedvars {dummy}
}

lappend Bevel_1(fixedvals) { 0 }
lappend Bevel_1(fixedvals) { 0 }
lappend Bevel_1(fixedvals) { 0 }
lappend Bevel_1(fixedvals) { 0 }


# ExtrNC Variation #1
array set ExtrNC_1 {
    precmd {
	goDown -1;
	crtOb NPatch;
	hSL;
	getProp;
	switch $l {
	    1 { set ::NPatchAttrData(Knot-Type_U) 1; }
	    2 { set ::NPatchAttrData(Knot-Type_V) 1; }
	    3 { set ::NPatchAttrData(Knot-Type_U) 1;
		set ::NPatchAttrData(Knot-Type_V) 1; }
	}
	setProp;
	notifyOb;
	goUp;
	hSL
    }
    arr ExtrNCAttrData

    freevars { Side Parameter Relative Revert }

    Side { 0 1 2 3 4 5 6 7 8 }
    Parameter { 0.0 0.1 0.33 0.5 0.66 0.9 1.0 }
    Relative { 0 1 }
    Revert { 0 1 }

    fixedvars {dummy}
}

lappend ExtrNC_1(fixedvals) { 0 }
lappend ExtrNC_1(fixedvals) { 0 }
lappend ExtrNC_1(fixedvals) { 0 }
lappend ExtrNC_1(fixedvals) { 0 }


# ExtrNP Variation #1
array set ExtrNP_1 {
    precmd {
	goDown -1;
	crtOb NPatch;
	notifyOb;
	goUp;
	hSL
    }
    arr ExtrNPAttrData
    fixedvars { UMin UMax VMin VMax }
}

lappend ExtrNP_1(fixedvals) { 0.1 0.9 0.1 0.9 }
lappend ExtrNP_1(fixedvals) { 0.5 1.0 0.1 0.9 }
lappend ExtrNP_1(fixedvals) { 0.5 1.0 0.5 1.0 }


# OffsetNC Variation #1
array set OffsetNC_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {crtOb NCurve}
	    1 {crtOb NCurve -length 2}
	    2 {crtOb NCurve -length 3}
	    3 {
		crtOb NCurve; hSL;
		getProp; set ::NCurveAttrData(Knot-Type) 1; setProp
	    }
	    4 {
		crtClosedBS -s 4; hSL; movOb 1.25 0 0;
	    }
	    5 {	crtOb NCircle }
	    6 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	notifyOb;
	goUp;
	hSL
    }
    freevars { Mode Revert }
    Mode { 0 1 }
    Revert { 0 1 }
    arr OffsetNCAttrData
    fixedvars {Offset}
}

for {set i 0} {$i < 7} {incr i} {
  lappend OffsetNC_1(fixedvals) { 0.1 }
}

# OffsetNP Variation #1
array set OffsetNP_1 {
    precmd {
	goDown -1;
	crtOb NPatch;
	notifyOb;
	goUp;
	hSL
    }
    arr OffsetNPAttrData
    fixedvars { Offset }
}

lappend OffsetNP_1(fixedvals) { 0.1 }
lappend OffsetNP_1(fixedvals) { 0.5 }
lappend OffsetNP_1(fixedvals) { 1.0 }
lappend OffsetNP_1(fixedvals) { -0.1 }
lappend OffsetNP_1(fixedvals) { -0.5 }
lappend OffsetNP_1(fixedvals) { -1.0 }


# ConcatNC Variation #1
array set ConcatNC_1 {
    precmd {
	goDown -1;
	crtOb NCurve; crtOb NCurve; hSL; movOb 1 0 0;
	notifyOb; goUp; hSL
    }
    arr ConcatNCAttrData
    fixedvars { dummy }
    fixedvals { {0} }
}


# ConcatNP Variation #1
array set ConcatNP_1 {
    precmd {
	goDown -1;
	crtOb NPatch; crtOb NPatch; hSL;
	switch $l {
	    0 {
		movOb 1 0 0;
	    }
	    1 {
		movOb 2 0 0;
	    }
	    2 {
		movOb 2 2 0;
	    }
	};
	notifyOb; goUp; hSL
    }
    arr ConcatNPAttrData
    fixedvars { dummy }
}

lappend ConcatNP_1(fixedvals) { 0 }
lappend ConcatNP_1(fixedvals) { 0 }
lappend ConcatNP_1(fixedvals) { 0 }


# Trim Variation #1
array set Trim_1 {
    precmd {
	goDown -1;
	crtOb NPatch; crtOb NCircle; hSL; scalOb 0.5 0.5 1.0; movOb 0.5 0.5 0;
	notifyOb; goUp; hSL
    }
    arr TrimAttrData
    fixedvars { dummy }
    fixedvals { {0} }
}

###
puts -nonewline "Testing "
foreach type $types {
    set aytestprefs(TestItem) "${type}"

    puts $log "Testing $type ...\n"

    puts -nonewline "${type}, "
    update
    aytest_var $type

    if { $::cancelled } {
	break;
    }
}
# foreach
puts -nonewline "\n"
}
}
# testValidToolObjects


#
# Test Modelling Tools
#
proc testModellingTools { tools } {
set ::aytesttools $tools
uplevel #0 {

set lengths {4 5 6}

array set Revert {
    types { NCurve ICurve ACurve NCircle ConcatNC }
    command { revertC; }
}

array set RevertUS {
    types { BPatch NPatch IPatch APatch PatchMesh }
    command { revertuS }
}

array set RevertVS {
    types { BPatch NPatch IPatch APatch PatchMesh }
    command { revertvS }
}

array set SwapUVS {
    types { BPatch NPatch IPatch APatch PatchMesh }
    command { swapuvS }
}

array set Refine {
    types { NCurve ICurve ACurve }
    command { refineC }
}

array set Coarsen {
    types { NCurve ICurve ACurve }
    command { coarsenC }
}

array set RefineUNP {
    types { NPatch }
    command { refineuNP }
}

array set RefineVNP {
    types { NPatch }
    command { refinevNP }
}

array set RefineK {
    types { NCurve }
    command { refineknNC }
}

array set InsertK {
    types { NCurve }
    command { insknNC 0.5 1 }
}

array set InsertK2 {
    types { NCurve }
    command { insknNC 0.5 2 }
}

array set RemoveK {
    types { NCurve }
    command {
	getProp
	set kn [lindex $::NCurveAttrData(Knots) $::NCurveAttrData(Order)]
	# set kn 1 => CRASH
	remknNC $kn 1 Inf
    }
}

array set RemoveKS {
    types { NCurve }
    command {
	getProp
	set kn [lindex $::NCurveAttrData(Knots) $::NCurveAttrData(Order)]
	# set kn 1 => CRASH
	remknNC $kn 1 0.0
    }
}

array set RemoveKI {
    types { NCurve }
    command {
	remknNC -i 0 1 Inf
    }
}

array set RemoveKIS {
    types { NCurve }
    command {
	remknNC -i 0 1 0.0
    }
}

array set RemoveKI1 {
    types { NCurve }
    command {
	remknNC -i 1 1 Inf
    }
}

array set RemoveKI1S {
    types { NCurve }
    command {
	remknNC -i 1 1 0.0
    }
}

array set RemoveKIEnd {
    types { NCurve }
    command {
	getProp
	set kn [expr $::NCurveAttrData(Length)+$::NCurveAttrData(Order)-1]
	remknNC -i $kn 1 Inf
    }
}

array set RemoveKIEndS {
    types { NCurve }
    command {
	getProp
	set kn [expr $::NCurveAttrData(Length)+$::NCurveAttrData(Order)-1]
	remknNC -i $kn 1 0.0
    }
}

array set RemoveSuK {
    types { NCurve }
    command {
	set kn 0.5
	while { 1 } {
	    set ::ay_error 0
	    insknNC $kn 1
	    if { $::ay_error == 0 } break;
	    incr kn 0.1
	}
	remsuknNC
    }
}

array set TrimNC {
    types { NCurve }
    command {
	getProp
	set index [expr $::NCurveAttrData(Order)-1]
	set k1 [lindex $::NCurveAttrData(Knots) $index]
	incr index
	set k2 [lindex $::NCurveAttrData(Knots) $index]
	set kn1 [expr {$k1+($k2-$k1)*0.25}]
	set kn2 [expr {$k1+($k2-$k1)*0.75}]
	trimNC $kn1 $kn2
    }
}

array set TweenNC {
    types { NCurve }
    command {
	set index [getSel]
	copOb
	pasOb -move
	hSL
	movOb 0.1 0.1 0.0
	rotOb 0.0 0.0 30.0
	selOb $index [expr {$index+1}]
	tweenNC
	selOb $index [expr {$index+1}] [expr {$index+2}]
    }
}

array set InterpNC {
    types { NCurve }
    command {
	getProp
	copOb
	interpNC
	pasOb -move
	hSL
	interpNC -o [expr $NCurveAttrData(Order) + 1]
    }
}

array set InterpNCC {
    types { NCurve }
    command {
	getProp
	interpNC -c 1
	pasOb -move
	hSL
	interpNC -c 1 -o [expr $NCurveAttrData(Order) + 1]
    }
}

array set ApproxNC {
    types { NCurve }
    command {
	getProp;
	if { $::NCurveAttrData(Length) > 2 } { approxNC }
    }
}

array set ApproxNCC {
    types { NCurve }
    command {
	getProp;
	if { $::NCurveAttrData(Length) > 2 } { approxNC -c 1 }
    }
}
array set FairNC {
    types { NCurve }
    command {
	getProp;
	if { $::NCurveAttrData(Length) > 2 && $::NCurveAttrData(Order) > 2 } {
	    fairNC
	}
    }
}

array set FairNPU {
    types { NPatch }
    command {
	getProp;
	if { $::NPatchAttrData(Width) > 2 && $::NPatchAttrData(Order_U) > 2 } {
	    copOb
	    fairNP
	    pasOb
	    hSL
	    fairNP -w 1
	    pasOb -move
	    hSL
	    selPnts 0 1 [expr $::NPatchAttrData(Width) *
			 $::NPatchAttrData(Height) - 1]
	    fairNP
	    fairNP -w 1
	}
    }
}

array set FairNPV {
    types { NPatch }
    command {
	getProp;
	if { $::NPatchAttrData(Height) > 2 && $::NPatchAttrData(Order_V) > 2 } {
	    fairNP -m 1
	    pasOb
	    hSL
	    fairNP -m 1 -w 1
	    pasOb -move
	    hSL
	    selPnts 0 1 [expr $::NPatchAttrData(Width) *
			 $::NPatchAttrData(Height) - 1]
	    fairNP -m 1
	    fairNP -m 1 -w 1
	}
    }
}
array set FairNPUV {
    types { NPatch }
    command {
	getProp;
	if { $::NPatchAttrData(Width) > 2 && $::NPatchAttrData(Order_U) > 2 &&
	     $::NPatchAttrData(Height) > 2 && $::NPatchAttrData(Order_V) > 2 } {
	    fairNP -m 2
	    pasOb
	    hSL
	    fairNP -m 2 -w 1
	    pasOb -move
	    hSL
	    selPnts 0 1 [expr $::NPatchAttrData(Width) *
			 $::NPatchAttrData(Height) - 1]
	    fairNP -m 2
	    fairNP -m 2 -w 1
	}
    }
}
array set FairNPVU {
    types { NPatch }
    command {
	getProp;
	if { $::NPatchAttrData(Width) > 2 && $::NPatchAttrData(Order_U) > 2 &&
	     $::NPatchAttrData(Height) > 2 && $::NPatchAttrData(Order_V) > 2 } {
	    fairNP -m 3
	    pasOb
	    hSL
	    fairNP -m 3 -w 1
	    pasOb -move
	    hSL
	    selPnts 0 1 [expr $::NPatchAttrData(Width) *
			 $::NPatchAttrData(Height) - 1]
	    fairNP -m 3
	    fairNP -m 3 -w 1
	}
    }
}
array set SplitNC {
    types { NCurve }
    command {
	getProp
	set index [expr $::NCurveAttrData(Order)-1]
	set k1 [lindex $::NCurveAttrData(Knots) $index]
	incr index
	set k2 [lindex $::NCurveAttrData(Knots) $index]
	set kn [expr {$k1+($k2-$k1)*0.5}]
	splitNC $kn; hSL; delOb; hSL
    }
}

array set ExtractNC {
    types { NCurve }
    command {
	getProp
	extrNC -relative 0.1 0.9; hSL; delOb; hSL
    }
}

array set SplitNPU {
    types { NPatch }
    command {
	getProp
	set index [expr $::NPatchAttrData(Order_U)-1]
	set k1 [lindex $::NPatchAttrData(Knots_U) $index]
	incr index
	set k2 [lindex $::NPatchAttrData(Knots_U) $index]
	set kn [expr {$k1+($k2-$k1)*0.5}]
	splituNP $kn; hSL; delOb; hSL
    }
}

array set SplitNPV {
    types { NPatch }
    command {
	getProp
	set index [expr $::NPatchAttrData(Order_V)-1]
	set k1 [lindex $::NPatchAttrData(Knots_V) $index]
	incr index
	set k2 [lindex $::NPatchAttrData(Knots_V) $index]
	set kn [expr {$k1+($k2-$k1)*0.5}]
	splitvNP $kn; hSL; delOb; hSL
    }
}

array set Open {
    types { NCurve ICurve ACurve ConcatNC }
    command { openC }
}

array set Close {
    types { NCurve ICurve ACurve ConcatNC }
    command { closeC }
}

array set ShiftC {
    types { NCurve ICurve ACurve }
    command { shiftC }
}

array set ShiftC2 {
    types { NCurve ICurve ACurve }
    command { shiftC 2 }
}

array set ShiftCM {
    types { NCurve ICurve ACurve }
    command { shiftC -1 }
}

array set ShiftCM2 {
    types { NCurve ICurve ACurve }
    command { shiftC -2 }
}

array set ToXYC {
    types { NCurve ICurve ACurve }
    command {
	setPnt 0 0.5 0.5 0.5;
	toXYC
    }
}

array set ToXZC {
    types { NCurve ICurve ACurve }
    command {
	setPnt 0 0.5 0.5 0.5;
	toXZC
    }
}

array set ToYZC {
    types { NCurve ICurve ACurve }
    command {
	setPnt 0 0.5 0.5 0.5;
	toYZC
    }
}

array set EstLenNC {
    types { NCurve }
    command { estlenNC len }
}

array set ElevateNC {
    types { NCurve }
    command { elevateNC }
}

array set ElevateNC2 {
    types { NCurve }
    command { elevateNC 2 }
}

array set ElevateNC3 {
    types { NCurve }
    command { elevateNC 3 }
}

array set ReduceNC {
    types { NCurve }
    command { elevateNC; reduceNC }
}

array set ExtendNC {
    types { NCurve }
    command { extendNC 10.0 20.0 30.0 }
}

array set ClampNC {
    types { NCurve }
    command { clampNC }
}

array set ClampNCS {
    types { NCurve }
    command { clampNC -s }
}

array set ClampNCE {
    types { NCurve }
    command { clampNC -e }
}

array set UnclampNC {
    types { NCurve }
    command { unclampNC }
}

array set UnclampNCS {
    types { NCurve }
    command { unclampNC -s }
}

array set UnclampNCE {
    types { NCurve }
    command { unclampNC -e }
}

array set ClampUNP {
    types { NPatch }
    command { clampuNP }
}

array set ClampUNPS {
    types { NPatch }
    command { clampuNP -s }
}

array set ClampUNPE {
    types { NPatch }
    command { clampuNP -e }
}

array set ClampVNP {
    types { NPatch }
    command { clampvNP }
}

array set ClampVNPS {
    types { NPatch }
    command { clampvNP -s }
}

array set ClampVNPE {
    types { NPatch }
    command { clampvNP -e }
}

array set UnclampUNP {
    types { NPatch }
    command { unclampuNP }
}

array set UnclampUNPS {
    types { NPatch }
    command { unclampuNP -s }
}

array set UnclampUNPE {
    types { NPatch }
    command { unclampuNP -e }
}

array set UnclampVNP {
    types { NPatch }
    command { unclampvNP }
}

array set UnclampVNPS {
    types { NPatch }
    command { unclampvNP -s }
}

array set UnclampVNPE {
    types { NPatch }
    command { unclampvNP -e }
}

array set ElevateUNP {
    types { NPatch }
    command { elevateuNP }
}

array set ElevateUNP2 {
    types { NPatch }
    command { elevateuNP 2 }
}

array set ElevateUNP3 {
    types { NPatch }
    command { elevateuNP 3 }
}

array set ElevateVNP {
    types { NPatch }
    command { elevatevNP }
}

array set ElevateVNP2 {
    types { NPatch }
    command { elevatevNP 2 }
}

array set ElevateVNP3 {
    types { NPatch }
    command { elevatevNP 3 }
}

array set ReduceUNP {
    types { NPatch }
    command { elevateuNP; reduceuNP }
}

array set ReduceVNP {
    types { NPatch }
    command { elevatevNP; reducevNP }
}

array set InsertKUNP {
    types { NPatch }
    command { insknuNP 0.5 1 }
}

array set InsertK2UNP {
    types { NPatch }
    command { insknuNP 0.5 2 }
}

array set InsertKVNP {
    types { NPatch }
    command { insknvNP 0.5 1 }
}

array set InsertK2VNP {
    types { NPatch }
    command { insknvNP 0.5 2 }
}

array set RemoveKUNP {
    types { NPatch }
    command {
	getProp
	set kn [lindex $::NPatchAttrData(Knots_U) $::NPatchAttrData(Order_U)]
	remknuNP $kn 1 Inf
    }
}

array set RemoveKIUNP {
    types { NPatch }
    command {
	remknuNP -i 0 1 Inf
    }
}

array set RemoveKVNP {
    types { NPatch }
    command {
	getProp
	set kn [lindex $::NPatchAttrData(Knots_V) $::NPatchAttrData(Order_V)]
	remknvNP $kn 1 Inf
    }
}

array set RemoveKIVNP {
    types { NPatch }
    command {
	remknvNP -i 0 1 Inf
    }
}

array set RemoveSuKUNP {
    types { NPatch }
    command {
	set kn 0.5
	while { 1 } {
	    set ::ay_error 0
	    insknuNP $kn 1
	    if { $::ay_error == 0 } break;
	    incr kn 0.1
	}
	remsuknuNP
    }
}

array set RemoveSuKVNP {
    types { NPatch }
    command {
	set kn 0.5
	while { 1 } {
	    set ::ay_error 0
	    insknvNP $kn 1
	    if { $::ay_error == 0 } break;
	    incr kn 0.1
	}
	remsuknvNP
    }
}

array set CloseUNP {
    types { NPatch }
    command { closeuNP }
}

array set CloseVNP {
    types { NPatch }
    command { closevNP }
}

array set TweenNP {
    types { NPatch }
    command {
	set index [getSel]
	copOb
	pasOb -move
	hSL
	movOb 0.1 0.1 0.0
	rotOb 0.0 0.0 30.0
	selOb $index [expr {$index+1}]
	tweenNP
	selOb $index [expr {$index+1}] [expr {$index+2}]
    }
}


# instead of using the full palette of possible derivative lengths
# of ICurve_1, we content ourselves with 0.1/1.0 variations here
set trimic {set ICurve_1(SDLen) {0.1 1.0}; set ICurve_1(EDLen) {0.1 1.0};}

foreach tool { Revert Refine Coarsen Open Close ToXYC ToXZC ToYZC } {
    set ${tool}(precmd) $trimic
}

aytest_crtnvars $aytestprefs(Breadth)

# test modelling tools
puts $log "Testing modelling tools ...\n"
foreach tool $aytesttools {
    puts $log "Testing $tool ...\n"
    puts -nonewline "${tool}, "
    update
    set cmd [subst "\$${tool}(command)"]
    set types [subst "\$${tool}(types)"]
    foreach type $types {
	set aytestprefs(TestItem) "${tool} - ${type}"

	if { [info exists ${tool}(precmd)] } {
	    eval [subst "\$${tool}(precmd)"]
	}
	set i 1
	while { [info exists ${type}_${i}] } {
	    set ${type}_${i}(tstcmd) $cmd
	    incr i
	}
	aytest_var $type
    }
    # foreach type

    if { $::cancelled } {
	break;
    }
}
# foreach tool
puts -nonewline "\n"

}
}
# testModellingTools


#
# Test All Solid Object Variations
#
proc testAllSolidVariations { types } {
set ::types $types
uplevel #0 {

puts $log "Testing all solid object variations (Errors expected!) ...\n"

# XXXX add NaN, Inf?
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 -0.1 -0 0 0.1 1 89 90 91 179 180 181 269 270 271 359 360}
set floatvals {-1000 -100 -20 -2.5 -2 -1.5 -1.0 -0.9 -0.1 -0 0 0.1 0.9 1.0 1.5 2 2.5 20 100 1000}


#############
# Box
#############

array set Box_1 {
    arr BoxAttrData
    tstcmd {aytest_varcmds}
    freevars {Width Length Height}
    fixedvars {dummy}
    fixedvals { {0} }
}
set Box_1(Width) $floatvals
set Box_1(Length) $floatvals
set Box_1(Height) $floatvals


#############
# Sphere
#############

array set Sphere_1 {
    arr SphereAttrData
    tstcmd {aytest_varcmds}
    freevars {Closed ThetaMax Radius ZMin ZMax}
    Closed {0 1}
    fixedvars {dummy}
    fixedvals { {0} }
}
set Sphere_1(ThetaMax) $angles
set Sphere_1(Radius) $floatvals
set Sphere_1(ZMin) $floatvals
set Sphere_1(ZMax) $floatvals


#############
# Cylinder
#############

array set Cylinder_1 {
    arr CylinderAttrData
    tstcmd {aytest_varcmds}
    freevars {Closed ThetaMax Radius ZMin ZMax}
    Closed {0 1}
    fixedvars {dummy}
    fixedvals { {0} }
}

set Cylinder_1(ThetaMax) $angles
set Cylinder_1(Radius) $floatvals
set Cylinder_1(ZMin) $floatvals
set Cylinder_1(ZMax) $floatvals


#############
# Cone
#############

array set Cone_1 {
    arr ConeAttrData
    tstcmd {aytest_varcmds}
    freevars {Closed ThetaMax Radius Height}
    Closed {0 1}
    fixedvars {dummy}
    fixedvals { {0} }
}
set Cone_1(ThetaMax) $angles
set Cone_1(Radius) $floatvals
set Cone_1(Height) $floatvals


#############
# Disk
#############

array set Disk_1 {
    arr DiskAttrData
    tstcmd {aytest_varcmds}
    freevars {Radius Height ThetaMax}
    fixedvars {dummy}
    fixedvals { {0} }
}
set Disk_1(ThetaMax) $angles
set Disk_1(Radius) $floatvals
set Disk_1(Height) $floatvals


#############
# Hyperboloid
#############

array set Hyperboloid_1 {
    arr HyperboloidAttrData
    tstcmd {aytest_varcmds}
    freevars {Closed ThetaMax P1_X P1_Y P1_Z}
    Closed {0 1}
    fixedvars {P2_X P2_Y P2_Z}
}
lappend Hyperboloid_1(fixedvals) { 0.0 0.0 0.0 }
lappend Hyperboloid_1(fixedvals) { 1.0 -1.0 1.0 }
set Hyperboloid_1(ThetaMax) $angles
set Hyperboloid_1(P1_X) $floatvals
set Hyperboloid_1(P1_Y) $floatvals
set Hyperboloid_1(P1_Z) $floatvals


array set Hyperboloid_2 {
    arr HyperboloidAttrData
    tstcmd {aytest_varcmds}
    freevars {Closed ThetaMax P2_X P2_Y P2_Z}
    Closed {0 1}
    fixedvars {P1_X P1_Y P1_Z}
}
lappend Hyperboloid_2(fixedvals) { 0.0 0.0 0.0 }
lappend Hyperboloid_2(fixedvals) { 1.0 -1.0 1.0 }
set Hyperboloid_2(ThetaMax) $angles
set Hyperboloid_2(P2_X) $floatvals
set Hyperboloid_2(P2_Y) $floatvals
set Hyperboloid_2(P2_Z) $floatvals


#############
# Paraboloid
#############

array set Paraboloid_1 {
    arr ParaboloidAttrData
    tstcmd {aytest_varcmds}
    freevars {Closed ThetaMax RMax ZMin ZMax}
    Closed {0 1}
    fixedvars {dummy}
    fixedvals { {0} }
}
set Paraboloid_1(ThetaMax) $angles
set Paraboloid_1(RMax) $floatvals
set Paraboloid_1(ZMin) $floatvals
set Paraboloid_1(ZMax) $floatvals


#############
# Torus
#############

array set Torus_1 {
    arr TorusAttrData
    tstcmd {aytest_varcmds}
    freevars {Closed ThetaMax MajorRad MinorRad PhiMin PhiMax}
    Closed {0 1}
    fixedvars {dummy}
    fixedvals { {0} }
}
set Torus_1(ThetaMax) $angles
set Torus_1(MajorRad) $floatvals
set Torus_1(MinorRad) $floatvals
set Torus_1(PhiMin) $angles
set Torus_1(PhiMax) $angles

###
foreach type $types {
    set aytestprefs(TestItem) "${type}"
    puts $log "Testing $type ...\n"
    puts "Testing $type ..."
    aytest_var $type

    if { $::cancelled } {
	break;
    }
}
# foreach

}
}
# testAllSolidVariations


#
# Test Custom Objects
#
proc testCustomObjects { types } {
set ::types $types
uplevel #0 {
puts $log "Testing custom objects callbacks...\n"

# these types do not support getPnt (when empty):
set nopnttypes ""
lappend nopnttypes MetaObj

# these types do not support conversion (or flag errors upon conversion
# attempts, when empty):
set noconvtypes ""
#lappend noconvtypes

set view1 ""
if { [winfo exists .fv.fViews.fview1.f3D.togl] } {
    set view1 .fv.fViews.fview1.f3D.togl
}
set view2 ""
if { [winfo exists .fv.fViews.fview2.f3D.togl] } {
    set view2 .fv.fViews.fview2.f3D.togl
}

puts -nonewline "Testing "
foreach type $types {
    set aytestprefs(TestItem) "${type}"

    puts -nonewline "${type}, "

    loadPlugin ${type}

    puts $log "Creating a $type ...\n"
    crtOb $type
    hSL

    puts $log "Copying a $type ...\n"
    copOb
    pasOb
    hSL

    puts $log "Deleting a $type ...\n"
    delOb
    hSL

    puts $log "Get properties of $type ...\n"
    getProp

    puts $log "Set properties of $type ...\n"
    setProp

    puts $log "Saving a $type ...\n"
    saveScene $scratchfile 1

    puts $log "Reading a $type ...\n"
    insertScene $scratchfile
    catch {file delete $scratchfile}

    puts $log "Exporting a $type ...\n"
    wrib $scratchfile -selonly
    catch {file delete $scratchfile}

    if { [winfo exists $view1] } {
	puts $log "Drawing a $type ...\n"
	$view1 mc
	$view1 redraw
    }

    if { [winfo exists $view2] } {
	puts $log "Shading a $type ...\n"
	$view2 mc
	$view2 redraw
    }

    if { [winfo exists $view1] } {
	puts $log "Bounding box calculation of $type ...\n"
	$view1 mc
	$view1 zoomob
    }

    puts $log "Notifying a $type ...\n"
    notifyOb

    puts $log "Selecting points of $type ...\n"
    selPnts -all
    selPnts

    if { [lsearch -exact $nopnttypes $type] == -1 } {
	selPnts 0 2
	selPnts
    }

    if { [lsearch -exact $noconvtypes $type] == -1 } {
	puts $log "Converting a $type ...\n"
	convOb
	convOb -inplace
    }

    # missing tests: select points via action callback,
    # draw points/handles, (comparison - AI?), export?

    selOb
    goTop

    catch {viewCloseAll}

    if { $::cancelled } {
	break;
    }
}
# foreach

}
}
# testCustomObjects

#
# Test Script Objects
#
proc testScriptObjects { types } {
set ::types $types
uplevel #0 {
puts $log "Testing script objects implementation and callbacks...\n"

array set extruden_1 {
    precmd {
	goDown -1
	switch $l {
	    0 { crtClosedBS -s 4; }
	    1 { crtOb NCircle; }
	}
	notifyOb
	goUp
	hSL
    }
}

array set tsurf_1 {
    precmd {
	goDown -1
	crtOb NCurve
	crtOb NCurve
	hSL
	rotOb 0 0 45
	goUp
	hSL
    }
}

array set dualsweep_1 {
    precmd {
	goDown -1
	crtOb NCurve -length 3 -cv {0 0 1 1  0 0.5 0.5 1  0 1 1 1}
	crtClosedBS -s 4
	hSL
	rotOb 0 90 90
	copOb
	pasOb -move
	hSL
	movOb 0 1 0
	goUp
	hSL
    }
}

# CRASH
array set dualsweep_2 {
    precmd {
	goDown -1
	crtClosedBS -s 4
	crtClosedBS -s 4
	hSL
	movOb 0 0 1
	crtOb NCurve -length 3 -cv {1 0 0 1  2 0 0.5 1  1 0 1 1}
	goUp
	hSL
    }
}

# these types do not support getPnt (when empty):
set nopnttypes ""
#lappend nopnttypes

# these types do not support conversion (or flag errors upon conversion
# attempts, when empty):
set noconvtypes ""
#lappend noconvtypes

set view1 ""
if { [winfo exists .fv.fViews.fview1.f3D.togl] } {
    set view1 .fv.fViews.fview1.f3D.togl
}
set view2 ""
if { [winfo exists .fv.fViews.fview2.f3D.togl] } {
    set view2 .fv.fViews.fview2.f3D.togl
}

puts -nonewline "Testing "
foreach type $types {
    puts -nonewline "${type}, "
    set aytestprefs(TestItem) "${type}"

    crtOb Script
    hSL

    getProp
    catch [set scfile [open scripts/${type}.tcl r]]
    if { $scfile != "" } {
	set script [read $scfile]
    }
    set ScriptAttrData(Script) $script
    set stype [lindex $::testScriptObjectsTypes \
		   [lsearch $::testScriptObjectsItems $type]]
    set ScriptAttrData(Type) $stype
    setProp

    # create children
    if { [info exists ${type}_1(precmd)] } {
	set l 0
	eval [set ${type}_1(precmd)]
    }

    # activate the script
    getProp
    set ScriptAttrData(Active) 1
    setProp

    # now test the callbacks

    puts $log "Save...\n"
    saveScene $scratchfile 1

    puts $log "Read...\n"
    insertScene $scratchfile
    catch {file delete $scratchfile}

    puts $log "Export...\n"
    wrib $scratchfile -selonly
    catch {file delete $scratchfile}

    if { [winfo exists $view1] } {
	puts $log "Get BB...\n"
	$view1 mc
	$view1 zoomob
	puts $log "Draw...\n"
	$view1 mc
	$view1 redraw
    }

    if { [winfo exists $view2] } {
	$view2 mc
	$view2 zoomob
	puts $log "Shade...\n"
	$view2 mc
	$view2 redraw
    }

    puts $log "Notify...\n"
    notifyOb

    puts $log "Select Pnts...\n"
    selPnts -all
    selPnts
    selPnts 0 2
    selPnts

    puts $log "Convert...\n"
    convOb
    convOb -inplace

    # cleanup
    if { ! $::aytestprefs(KeepObjects) } {
	delOb
    }

    if { $::cancelled } {
	break;
    }
    incr i
}
# foreach

}
}
# testScriptObjects


# aytest_varcmds:
#  what to do with the object variants
#
proc aytest_varcmds { } {
uplevel #0 {

cancelled

# create a level and move the newly created object to it
copOb
crtOb Level
goDown -1
pasOb
hSL

set view1 ""
if { [winfo exists .fv.fViews.fview1.f3D.togl] } {
    set view1 .fv.fViews.fview1.f3D.togl
}
set view2 ""
if { [winfo exists .fv.fViews.fview2.f3D.togl] } {
    set view2 .fv.fViews.fview2.f3D.togl
}

puts $log "Copy...\n"
copOb
pasOb
hSL

puts $log "Delete...\n"
delOb
hSL

puts $log "Get properties...\n"
getProp

puts $log "Set properties...\n"
setProp

puts $log "Save...\n"
saveScene $scratchfile 1

puts $log "Read...\n"
insertScene $scratchfile
catch {file delete $scratchfile}

puts $log "Export...\n"
wrib $scratchfile -selonly
catch {file delete $scratchfile}

if { [winfo exists $view1] } {
    puts $log "Get BB...\n"
    $view1 mc
    $view1 zoomob
    puts $log "Draw...\n"
    $view1 mc
    $view1 redraw
}

if { [winfo exists $view2] } {
    $view2 mc
    $view2 zoomob
    puts $log "Shade...\n"
    $view2 mc
    $view2 redraw
}

puts $log "Notify...\n"
notifyOb

puts $log "Select Pnts...\n"
selPnts -all
selPnts
selPnts 0 2
selPnts

puts $log "Convert...\n"
convOb
convOb -inplace

# missing tests: select points via action callback,
# draw points/handles, (comparison - AI?)

# cleanup
goUp
hSL
delOb
hSL

catch {viewCloseAll}

}
}
# aytest_varcmds


# forall:
# create cartesian product of a list of lists
# and run a cmd for every resulting combination
# taken from: http://wiki.tcl.tk/2546
# (Cartesian product of a list of lists)
# original author: Eric Boudaillier;
# Synopsis:
#  forall var1 list1 var2 list2 [varn listn] cmd
#  example:
#  forall a {1 2 3} b {3 4} {puts "$a $b"}
#  =>
#  1 3
#  1 4
#  2 3
#  2 4
#  3 3...
proc forall {args} {
    if {[llength $args] < 3 || [llength $args] % 2 == 0} {
	return -code error "wrong \# args: should be \"forall varList list ?varList list ...? body\""
    }
    set body [lindex $args end]
    set args [lrange $args 0 end-1]
    while {[llength $args]} {
	set varName [lindex $args end-1]
	set list    [lindex $args end]
	set args    [lrange $args 0 end-2]
	set body    [list foreach $varName $list $body]
    }
    uplevel 1 $body
}
# forall


# aytest_var:
#  test object variations
#  ToDo: make final body command configurable
proc aytest_var { type } {
  global aytestprefs

  set i 1
  set totalvars 0
  while {[info exists ::${type}_$i]} {
      incr totalvars
      incr i
  }

  set i 1
  while {[info exists ::${type}_$i]} {

      set aytestprefs(TestVariant) "$i/$totalvars"

      # create a Level for each object type variation tested
      crtOb Level
      goDown -1

      eval set arr \$::${type}_${i}(arr)
      eval set fixedvars \$::${type}_${i}(fixedvars)
      eval set fixedvals \$::${type}_${i}(fixedvals)
      set l 0

      foreach valset $fixedvals {

	  # create a Level for each value set tested
	  crtOb Level
	  goDown -1

	  # create a prototype object
	  crtOb $type
	  hSL

	  # execute prerequisites
	  if { [info exists ::${type}_${i}(precmd)] } {
	      eval set precmd \$::${type}_${i}(precmd)
	      eval $precmd
	  }

	  # set fixed variables from the valset
	  getProp
	  set j 0
	  while { $j < [llength $fixedvars] } {
	      set ::${arr}([lindex $fixedvars $j]) [lindex $valset $j]
	      incr j
	  }
	  setProp

	  # valset has free variables?
	  if { [info exists ::${type}_${i}(freevars)] } {
	      # yes, set free variables

	      set body "forall "
	      eval set freevars \$::${type}_${i}(freevars)
	      foreach freevar $freevars {
		  append body ::${arr}($freevar)
		  append body " "
		  eval set freevals \$::${type}_${i}($freevar)
		  lappend body $freevals
		  append body " "
	      }

	      if { $aytestprefs(ShowProgress) } {
		  set aytestprefs(Progress) 0.0
		  update
		  set countbody $body
		  set ::tot 0
		  append countbody "\{incr ::tot;\}"
		  catch {eval $countbody}
		  set ::cur 0
		  set aytestprefs(Progress) 1.0
		  update
	      }

	      set k 0
	      set cmds ""
	      if { [info exists ::${type}_${i}(valcmd)] } {
		  append cmds "if \{ "
		  eval append cmds \$::${type}_${i}(valcmd)
		  append cmds "\} \{ "
	      }
	      if { $aytestprefs(DebugCreate) } {
		  append cmds {\
		      selOb 0;copOb;pasOb;hSL;
		      set ay_error 0
		      setProp;
		      if { $ay_error != 0 } { parray \$::${type}_${i}(arr) }
		      movOb $k $l $i;
		      incr k 2;\
		  }
	      } else {
		  append cmds {\
		      selOb 0;copOb;pasOb;hSL;
		      setProp;getProp;
		      movOb $k $l $i;
		      incr k 2;\
		  }
	      }

	      if { [info exists ::${type}_${i}(tstcmd)] } {
		  eval append cmds \$::${type}_${i}(tstcmd)
	      }
	      if { ! $::aytestprefs(KeepObjects) } {
		  append cmds ";delOb;"
	      }
	      if { [info exists ::${type}_${i}(valcmd)] } {
		  append cmds " \};"
	      }

	      if { $aytestprefs(ShowProgress) } {
       append cmds { incr ::cur; set newpro [expr $::cur*100.0/$::tot]; }
       append cmds { global aytestprefs;
	            if { $newpro > [expr $aytestprefs(Progress) + 0.1] } \
			 {
			     set aytestprefs(Progress) $newpro; update;
			     if { $::cancelled } {
				 return;
			     }
			 }
                    }
	      } else {
		  append cmds {
		      update; if { $::cancelled } {return;}
		  }
	      }
	      # if moreoptions

	      lappend body $cmds

	      # call forall with test commands
	      catch {eval $body}

	      if { $::cancelled } {
		  return;
	      }
	      selOb 0
	  } else {
	      # no, there are no free variables

	      movOb $j $i 0.0;

	      if { [info exists ::${type}_${i}(tstcmd)] } {
		  eval \$::${type}_${i}(tstcmd)
	      }
	  }
	  # if
	  incr l

	  if { ! $::aytestprefs(KeepObjects) } {
	      delOb
	  }

	  goUp

	  if { ! $::aytestprefs(KeepObjects) } {
	      hSL;delOb
	  }
      }
      # foreach
      rV
      incr i

      goUp

      if { ! $::aytestprefs(KeepObjects) } {
	  hSL;delOb
      }
  }
  # while

  if { $::cancelled } {
      return;
  }

  if { $aytestprefs(ShowProgress) } {
      set aytestprefs(Progress) 100; update;
  }

 return;
}
# aytest_var


# aytest_runTests:
#  actually run all tests selected in the GUI
#
proc aytest_runTests { tests items } {
    global ayprefs aytestprefs

    if { ($tests == {}) || ($items == {}) } {
	return;
    }

    set aytestprefs(TestItem) "n/a"
    set aytestprefs(Status) "Running"

    $aytestprefs(closebutton) conf -state disabled

    . configure -cursor watch
    .testGUI configure -cursor watch
    if { [winfo exists .fl.con] == 1 } {
	set oldprompt ${::.fl.con(-prompt)}
	set ::.fl.con(-prompt) ""
    }
    update
    set ::cancelled 0

    foreach test $tests {
	set aytestprefs(TestSet) [.testGUI.fu.fl.l1 get $test]
	set testproc [lindex $aytestprefs(TestProcs) $test]

	set testitems ""
	if { [lindex $items 0] == 0 } {
	    # All Items
	    set aytestprefs(Breadth) 1
	    eval set testitems \$::${testproc}Items
	} else {
	    # Selected Items
	    set aytestprefs(Breadth) 0
	    eval set allitems \$::${testproc}Items
	    foreach item $items {
		incr item -1
		lappend newitems [lindex $allitems $item]
	    }
	    if { [info exists newitems] } {
		catch {set testitems $newitems}
	    } else {
		set testitems ""
	    }
	}

	if { [llength $testitems] == 0 } {
	    ayError 2 "Test Ayam" "No items selected/to test!"
	    set aytestprefs(Status) "Select Items"
	    return;
	}

	if { $aytestprefs(RandomOrder) } {
	    set nTestItems [llength $testitems]
	    set randomIndices {}
	    while { [llength $randomIndices] < $nTestItems} {
		set newRandomIndex [expr {int(rand()*$nTestItems)}]
		while { [lsearch -int $randomIndices $newRandomIndex] > -1 } {
		    set newRandomIndex [expr {int(rand()*$nTestItems)}]
		}
		lappend randomIndices $newRandomIndex
	    }
	    foreach index $randomIndices {
		lappend randomList [lindex $testitems $index]
	    }
	    set testitems $randomList
	}

	set ::scratchfile [file join $ayprefs(TmpDir) aytestscratchfile.ay]

	set ::logfile [file join $ayprefs(TmpDir) aytestlog]
	set ::log [open $::logfile a]

	newScene
	selOb

	set aytestprefs(Progress) 0.0

	incr test

	puts "Running test $test..."

	catch {$testproc $testitems}

	close $::log

	if { $::cancelled } {
	    set ::cancelled 0
	    set aytestprefs(Status) "Cancelled"
	    puts "\nCancelled test $test..."
	    break;
	}

	puts "\nFinished test $test..."
    }
    # foreach

    . configure -cursor {}
    if { [winfo exists .testGUI] } {
	.testGUI configure -cursor {}
    }
    if { [winfo exists .fl.con] == 1 } {
	set ::.fl.con(-prompt) $oldprompt
	Console:prompt .fl.con
    }
    update

    set aytestprefs(TestItem) "n/a"
    set aytestprefs(TestVariant) "n/a"
    if { $aytestprefs(Status) != "Cancelled" } {
	set aytestprefs(Status) "Finished"
    }

    $aytestprefs(closebutton) conf -state normal

 return;
}
# aytest_runTests

###

# set up items to test in test #1
set items {}
lappend items Box Sphere Cylinder Cone Disk Hyperboloid Paraboloid Torus
lappend items NCurve ICurve ACurve NCircle
lappend items NPatch IPatch APatch BPatch PatchMesh
lappend items Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon DSkin
lappend items Cap Bevel ExtrNC ExtrNP OffsetNC OffsetNP ConcatNC ConcatNP
lappend items Trim Text
lappend items Camera Light Material RiInc RiProc Script Select
lappend items Clone Mirror
set testDefaultCallbacksItems $items

# set up items to test in test #2
set items {}
lappend items Box Sphere Cylinder Disk Cone Paraboloid Torus Hyperboloid
set testValidSolidVariationsItems $items

# set up items to test in test #3
set items {}
lappend items NCurve ICurve ACurve NPatch IPatch APatch PatchMesh NCircle
set testValidNURBSItems $items

# set up items to test in test #4
set items {}
lappend items Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon DSkin
lappend items Cap Bevel ExtrNC ExtrNP OffsetNC OffsetNP ConcatNC ConcatNP
lappend items Trim
set testValidToolObjectsItems $items

# set up tools to test in test #5
set items {}
lappend items Revert Open Close Refine Coarsen
lappend items RefineK InsertK InsertK2 RemoveK RemoveKI RemoveKS RemoveKIS
lappend items RemoveKI1 RemoveKI1S RemoveKIEnd RemoveKIEndS RemoveSuK
lappend items ShiftC ShiftC2 ShiftCM ShiftCM2 ToXYC ToXZC ToYZC
lappend items ElevateNC ElevateNC2 ElevateNC3 ReduceNC EstLenNC ExtendNC
lappend items SplitNC ExtractNC TrimNC TweenNC InterpNC InterpNCC
lappend items ApproxNC ApproxNCC FairNC ClampNC ClampNCS ClampNCE
lappend items UnclampNC UnclampNCS UnclampNCE
lappend items RevertUS RevertVS SwapUVS RefineUNP RefineVNP
lappend items ClampUNP ClampUNPS ClampUNPE
lappend items ClampVNP ClampVNPS ClampVNPE
lappend items UnclampUNP UnclampUNPS UnclampUNPE
lappend items UnclampVNP UnclampVNPS UnclampVNPE
lappend items ElevateUNP ElevateUNP2 ElevateUNP3
lappend items ElevateVNP ElevateVNP2 ElevateVNP3
lappend items ReduceUNP ReduceVNP
lappend items InsertKUNP InsertK2UNP InsertKVNP InsertK2VNP
lappend items RemoveKUNP RemoveKIUNP RemoveKVNP RemoveKIVNP
lappend items RemoveSuKUNP RemoveSuKVNP
lappend items SplitNPU SplitNPV CloseUNP CloseVNP TweenNP
lappend items FairNPU FairNPV FairNPUV FairNPVU
set testModellingToolsItems $items

# set up items to test in test #6
set items {}
lappend items Box Sphere Cylinder Cone Disk Hyperboloid Paraboloid Torus
set testAllSolidVariationsItems $items

# set up items to test in test #7
set items {}
lappend items MetaObj SfCurve SDNPatch BCurve SDCurve
set testCustomObjectsItems $items

# set up items to test in test #8
set items {}
lappend items cbox tcircle tcone spiral helix extruden tsurf dualsweep
set testScriptObjectsItems $items

# set up script object types corresponding to each test item
set types {}
lappend types 1 1 1 1 1 2 2 2
set testScriptObjectsTypes $types

###

# everything is set, start the GUI
aytest_selectGUI
