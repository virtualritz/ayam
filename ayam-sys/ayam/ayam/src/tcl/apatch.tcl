# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2021 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# apatch.tcl - approximating curves objects Tcl code

proc init_APatch { } {
    global ay APatch_props APatchAttr APatchAttrData

    set APatch_props { Transformations Attributes Tags APatchAttr }

    array set APatchAttr {
	arr   APatchAttrData
	sproc ""
	gproc ""
	w     fAPatchAttr
    }

    array set APatchAttrData {
	Mode 1
	DisplayMode 0
	NPInfoBall "N/A"
	Knot-Type_U 0
	Knot-Type_V 0
    }

    # create APatchAttr-UI
    set w [frame $ay(pca).$APatchAttr(w)]
    set a $APatchAttr(arr)

    addVSpace $w s1 2
    addParam $w $a Width
    addParam $w $a Height

    addVSpace $w s2 4
    addMenu $w $a Mode [list UV VU U V]
    addParam $w $a AWidth
    addParam $w $a AHeight
    addParam $w $a Order_U
    addParam $w $a Order_V
    addMenu $w $a Knot-Type_U [list Chordal Centripetal]
    addMenu $w $a Knot-Type_V [list Chordal Centripetal]
    addCheck $w $a Close_U
    addCheck $w $a Close_V

    addVSpace $w s3 4
    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(ncdisplaymodes)

    addVSpace $w s4 4
    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_APatch
