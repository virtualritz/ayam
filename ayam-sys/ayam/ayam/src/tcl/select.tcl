# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# select.tcl - select objects Tcl code

proc init_Select { } {
    global ay Select_props SelectAttr SelectAttrData

    set Select_props { Transformations Attributes Material Tags SelectAttr }

    array set SelectAttr {
	arr   SelectAttrData
	sproc ""
	gproc ""
	w     fSelectAttr
    }

    array set SelectAttrData {
	Indices 0
    }

    # create SelectAttr-UI
    set w [frame $ay(pca).$SelectAttr(w)]
    set a $SelectAttr(arr)
    addVSpace $w s1 2
    addString $w $a Indices {"0,2" "end-0" "0,0,0" "0,4-end,1"}

 return;
}
# init_Select
