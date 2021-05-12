# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# icurve.tcl - interpolating curves objects Tcl code

# icurve_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc icurve_getAttr { } {
    global ay ayprefs ICurveAttr ICurveAttrData

    set oldfocus [focus]

    catch {destroy $ay(pca).$ICurveAttr(w)}
    set w [frame $ay(pca).$ICurveAttr(w)]

    getProp

    set ay(bok) $ay(appb)

    # create ICurveAttr-UI
    set a $ICurveAttr(arr)

    addVSpace $w s1 2
    addMenu $w $a Type [list "Open" "Closed"]
    addParam $w $a Length
    addParam $w $a Order
    addMenu $w $a ParamType [list "Chordal" "Centripetal" "Uniform"]
    addMenu $w $a Derivatives [list "Automatic" "Manual"]
    if { $ICurveAttrData(Derivatives) == 0 } {
	addParam $w $a SDLen
	addParam $w $a EDLen
    }

    addVSpace $w s2 4
    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(ncdisplaymodes)

    addVSpace $w s3 4
    addText $w $a "Created NURBS Curve:"
    addInfo $w $a NCInfo

    plb_setwin $w $oldfocus

 return;
}
# icurve_getAttr

proc init_ICurve { } {
    global ay ICurve_props ICurveAttr ICurveAttrData

    set ICurve_props { Transformations Attributes Tags ICurveAttr }

    array set ICurveAttr {
	arr   ICurveAttrData
	sproc ""
	gproc icurve_getAttr
	w     fICurveAttr
    }

    array set ICurveAttrData {
	Type 0
	DisplayMode 0
	ParamType 0
	Derivatives 0
	NCInfoBall "N/A"
    }

 return;
}
# init_ICurve
