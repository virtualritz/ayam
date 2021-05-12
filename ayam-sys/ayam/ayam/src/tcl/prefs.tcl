# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# prefs.tcl - managing preferences


# prefs_set:
#  transfer preference settings to C-context
proc prefs_set {} {
    global env ay ayprefs tcl_precision

    set tcl_precision $ayprefs(TclPrecision)
    set env(SHADERS) [shader_unglobShaderPaths $ayprefs(Shaders)]
    setPrefs

    if { $ayprefs(EnvFile) != "" } {
	set ayprefs(LoadEnv) 1
    }

 return;
}
# prefs_set


# prefs_rsnb:
#  resize notebook nb so that the page <page> is displayed in full size
proc prefs_rsnb { nb page } {
    global ay ayprefs tcl_platform

    update

    if { ($ayprefs(SaveDialogGeom) != 0) && ($ay(prefsgeom) != "") } {
	set owidth [string range $ay(prefsgeom) 0\
		[string first x $ay(prefsgeom)]]
	set owidth [string trimright $owidth x]
    }

    set oldwidth [expr [winfo width $nb] - 4]
    if { $oldwidth < 50 } { set oldwidth 340 }

    wm geometry .prefsw {}
    $nb configure -height [winfo reqheight [$nb getframe $page]]

    if { ($ayprefs(SaveDialogGeom) != 0) && ($ay(prefsgeom) != "") && \
	    [info exists owidth] } {
	update
	set ng [winGetGeom .prefsw]
	set ng ${owidth}[string range $ng [string first x $ng] end]
	if { ($tcl_platform(platform) != "windows") &&
	     ($ayprefs(TwmCompat) == 0) } {
	    set x [winfo rootx .prefsw]
	    set y [winfo rooty .prefsw]
	} else {
	    set ng [wm geom .prefsw]
	    regexp {([0-9]+)?x?([0-9]+)?(\+)?([0-9\-]+)?(\+)?([0-9\-]+)?} \
		$ng blurb nw nh blurb2 x blurb3 y
	}
	set ng ""
	if { $x >= 0 } { append ng "+$x" } else { append ng "-$x" }
	if { $y >= 0 } { append ng "+$y" } else { append ng "-$y" }
	$nb configure -width $oldwidth
	winMoveOrResize .prefsw $ng
    }

    set ay(prefssection) $page

 return;
}
# prefs_rsnb


# prefs_open:
#  open the preferences editor
proc prefs_open {} {
    global ay ayprefs ayprefse ayprefss tcl_platform aymainshortcuts

    # copy array ayprefs to ayprefse (we operate on this array)
    # and to ayprefss (for potential reversal operation)
    foreach avn [array names ayprefs] {
	set ayprefse($avn) $ayprefs($avn)
	set ayprefss($avn) $ayprefs($avn)
    }

    winAutoFocusOff

    set ay(prefsFocus) [focus]

    set w .prefsw
    catch {destroy $w}

    set width 370
    #if { $tcl_platform(platform) == "windows" } {
    #set width 400
    #}
    toplevel $w -class Ayam -width $width -height 400
    wm withdraw $w
    if { $ayprefs(SingleWindow) } {
	if { $ay(ws) == "Aqua" } {
	    winMakeFloat $w
	} else {
	    wm transient $w .
	}
    }
    wm title $w "Ayam Preferences"
    wm iconname $w "Prefs"

    # center window (if no saved geometry is available)
    update idletasks
    if { ($ayprefs(SaveDialogGeom) == 0) || ($ay(prefsgeom) == "") } {
	winCenter $w
    }

    # bind to close button of window decoration
    wm protocol $w WM_DELETE_WINDOW {
	global ay ayprefs ayprefse
	prefs_unset
	restoreFocus $ay(prefsFocus)
	destroy .prefsw
	winAutoFocusOn
    }

    # Tabbed-Notebook
    set f [frame $w.f2 -relief sunken -bd 2]
    pack $f -in $w -fill both -expand yes -pady 2
    set nb [NoteBook $w.f2.nb -height 0]

    set ay(bca) .prefsw.f3.bca
    set ay(bok) .prefsw.f3.bap

    # PrefsGUIs

    # Main
    set fw [$nb insert end Main -text Main -raisecmd "prefs_rsnb $nb Main"]

    addText $fw e0 "Shaders:"
    set s [string map [list <sep> $ay(separator)] [ms info_separator]]
    set h [ms ayprefse_Shaders]
    append h $s
    addMDirB $fw ayprefse Shaders $h
    addCommandB $fw c1 "Scan Shaders" {
	set ayprefs(Shaders) $ayprefse(Shaders)
	update
	shader_scanAll
    } [ms ayprefse_ScanShaders]
    addText $fw e1 "GUI:"
    set l $ay(locales)
    addStringB $fw ayprefse Locale [ms ayprefse_Locale] $l
    addCheckB $fw ayprefse SingleWindow [ms ayprefse_SingleWindow]
    if { $ayprefs(SingleWindow) == 0 } {
	addCheckB $fw ayprefse AutoResize [ms ayprefse_AutoResize]
    }
    addCheckB $fw ayprefse AutoFocus [ms ayprefse_AutoFocus]
    if { $ay(ws) == "X11" } {
	addCheckB $fw ayprefse TwmCompat [ms ayprefse_TwmCompat]
    }
    #addCheckB $fw ayprefse ListTypes [ms ayprefse_ListTypes]
    #addCheckB $fw ayprefse MarkHidden [ms ayprefse_MarkHidden]
    addCheckB $fw ayprefse AutoSavePrefs [ms ayprefse_AutoSavePrefs]
    addCheckB $fw ayprefse BakOnReplace [ms ayprefse_BakOnReplace]
    addCheckB $fw ayprefse AddExtensions [ms ayprefse_AddExtensions]
    #addCheckB $fw ayprefse LoadEnv [ms ayprefse_LoadEnv]
    #addCheckB $fw ayprefse NewLoadsEnv [ms ayprefse_NewLoadsEnv]
    addFileTB $fw ayprefse EnvFile {{"Ayam Scene" ".ay"}} [ms ayprefse_EnvFile]
    set h [ms ayprefse_Scripts]
    append h $s
    addMFileB $fw ayprefse Scripts $h
    set h [ms ayprefse_Plugins]
    append h $s
    addMDirB $fw ayprefse Plugins $h

    set docdefs {"http://ayam.sourceforge.net/docs/"}
    global AYWITHAQUA
    if { $AYWITHAQUA == 1 } {
	set t [file join [file dirname [info nameofexecutable]] \
		   ../Resources/docs/ayam.html]
	lappend docdefs "file://$t"
    } else {
	set t [file join [file dirname [info nameofexecutable]] \
		   ../doc/html/ayam.html]
	lappend docdefs "file://$t"
    }
    addStringB $fw ayprefse Docs [ms ayprefse_Docs] $docdefs

    set tmpdefs "/tmp"
    if { $tcl_platform(platform) == "windows" } {
	global env
	set tmpdefs "$env(TEMP)"
	regsub -all {\\} $tmpdefs {/} tmpdefs
    }
    addStringB $fw ayprefse TmpDir [ms ayprefse_TmpDir] $tmpdefs

    # Modeling
    set fw [$nb insert end Modeling -text Modeling\
	    -raisecmd "prefs_rsnb $nb Modeling"]

    addParamB $fw ayprefse PickEpsilon [ms ayprefse_PickEpsilon]\
	{ 0.01 0.05 0.1 0.2 }
    addParamB $fw ayprefse HandleSize [ms ayprefse_HandleSize] { 4 5 6 8 }
    addCheckB $fw ayprefse LazyNotify [ms ayprefse_LazyNotify]
    addMenuB $fw ayprefse CompleteNotify [ms ayprefse_CompleteNotify]\
	{"Never" "Always" "Lazy"}
    addCheckB $fw ayprefse EditSnaps [ms ayprefse_EditSnaps]
    addCheckB $fw ayprefse Snap3D [ms ayprefse_Snap3D]
    addCheckB $fw ayprefse FlashPoints [ms ayprefse_FlashPoints]
    addMenuB $fw ayprefse RationalPoints [ms ayprefse_RationalPoints]\
	{"Euclidean" "Homogeneous"}
    addCheckB $fw ayprefse GlobalMark [ms ayprefse_GlobalMark]
    addCheckB $fw ayprefse CreateAtMark [ms ayprefse_CreateAtMark]

    set l $ay(defactions)
    addMenuB $fw ayprefse DefaultAction [ms ayprefse_DefaultAction] $l
    addCheckB $fw ayprefse PickCycle [ms ayprefse_PickCycle]
    addParamB $fw ayprefse UndoLevels [ms ayprefse_UndoLevels] { 0 1 10 20 }

    # Drawing
    set fw [$nb insert end Drawing -text Drawing\
	    -raisecmd "prefs_rsnb $nb Drawing"]

    addParamB $fw ayprefse Tolerance [ms ayprefse_Tolerance]\
	    { 5 10 25 50 75 90 }
    set l [lrange $ay(npdisplaymodes) 1 end]
    addMenuB $fw ayprefse NPDisplayMode [ms ayprefse_NPDisplayMode] $l
    set l [lrange $ay(ncdisplaymodes) 1 end]
    addMenuB $fw ayprefse NCDisplayMode [ms ayprefse_NCDisplayMode] $l

    addParamB $fw ayprefse ToleranceA [ms ayprefse_ToleranceA]\
	    { -3 -2 -1.5 0 50 75 90 }
    set l "NPDisplayMode"
    foreach m [lrange $ay(npdisplaymodes) 1 end] { lappend l $m }
    addMenuB $fw ayprefse NPDisplayModeA [ms ayprefse_NPDisplayModeA] $l
    set l "NCDisplayMode"
    foreach m [lrange $ay(ncdisplaymodes) 1 end] { lappend l $m }
    addMenuB $fw ayprefse NCDisplayModeA [ms ayprefse_NCDisplayModeA] $l

    addCheckB $fw ayprefse UseMatColor [ms ayprefse_UseMatColor]
    addColorB $fw ayprefse Background [ms ayprefse_Background]
    addColorB $fw ayprefse Object [ms ayprefse_Object]
    addColorB $fw ayprefse Selection [ms ayprefse_Selection]
    addColorB $fw ayprefse Grid [ms ayprefse_Grid]
    addColorB $fw ayprefse Tag [ms ayprefse_Tag]
    addColorB $fw ayprefse Shade [ms ayprefse_Shade]
    addColorB $fw ayprefse Light [ms ayprefse_Light]

    uie_setLabelWidth $fw 15

    # RIB Export
    set fw [$nb insert end RIB-Export -text RIB-Export\
	    -raisecmd "prefs_rsnb $nb RIB-Export"]

    prefs_addRIBExport $nb $fw

    # Misc
    set fw [$nb insert end Misc -text Misc -raisecmd "prefs_rsnb $nb Misc"]

    addText $fw e0 "Errors:"
    addCheckB $fw ayprefse RedirectTcl [ms ayprefse_RedirectTcl]
    addCheckB $fw ayprefse Logging [ms ayprefse_Logging]
    addSFileB $fw ayprefse LogFile [ms ayprefse_LogFile]
    addMenuB $fw ayprefse ErrorLevel [ms ayprefse_ErrorLevel]\
	    {Silence Errors Warnings All}
    addText $fw e2 "UI:"
    addCheckB $fw ayprefse SaveAddsMRU [ms ayprefse_SaveAddsMRU]
    addCheckB $fw ayprefse ImportSetsCD [ms ayprefse_ImportSetsCD]
    addCheckB $fw ayprefse ExportSetsCD [ms ayprefse_ExportSetsCD]
    addCommandB $fw ayprefse SetActionMenu {
	vmenu_configureactionmenu
	set ayprefse(VMenuList) $ayprefs(VMenuList)
    } [ms ayprefse_SetActionMenu]

    if { $ayprefs(SingleWindow) == 0 } {
	addCheckB $fw ayprefse ToolBoxTrans [ms ayprefse_ToolBoxTrans]
	addCheckB $fw ayprefse ToolBoxShrink [ms ayprefse_ToolBoxShrink]
    }

    #addCheckB $fw ayprefse Transient [ms ayprefse_Transient]
    addParamB $fw ayprefse TclPrecision [ms ayprefse_TclPrecision]\
	    { 4 5 6 12 17 }
    addMenuB $fw ayprefse SaveDialogGeom [ms ayprefse_SaveDialogGeom]\
	    {Never WhileRunning Always}
    addText $fw e3 "Tesselation:"
    addMenuB $fw ayprefse SMethod [ms ayprefse_SMethod]\
	    $ay(smethods)
    addParamB $fw ayprefse SParamU [ms ayprefse_SParam] { 10 0.5 1 30 }
    addParamB $fw ayprefse SParamV [ms ayprefse_SParam] { 10 0.5 1 30 }

    uie_setLabelWidth $fw 16

    # end of PrefsGUIs

    pack $nb -fill both -expand yes

    # controlling buttons
    set f [frame $w.f3]
    button $f.bok -text "Ok" -width 8 -command {
	global ay
	prefs_warnNeedRestart 0
	prefs_commit
	prefs_unset
	prefs_set
	rV
	restoreFocus $ay(prefsFocus)
	destroy .prefsw
	prefs_warnNeedRestart 1
	winAutoFocusOn
    }

    button $f.bap -text "Apply" -width 8 -command {
	prefs_warnNeedRestart 0
	prefs_commit
	prefs_warnNeedRestart 1
	prefs_set
	rV
    }

    button $f.bdef -text "Revert" -width 8 -command {
	prefs_revert
	prefs_set
	rV
    }

    button $f.bca -text "Cancel" -width 8 -command {
	global ay
	prefs_unset
	restoreFocus $ay(prefsFocus)
	destroy .prefsw
	winAutoFocusOn
    }

    pack $f.bok $f.bap $f.bdef $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x -expand no

    bind $w <Enter> {
	global ayprefs
	if { $ayprefs(AutoFocus) == 1 } {
	    if { "%W" == "[winfo toplevel %W]" } {
		focus %W
	    }
	}
    }

    # show the window
    wm deiconify $w
    update
    # show last selected section
    $nb raise $ay(prefssection)
    update
    $nb see $ay(prefssection)
    # resize notebook so that section is visible
    prefs_rsnb $nb $ay(prefssection)

    # establish "Help"-binding
    global aymainshortcuts
    bind $w <[repctrl $aymainshortcuts(Help)]> {
      openUrl [concatUrls \
	 ${ayprefs(Docs)} ayam-2.html\#pref[string tolower $ay(prefssection)]]
    }

    # establish "Zap"-binding
    bind $w <[repctrl $aymainshortcuts(Zap)]> zap
    bind $w <Map> unzap

    # Esc-key == Cancel button
    bind $w <Escape> "$f.bca invoke"

    # bind function keys
    shortcut_fkeys $w

    # bind Control-Tab to switch the notebook to the next page
    bind $w <Control-Tab> "prefs_nextpage $nb 1;break"
    bind $w <Shift-Control-Tab> "prefs_nextpage $nb 0;break"
    catch {bind $w <Control-ISO_Left_Tab> "prefs_nextpage $nb 0;break"}

    if { ($ayprefs(SaveDialogGeom) > 0) && ($ay(prefsgeom) != "") } {
	winMoveOrResize $w $ay(prefsgeom)
    }
    if { $ayprefs(SaveDialogGeom) > 0 } {
	bind $w <Configure> "\
	 if { \"%W\" == \"$w\" } { set ay(prefsgeom) \[winGetGeom $w\] } "
    }

    focus $f.bok
    tkwait window $w

 return;
}
# prefs_open


proc prefs_layoutRIBExport { nb n1 n2 op } {
    global ayprefse

    if { [winfo exists $nb] && [$nb raise] == "RIB-Export" } {
	set fw  [$nb getframe RIB-Export]

	foreach w [winfo children $fw] {
	    destroy $w
	}

	prefs_addRIBExport $nb $fw

	prefs_rsnb $nb "RIB-Export"
    }

 return;
}


proc prefs_addRIBExport { nb fw } {
    global ay ayprefse

    addFileB $fw ayprefse RIBFile [ms ayprefse_RIBFile]\
	    [list Scenefile Scene Ask]
    addFileB $fw ayprefse Image [ms ayprefse_Image] [list RIB Ask]
    addCheckB $fw ayprefse ResInstances [ms ayprefse_ResInstances]
    addCheckB $fw ayprefse CheckLights [ms ayprefse_CheckLights]
    addMenuB $fw ayprefse DefaultMat [ms ayprefse_DefaultMat]\
	    [list none matte default]
    addCheckB $fw ayprefse RIStandard [ms ayprefse_RIStandard]
    addCheckB $fw ayprefse WriteIdent [ms ayprefse_WriteIdent]
    addCheckB $fw ayprefse ExcludeHidden [ms ayprefse_ExcludeHidden]

    addText $fw e0 "Rendering:"

    set f [frame $fw.fr -bd 1 -relief sunken -padx 2 -pady 3]
    pack $f -in $fw -side top -expand yes -fill x

    addMenuB $f ayprefse SetRenderer [ms ayprefse_SetRenderer]\
	    [list Preview QuickPreview File ShadowMap All]

    trace add variable ayprefse(SetRenderer) write "prefs_layoutRIBExport $nb"

    bind $f.fSetRenderer <Destroy>\
    "trace remove variable ayprefse(SetRenderer) write \"prefs_layoutRIBExport $nb\""

    addVSpace $f s1 3

    if { $ayprefse(SetRenderer) == 0 || $ayprefse(SetRenderer) == 4 } {
	addMenuB $f ayprefse RenderMode [ms ayprefse_RenderMode]\
	    [list CommandLineArg RiDisplay Viewport]
	addStringB $f ayprefse Render [ms ayprefse_Render]\
      [list "rendrib -d 4 -Progress %s" "rendrib -d 4 %s" "aqsis -fb %s" "..."]
	addCheckB $f ayprefse RenderUI [ms ayprefse_RenderUI]
	addStringB $f ayprefse RenderPT [ms ayprefse_RenderPT]\
	    [list "R90000 %d" "Done computing %d" "%d"]
	addStringB $f ayprefse DisplayDriver [ms ayprefse_DisplayDriver]\
	    [list "fifodspy" "pixiefifodspy"]
    }

    if { $ayprefse(SetRenderer) == 1 || $ayprefse(SetRenderer) == 4 } {
	addMenuB $f ayprefse QRenderMode [ms ayprefse_QRenderMode]\
	    [list CommandLineArg RiDisplay Viewport]
	addStringB $f ayprefse QRender [ms ayprefse_QRender]\
	    [list "rgl %s" "rgl -rd 10 %s" "..."]
	addCheckB $f ayprefse QRenderUI [ms ayprefse_QRenderUI]
	addStringB $f ayprefse QRenderPT [ms ayprefse_QRenderPT]\
	    [list "R90000 %d" "Done computing %d" "%d"]
	addStringB $f ayprefse QDisplayDriver [ms ayprefse_QDisplayDriver]\
	    [list "fifodspy" "pixiefifodspy"]
    }

    if { $ayprefse(SetRenderer) == 2 || $ayprefse(SetRenderer) == 4 } {
	addStringB $f ayprefse FRender [ms ayprefse_FRender]\
	    [list "rendrib -Progress %s" "rendrib %s" "aqsis %s" "..."]
	addCheckB $f ayprefse FRenderUI [ms ayprefse_FRenderUI]
	addStringB $f ayprefse FRenderPT [ms ayprefse_FRenderPT]\
	    [list "R90000 %d" "Done computing %d" "%d"]
	addStringB $f ayprefse FDisplay [ms ayprefse_FDisplay]\
	    [list "image.exr,exr_dspy,rgba"]
    }

    if { $ayprefse(SetRenderer) == 3 || $ayprefse(SetRenderer) == 4 } {
	addStringB $f ayprefse SMRender [ms ayprefse_SMRender]\
	    [list "rendrib -Progress %s" "rendrib %s" "aqsis %s" "..."]
	addCheckB $f ayprefse SMRenderUI [ms ayprefse_SMRenderUI]
	addStringB $f ayprefse SMRenderPT [ms ayprefse_SMRenderPT]\
	    [list "R90000 %d" "Done computing %d" "%d"]
    }

    addVSpace $fw s2 3

    addCheckB $fw ayprefse AutoCloseUI [ms ayprefse_AutoCloseUI]

    addText $fw e1 "ShadowMaps:"
    addMenuB $fw ayprefse ShadowMaps [ms ayprefse_ShadowMaps]\
	[list Never Automatic Manual]

    trace add variable ayprefse(ShadowMaps) write "prefs_layoutRIBExport $nb"

    bind $fw.fShadowMaps <Destroy>\
    "trace remove variable ayprefse(ShadowMaps) write \"prefs_layoutRIBExport $nb\""

    if { $ayprefse(ShadowMaps) != 0 } {
	addStringB $fw ayprefse SMFileFormat [ms ayprefse_SMFileFormat]\
	    [list "zfile" "shadow"]
	addStringB $fw ayprefse SMFileType [ms ayprefse_SMFileType]\
	    [list "z" "avgz" "volshadow"]
	addCheckB $fw ayprefse SMChangeShaders [ms ayprefse_SMChangeShaders]
    }

    uie_setLabelWidth $fw 16

    global AYENABLEPPREV
    if { $AYENABLEPPREV == 1 } {
	addStringB $fw ayprefse PPRender [ms ayprefse_PPRender] [list "rgl"]
    }

 return;
}
# prefs_addRIBExport


# prefs_save:
#  save preference settings to ayamrc file
proc prefs_save { } {
    global ay ayprefs

    set ayrc $ay(ayamrc)

    if { [file exists $ayrc] } {
	if { ! [file writable $ayrc] } {
	    ayError 1 "prefs_save" "\"$ayrc\" is not writable! No preferences saved!"
	    return;
	}
	set err [ catch {
	    file copy -force -- $ayrc ${ayrc}${ayprefs(BackupExt)}
	} ]
	update
	set newfile [open $ayrc w]
    } else {
	if { $ayrc == "" } { set ayrc "~/.ayamrc" }
	set newfile [open $ayrc w]
    }

    global aygeom aymainshortcuts ayviewshortcuts riattr riopt

    # get main window geometry
    set ayprefs(mainGeom) [winGetGeom .]

    # do not store geometry if window mode changed in this session
    if { $ayprefs(SingleWindow) } {
	if { $ayprefs(LastWindowMode) != "Single" } {
	    set ayprefs(mainGeom) ""
	}
    } else {
	if { $ayprefs(LastWindowMode) != "Floating" } {
	    set ayprefs(mainGeom) ""
	}
    }

    # get main window state
    set ayprefs(mainState) [wm state .]

    # get toolbox window geometry
    set ayprefs(toolBoxGeom) ""
    catch { set ayprefs(toolBoxGeom) [winGetGeom .tbw] }

    # get preferences window geometry
    if { $ayprefs(SaveDialogGeom) > 1 } {
	if { $ay(prefsgeom) != "" } {
	    set ayprefs(PrefsGeom) $ay(prefsgeom)
	}
	# also remember the currently open preferences section
	set ayprefs(PrefsSection) $ay(prefssection)
    } else {
	set ayprefs(PrefsGeom) ""
	# reset the currently open preferences section
	set ayprefs(PrefsSection) "Main"
    }

    # get pane configuration
    if { $ayprefs(SavePanes) == 1 } {
	# save config for single window GUI mode only
	if { [winfo exists .__h2] } {
	    set ayprefs(PaneConfig) ""
	    set ayprefs(PaneConfig) [winGetPaneLayout]
	}
    }

    # remember from which window system we write this ayamrc
    set ayprefs(WindowSystem) $ay(ws)

    # write header
    puts $newfile "# Emacs, this is -*- Mode: Tcl -*-\n"
    puts $newfile "# These are the saved preference settings for Ayam, a free"
    puts $newfile "# 3D modeling environment for the RenderMan interface."
    puts $newfile "# See: http://ayam.sourceforge.net/\n"
    puts $newfile "# Edit, if you wish, but keep in mind:"
    puts $newfile "# _This file is parsed by Tcl!_\n"

    puts $newfile "# Preferences:"

    # write preferences
    set names [lsort [array names ayprefs]]
    foreach pref $names {
	eval [subst "set val {{\$ayprefs($pref)}}"]
	puts $newfile "set ayprefs($pref) $val"
    }

    # now write the hotkeys/shortcuts
    puts $newfile "\n# Hotkeys:"

    # hotkeys for main window
    puts $newfile "# main window:"
    set keys [lsort [array names aymainshortcuts]]
    foreach key $keys {
	eval [subst "set val {{\$aymainshortcuts($key)}}"]
	puts $newfile "set aymainshortcuts($key) $val"
    }

    # hotkeys for view windows
    puts $newfile "# view windows:"
    set keys [lsort [array names ayviewshortcuts]]
    foreach key $keys {
	eval [subst "set val {{\$ayviewshortcuts($key)}}"]
	puts $newfile "set ayviewshortcuts($key) $val"
    }

    # dialog window geometries
    if { $ayprefs(SaveDialogGeom) > 1 } {
	puts $newfile "\n# dialog geometry:"
	foreach key [array names aygeom] {
	    eval [subst "set val {{\$aygeom($key)}}"]
	    puts $newfile "set aygeom($key) $val"
	}
    }

    # now write RiOption and RiAttribute tag database
    # RiOptions
    puts $newfile "\n# RiOptions:"
    foreach key [array names riopt] {
	eval [subst "set val {{\$riopt($key)}}"]
	puts $newfile "set riopt($key) $val"
    }

    # RiAttributes
    puts $newfile "\n# RiAttributes:"
    foreach key [array names riattr] {
	eval [subst "set val {{\$riattr($key)}}"]
	puts $newfile "set riattr($key) $val"
    }

    # write footer
    puts $newfile "return;"

    close $newfile
    update
    ayError 4 "prefs_save" "Done saving preferences to \"$ayrc\"."

    update

 return;
}
# prefs_save


# prefs_toggleLazyNotification:
#
proc prefs_toggleLazyNotification { } {
    global ayprefs ayprefse aymainshortcuts

    if { $ayprefs(LazyNotify) == 1 } {
	set ayprefse(LazyNotify) 0
	set ayprefs(LazyNotify) 0
	ayError 4 $aymainshortcuts(SwLazyNotify) "LazyNotification turned off."
    } else {
	set ayprefse(LazyNotify) 1
	set ayprefs(LazyNotify) 1
	ayError 4 $aymainshortcuts(SwLazyNotify) "LazyNotification turned on."
    }

    setPrefs

 return;
}
# prefs_toggleLazyNotification


# prefs_toggleSurfaceWire:
#  toggle drawing of Curves/Surfaces vs. Wireframes (bound to F-Key)
proc prefs_toggleSurfaceWire { } {
    global ay ayprefs ayprefse aymainshortcuts

    if { ![info exists ay(draw_nurbs)] } {
	if { $ayprefs(NPDisplayMode) == 0 && $ayprefs(NCDisplayMode) == 0 } {
	   set ay(draw_nurbs) 0
	} else {
	    set ay(draw_nurbs) 1
	}
    }

    if { $ay(draw_nurbs) == 0 } {
	if { $ayprefs(NPDisplayMode) == 0 || $ayprefs(NCDisplayMode) == 0 } {
	    if { [info exists ay(savedNPDisplayMode)] } {
		set ayprefse(NPDisplayMode) $ay(savedNPDisplayMode)
	    } else {
		set ayprefse(NPDisplayMode) 2
	    }
	    if { [info exists ay(savedNCDisplayMode)] } {
		set ayprefse(NCDisplayMode) $ay(savedNCDisplayMode)
	    } else {
		set ayprefse(NCDisplayMode) 2
	    }
	    set ayprefs(NPDisplayMode) $ayprefse(NPDisplayMode)
	    set ayprefs(NCDisplayMode) $ayprefse(NCDisplayMode)
	    ayError 4 $aymainshortcuts(SwNURBSWire)\
		    "Drawing of Curves/Surfaces turned on."
	    setPrefs
	    set ay(draw_nurbs) 1
	    update
	    rV
	}
    } else {
	if { $ayprefs(NPDisplayMode) != 0 || $ayprefs(NCDisplayMode) != 0 } {
	    set ay(savedNPDisplayMode) $ayprefs(NPDisplayMode)
	    set ay(savedNCDisplayMode) $ayprefs(NCDisplayMode)
	    set ayprefse(NPDisplayMode) 0
	    set ayprefs(NPDisplayMode) 0
	    set ayprefse(NCDisplayMode) 0
	    set ayprefs(NCDisplayMode) 0
	    ayError 4 $aymainshortcuts(SwNURBSWire)\
		    "Drawing of Wireframes turned on."
	    setPrefs
	    set ay(draw_nurbs) 0
	    update
	    rV
	}
    }
    # if

 return;
}
# prefs_toggleSurfaceWire


# prefs_setSamplingTolerance:
#  set new sampling tolerance (bound to F-Key)
proc prefs_setSamplingTolerance { plus } {
    global ay ayprefs ayprefse aymainshortcuts

    if { $ay(sstlock) == 0 } {
	set ay(sstlock) 1
	update
	if { $plus == 1 } {
	    if { $ayprefs(Tolerance) < 90 } {
		set newval [ expr $ayprefs(Tolerance) + 10]
		set ayprefs(Tolerance) $newval
		set ayprefse(Tolerance) $newval
		ayError 4 $aymainshortcuts(SetSTP)\
			"SamplingTolerance set to ${newval}."
		setPrefs
		update
		rV
	    }
	} else {
	    if { $ayprefs(Tolerance) > 10 } {
		set newval [ expr $ayprefs(Tolerance) - 10]
		set ayprefs(Tolerance) $newval
		set ayprefse(Tolerance) $newval
		ayError 4 $aymainshortcuts(SetSTL)\
			"SamplingTolerance set to ${newval}."
		setPrefs
		update
		rV
	    }
	}
	# if
	set ay(sstlock) 0
    }
    # if

 return;
}
# prefs_setSamplingTolerance


# prefs_warnNeedRestart:
#  warn user that restart is needed for his preference changes
#  to take (full) effect
proc prefs_warnNeedRestart { warn } {
    global env ay ayprefs ayprefse

    if { !$warn } {
	if { ($ayprefs(Locale) != $ayprefse(Locale)) ||
	     ($ayprefs(SingleWindow) != $ayprefse(SingleWindow)) ||
	     ($ayprefs(Scripts) != $ayprefse(Scripts)) ||
	     ($ayprefs(ToolBoxTrans) != $ayprefse(ToolBoxTrans)) ||
	     ($ayprefs(RGTrans) != $ayprefse(RGTrans))
	 } {
	    set ay(warnrestart) 1
	} else {
	    set ay(warnrestart) 0
	}
    } else {
	if { $ay(warnrestart) } {
	    set t [ms info_rs1]
	    set m [ms info_rs2]

	    if { $ayprefs(FixDialogTitles) == 1 } {
		set m "$t\n\n$m"
	    }

	    set p "."

	    if { [winfo exists .prefsw] } {
		set p ".prefsw"
	    }

	    set answer [tk_messageBox -parent $p -title $t -type ok\
			    -icon warning -message $m]
	}
	# if
    }
    # if

 return;
}
# prefs_warnNeedRestart


# prefs_reset:
#  remove current preferences file and set preferences so that
#  no new preferences file will be created when exiting
proc prefs_reset {} {
    global ay ayprefs

    # only do something when preferences file really exists
    if { [file exists $ay(ayamrc)] } {
	set t [subst [ms info_rp1]]
	set m [subst [ms info_rp2]]

	if { $ayprefs(FixDialogTitles) == 1 } {
	    set m "$t\n\n$m"
	}

	set answer [tk_messageBox -title $t -type okcancel -icon warning\
			-message $m]

	if { $answer == "ok" } {
	    if [catch {file delete $ay(ayamrc)} errmsg] {
		ayError 1 prefs_reset $errmsg
	    } else {
		set ayprefs(AutoSavePrefs) 0
		set m [ms info_rp3]
		tk_messageBox -title "Info" -type ok -icon info -message $m
	    }
	}
	# if answer ok
    }
    # if have ayamrc

 return;
}
# prefs_reset


# prefs_nextpage:
#  show next/prev page (bound to <Control-Tab>/<Shift-Control-Tab>)
proc prefs_nextpage { nb dir } {
    global ay ayprefs

    set allpages [$nb pages]
    set allpageslen [llength $allpages]

    set index [$nb index $ay(prefssection)]

    if { $dir == 1 } {
	if { $index == [expr {$allpageslen - 1}] } {
	    set index 0
	} else {
	    incr index
	}
    } else {
	if { $index == 0 } {
	    set index [expr {$allpageslen - 1}]
	} else {
	    set index [expr {$index - 1}]
	}
    }
    set page [lindex $allpages $index]
    set ay(prefssection) $page
    $nb raise $page
    $nb see $page
    prefs_rsnb $nb $ay(prefssection)
    #focus -force [winfo toplevel $nb]
    focus -force $nb

 return;
}
# prefs_nextpage


# prefs_unset:
#  helper to remove all traces by calling unset on all entries in ayprefse
proc prefs_unset { } {
    global ayprefs ayprefse
    foreach avn [array names ayprefs] {
	unset ayprefse($avn)
    }
 return;
}
# prefs_unset


# prefs_commit:
#  helper to commit changes (copy array ayprefse to ayprefs)
proc prefs_commit { } {
    global ay ayprefs ayprefse
    foreach avn [array names ayprefs] {
	set ayprefs($avn) $ayprefse($avn)
    }
    set ay(oldAutoFocus) $ayprefse(AutoFocus)
 return;
}
# prefs_commit


# prefs_revert:
#  helper to undo all current changes
proc prefs_revert { } {
    global ay ayprefse ayprefss
    foreach avn [array names ayprefss] {
	set ayprefse($avn) $ayprefss($avn)
    }
    # update color entries
    update
    set w .prefsw.f2.nb.fDrawing
    foreach c {Background Object Selection Grid Tag Shade Light} {
	updateColorFromE $w ayprefse $c $w.f${c}.b1
    }
    set ay(oldAutoFocus) $ayprefse(AutoFocus)
 return;
}
# prefs_revert
