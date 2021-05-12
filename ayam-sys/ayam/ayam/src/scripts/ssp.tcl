#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2015 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# ssp.tcl - save selected points in SP tags

# vars:
set imgdata {\
R0lGODdhGQAZAMIAALy8vL28vLy9vAAAADMzM68AAAAAAAAAACwAAAAAGQAZAAADVgi63BxiRUfd
qJjezMH4XfZtoQaWFYlaq6l6Y/wuYzN76Ulr2HzJuJ2QQSoyCJuC0jb8AZUFpsK4QCqg0qB26+Ol
WGCLbiqruVq2MXq73k7aigBADk8AADs=}
image create photo ay_RSP_img -format GIF -data $imgdata

set imgdata {\
R0lGODdhGQAZAMIAALy8vL28vLy9vAAAAK8AADMzMwAAAAAAACwAAAAAGQAZAAADWAi63BxixTWc
bZPe7YbnIOBV4TaWHIle6tqc7vtZYz1T90t35q4zqopN1CAYGQVSUMMwEkTDpaIFcCKVTOIvK9UC
ezxg7hvGUc02KixGZrdjGbciAKDLAQkAOw==}
image create photo ay_SSP_img -format GIF -data $imgdata

# procs:

# ssp_save:
#  save selected points to SP tag
proc ssp_save { } {
    set fw [focus]
    forAll {
	selPnts -get pnts
	if { $pnts != "" } {
	    if { [hasTag sp] } {
		delTags SP
	    }
	    addTag SP $pnts
	}
    }
    restoreFocus $fw
 return;
}
# ssp_save


# ssp_restore:
#  restore selected points from SP tag
proc ssp_restore { } {
    set fw [focus]
    forAll {
	getTag SP pnts
	if { $pnts != "" } {
	    eval selPnts $pnts
	}
    }
    restoreFocus $fw
 return;
}
# ssp_restore


# add to toolbox
set b $ay(tbw).ssp
if { ![winfo exists $b] } {
    button $b -padx 0 -pady 0 -takefocus 0 -image ay_SSP_img\
	-command ssp_save
    lappend ay(toolbuttons) ssp
    balloon_set $b "Save selected points to tag"
}
set b $ay(tbw).rsp
if { ![winfo exists $b] } {
    button $b -padx 0 -pady 0 -takefocus 0 -image ay_RSP_img\
	-command ssp_restore
    lappend ay(toolbuttons) rsp
    balloon_set $b "Restore selected points from tag"
}
toolbox_layout


# add to custom menu
set m $ay(cm)
$m add command -label "Save Point Selection" -command ssp_save
$m add command -label "Restore Point Selection" -command ssp_restore

# register our tag type
registerTag SP

# EOF
