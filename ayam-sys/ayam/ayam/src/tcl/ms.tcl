# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2006 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# ms.tcl - msgcat "equivalent"

# ms_fixcancel:
#  for window systems where the info/warning dialogs disregard LC_LOCALE
#  we replace the english button name with the localized variant
proc ms_fixcancel {ar va} {
    global ay ms::$ar
    if { $ay(ws) == "Win32" } {
	eval "set c \$ms::${ar}(cancel)"
	eval "set s \$ms::${ar}($va)"
	set ms::${ar}($va) [string map [list Cancel $c] $s]
    }
 return;
}
# ms_fixcancel


# ms_create:
# create a translation file from english blueprint
proc ms_create { lang } {
    global ms::en ms::$lang

    set fname ${lang}.tcl
    set newfile [open $fname w]

    # write header
    puts $newfile "# Emacs, this is -*- Mode: Tcl -*-\n"
    puts $newfile "# This is a translation file for Ayam, a free"
    puts $newfile "# 3D modeling environment for the RenderMan interface."
    puts $newfile "# See: http://ayam.sourceforge.net/\n"
    puts $newfile "# Edit, if you wish, but keep in mind:"
    puts $newfile "# _This file is parsed by Tcl!_\n"

    # write translations
    puts $newfile "ms_init $lang\n"

    set map {"\n" "" "\\" "" "\"" "\\\""}
    set names [lsort [array names ms::en]]
    foreach elem $names {
	set name "ms::${lang}($elem)"
	if { [info exists $name] } {
	    eval [subst "set val {\$name}"]
	    set val [string map $map $val]
	    puts $newfile "ms_set $lang $elem \"$val\""
	} else {
	    eval [subst "set val {\$ms::en($elem)}"]
	    set val [string map $map $val]
	    puts $newfile "#ms_set $lang $elem \"$val\""
	}
    }

    # write footer
    puts $newfile "\nreturn;"

    close $newfile

    ayError 4 "ms_create" "Done creating translation file \"$fname\"."

 return;
}
# ms_create


# ms_set:
#  fill language specific text database for language <lang>;
#  the entry for GUI element <name> will get the description <val>
proc ms_set { lang name val } {

    set varname "${lang}(${name})"

    set ms::$varname $val

 return;
}
# ms_set


# ms_init:
#  initialize ms module for language <lang> by creating a dummy entry
proc ms_init { lang } {
    global ay

    array set ms::$lang { Dummy 1 }

    if { [lsearch $ay(locales) $lang] == -1 } {
	lappend ay(locales) $lang
    }

 return;
}
# ms_init


# ms_initmainlabels:
#  initialize main labels that were created before we knew
#  the current locale from ayamrc
proc ms_initmainlabels { } {
    global ay ayprefs

    catch { balloon_set $ay(plbl) [ms plb_label] [expr $ayprefs(Balloon)*3] }

    catch { balloon_set $ay(olbl) [ms olb_label] [expr $ayprefs(Balloon)*3] }

    catch { balloon_set $ay(treel) [ms tree_label] [expr $ayprefs(Balloon)*3] }

 return;
}
# ms_initmainlabels


# ms:
#  return language specific string for GUI element <name>
proc ms { name } {
    global ayprefs
    # Do we have a balloon text in the current locale?
    if { ![info exists ms::$ayprefs(Locale)($name)] } {
	# no, return english string
	if { [string first info $name] == 0 } {
	    return [subst "\$ms::en($name)"]
	}
	if { ![info exists ms::$ayprefs(Locale)(Missing)] } {
	    return [subst "Translation missing!\n\$ms::en($name)"]
	} else {
	    set str [subst "\$ms::$ayprefs(Locale)(Missing)"]
	    append str [subst "\n\$ms::en($name)"]
	    return $str
	}
    } else {
	# yes, return language specific string
	return [subst "\$ms::$ayprefs(Locale)($name)"]
    }
}
# ms

# create namespace ms
namespace eval ms {}

# fill "en"-locale...
ms_init en
ms_set en ayprefse_Shaders "A list of paths where compiled shaders reside."
ms_set en ayprefse_ScanShaders "Initiates rebuild of internal shader database."
ms_set en ayprefse_Locale "Language to use for balloon help texts.\
\nChanges will take effect after restart of Ayam!"
ms_set en ayprefse_SingleWindow "Switch to single toplevel window GUI?\
\nChanges will take effect after restart of Ayam!"
ms_set en ayprefse_AutoResize "Resize main window according to property GUI?"
ms_set en ayprefse_AutoFocus "Move focus to window with mouse cursor?\
\nChanges will take effect after restart of Ayam!"
ms_set en ayprefse_TwmCompat "Is the window manager TWM compatible?"
ms_set en ayprefse_ListTypes "Show object types in the tree/list view?"
ms_set en ayprefse_MarkHidden "Mark hidden objects in the tree view?"
ms_set en ayprefse_AutoSavePrefs "Save preferences on exit?"
ms_set en ayprefse_AddExtensions "Automatically add file name extensions?"
ms_set en ayprefse_BakOnReplace "Make backup copies of loaded scene files?"
ms_set en ayprefse_LoadEnv "Load environment on startup?"
ms_set en ayprefse_NewLoadsEnv "Load environment on File/New?"
ms_set en ayprefse_EnvFile "Path and name of the environment\
\n(created by menu \"Special/Save Environment\")."
ms_set en ayprefse_Scripts "A list of Tcls scripts to be executed on startup.\
\nChanges will take effect after restart of Ayam!"
ms_set en ayprefse_Plugins "A list of paths where plugins reside."
ms_set en ayprefse_Docs "An URL that points to the documentation."
ms_set en ayprefse_TmpDir "A path where temporary files are to be saved."

ms_set en ayprefse_PickEpsilon "Maximum allowed distance from picked point\
to editable point."
ms_set en ayprefse_HandleSize "Size of the handles of editable points in\
pixels."
ms_set en ayprefse_PickCycle "Cycle through pick candidates by further clicks?"
ms_set en ayprefse_LazyNotify "Notify parent objects about changes just on\
mouse up?"
ms_set en ayprefse_CompleteNotify "When shall a complete notification be\
carried out?"
ms_set en ayprefse_EditSnaps "Snap coordinates of edited points to grid\
coordinates?"
ms_set en ayprefse_Snap3D "Snap coordinate values in all three dimensions?"
ms_set en ayprefse_FlashPoints "Flash editable points in single point\
editing modes?"
ms_set en ayprefse_RationalPoints "Display style of rational points."
ms_set en ayprefse_GlobalMark "Maintain a single, global mark, or many local\
marks (one for each view)?"
ms_set en ayprefse_CreateAtMark "Create objects at the mark?"
ms_set en ayprefse_DefaultAction "Modelling action invoked, when <Esc> key\
is pressed."
ms_set en ayprefse_UndoLevels "Number of undoable modelling steps;\
\n0 means Undo/Redo is disabled."

# Drawing
ms_set en ayprefse_Tolerance "Sampling tolerance used when drawing\
NURBS curves or surfaces.\nSmaller values lead to slower rendering but higher\
 quality.\nNURBS objects may override this setting locally."
ms_set en ayprefse_NPDisplayMode "Determine how surfaces should be drawn\
\nSurface objects may override this setting locally."
ms_set en ayprefse_NCDisplayMode "Determine how curves should be drawn\
\nCurve objects may override this setting locally."
ms_set en ayprefse_ToleranceA "Sampling tolerance used for tesselating\
NURBS curves or surfaces\nwhen an action is active."
ms_set en ayprefse_NPDisplayModeA "Determine how surfaces should be drawn\
\nwhen an action is active."
ms_set en ayprefse_NCDisplayModeA "Determine how curves should be drawn\
\nwhen an action is active."
ms_set en ayprefse_UseMatColor "Use color of material for shaded views?"
ms_set en ayprefse_Background "Color to use for the background."
ms_set en ayprefse_Object "Color to use for unselected objects."
ms_set en ayprefse_Selection "Color to use for selected objects."
ms_set en ayprefse_Grid "Color to use for the grids."
ms_set en ayprefse_Tag "Color to use for tagged (selected) points."
ms_set en ayprefse_Shade "Color to use in shaded views when UseMatColor\
\nis not enabled or the object has no material or no material color."
ms_set en ayprefse_Light "Color to use for (unselected) light objects."

# RIB-Export
ms_set en ayprefse_RIBFile "Name of the RIB file to create on export.\
\nScene: Derive name from scene path and filename,\
\nScenefile: Derive name from scene filename (without path),\
\nAsk: Ask for filename on export."
ms_set en ayprefse_Image "Name of the image file that is created\nwhen\
the exported RIB file is rendered.\
\nRIB: Derive image filename from RIB filename."
ms_set en ayprefse_ResInstances "Resolve all instance objects to normal\
\nobjects while exporting to a RIB?"
ms_set en ayprefse_CheckLights "Add a distant headlight to the scene,\
\nif no other (active) lights exist?"
ms_set en ayprefse_DefaultMat "Write a default material statement to the\
\nRIB, that will be used by all objects without material?\
\nnone: no default material,\
\nmatte: write a simple RiMatte without parameters,\
\ndefault: write the material object named \"default\"."
ms_set en ayprefse_RIStandard "Omit all attributes and options that are not\
\ncontained in the RenderMan Interface Standard?"
ms_set en ayprefse_WriteIdent "Write an identificator derived from the\
objects name into the RIB?"
ms_set en ayprefse_ShadowMaps "Should ShadowMaps be used?\nAutomatic: Yes,\
create a RIB that automatically renders ShadowMaps all the time.\
\nManual: Yes, but the ShadowMaps will be rendered on user request only
(Menu: View/Create ShadowMaps)"
ms_set en ayprefse_ExcludeHidden "Omit hidden objects on RIB export?"
ms_set en ayprefse_RenderMode "How shall the preview renderer render to\
the screen?\n\
CommandLineArg: via command line argument (display in extra window), s.a.\
Render;\n\
RiDisplay: via RiDisplay-statement (display in extra window);\n\
Viewport: via Display-Driver-Plugin (display in Ayam), s.a. DisplayDriver."
ms_set en ayprefse_QRenderMode "How shall the quick preview renderer render to\
the screen?\n\
CommandLineArg: via command line argument (display in extra window), s.a.\
QRender;\n\
RiDisplay: via RiDisplay-statement (display in extra window);\n\
Viewport: via Display-Driver-Plugin (display in Ayam), s.a. QDisplayDriver."
ms_set en ayprefse_QRender "Command and parameters of the quick preview\
 renderer;\n\"%s\" will be replaced by the filename of the RIB."
ms_set en ayprefse_QRenderUI "Enable user interface for quick preview\
 rendering."
ms_set en ayprefse_QRenderPT "Output description of quick preview renderer;\
\n\"%d\" denotes the position of the progress number in the output."
ms_set en ayprefse_Render "Command and parameters of the preview renderer;\
\n\"%s\" will be replaced by the filename of the RIB."
ms_set en ayprefse_RenderUI "Enable user interface for preview rendering."
ms_set en ayprefse_RenderPT "Output description of preview renderer;\
\n\"%d\" denotes the position of the progress number in the output."
ms_set en ayprefse_FRender "Command and parameters of the file renderer;\
\n\"%s\" will be replaced by the filename of the RIB."
ms_set en ayprefse_FRenderUI "Enable user interface for rendering to a file."
ms_set en ayprefse_FRenderPT "Output description of the file renderer;\
\n\"%d\" denotes the position of the progress number in the output."
ms_set en ayprefse_SMRender "Command and parameters of the shadow map\
 renderer;\n\"%s\" will be replaced by the filename of the RIB."
ms_set en ayprefse_SMRenderUI "Enable user interface for shadow map rendering."
ms_set en ayprefse_SMRenderPT "Output description of the shadow map renderer;\
\n\"%d\" denotes the position of the progress number in the output."
ms_set en ayprefse_SMFileFormat "File format of shadow map.\
\nRenderMan: zfile\nGelato: shadow"
ms_set en ayprefse_SMFileType "Type of shadow map.\
\nz: normal shadow map (RenderMan, Gelato)\
\navgz: Woo shadow map (Gelato only!)\
\nshadowvol: volume shadow map (Gelato only!)"
ms_set en ayprefse_SMChangeShaders "Change shader names (prepend\
 \"shadow\")\nwhen rendering with shadow maps?"
ms_set en ayprefse_AutoCloseUI "Automatically close the user interface\
when rendering is finished?"

ms_set en ayprefse_DisplayDriver "Display driver to use for previews."
ms_set en ayprefse_QDisplayDriver "Display driver to use for quick previews."
ms_set en ayprefse_FDisplay "Display to use for rendering to files."

ms_set en ayprefse_PPRender "Renderer to use for the permanent preview feature."

ms_set en ayprefse_SetRenderer "Select renderer to edit."

# Misc
ms_set en ayprefse_RedirectTcl "Redirect all Tcl error messages to the\
console?"
ms_set en ayprefse_Logging "Log all messages to a file?"
ms_set en ayprefse_ErrorLevel "Which messages should go to the console?\
\nSilence - none\
\nErrors - only error messages\
\nWarnings - warnings and error messages\
\nAll - warnings, error messages, and informative messages"
ms_set en ayprefse_LogFile "Path and name of the file to log all messages to."
ms_set en ayprefse_SaveAddsMRU "Add the name of saved scenes to the\
\nMost-Recently-Used file menu entries?"
ms_set en ayprefse_ImportSetsCD "Shall import set the current directory?"
ms_set en ayprefse_ExportSetsCD "Shall export set the current directory?"
ms_set en ayprefse_SetActionMenu "Configure elements of the action menu!"
ms_set en ayprefse_ToolBoxTrans "Make the toolbox window transient?"
ms_set en ayprefse_ToolBoxShrink "Make the toolbox window shrink wrap around\
its contents,\nwhen the user resizes it?"
ms_set en ayprefse_RGTrans "Make all rendering user interfaces transient?"
ms_set en ayprefse_TclPrecision "Precision of Tcl mathematics.\nHigh values\
lead to exact but hard\nto edit numbers in entry fields."
ms_set en ayprefse_SaveDialogGeom "Save geometry of dialog windows for\
next invocation?"
ms_set en ayprefse_SMethod "Sampling method to be used for NURBS\nto PolyMesh\
conversions."
ms_set en ayprefse_SParam "Parameter of the sampling method for NURBS\nto\
PolyMesh conversions."

ms_set en plb_label "Double click this label\nto deselect property."
ms_set en olb_label "Double click this label\nto switch to the tree."
ms_set en tree_label "Double click this label\nto switch to the listbox."

# Mops-Import
ms_set en mopsi_options_ResetDM "Reset all DisplayMode attributes of\
imported\nobjects to \"Global\"?"
ms_set en mopsi_options_ResetST "Reset all SamplingTolerance attributes of\
imported\nobjects to \"0.0\"?"

# OBJ-Import/Export
ms_set en objio_options_Selected "Export only selected object(s)."
ms_set en objio_options_TessPoMesh "Tesselate all PolyMesh objects to\
triangles."
ms_set en objio_options_WriteCurves "Write NURBS curves to exported file?"

# info dialogs
ms_set en cancel "Cancel"
ms_set en info_sc1 "Scene changed!"
ms_set en info_sc2 "Select \"OK\" to lose all changes.\nSelect \"Cancel\" to stop operation."

ms_set en info_rs1 "Need restart!"
ms_set en info_rs2 "Changes need a restart of Ayam to take effect!"

ms_set en info_sm1 "Configuration problematic!"
ms_set en info_sm2 "Manual ShadowMaps should be enabled!\
		\nSelect \"OK\" to enable them and continue.\
		\nSelect \"Cancel\" to stop operation."

ms_set en info_warning "Warning!"
ms_set en info_pc1 "This operation may destroy the current property.\
    \nProceed with caution."

ms_set en info_rc1 "Correct Curve?"
ms_set en info_rc2 "Rotate curve to correct plane?"

ms_set en info_rp1 "Reset Preferences?"
ms_set en info_rp2 "Ready to remove file:\n\"$ay(ayamrc)\"?"
ms_set en info_rp3 "Preferences file removed!\nPlease restart Ayam now."

ms_set en info_fi1 "Renderer not responding!"
ms_set en info_fi2 "Renderer did not create the FIFO yet\
\n(DSPY not found or still rendering ShadowMaps).\
\nSelect \"OK\" to continue waiting.\nSelect \"Cancel\" to stop operation."

ms_set en info_separator "\nSeparate multiple entries by '<sep>'."

# view menu
#ms_set en vmenu1 "current modelling action"
ms_set en vmenu2 "change global/local mode\ncycle with <lmk>"
ms_set en vmenu3 "change drawing mode\ncycle with <dmu>/<dmd>"
ms_set en vmenu4 "change gridsize\ncycle with <gu>/<gd>"



#
# fill "de"-locale
ms_init de
ms_set de Missing "�bersetzung fehlt!"
ms_set de ayprefse_Shaders "Eine Liste von Verzeichnissen, in denen sich\
\n�bersetzte Shader befinden."
ms_set de ayprefse_ScanShaders "Baut interne Shader-Datenbank neu auf."
ms_set de ayprefse_SingleWindow "Soll nur ein Hauptfenster benutzt werden?\
\n�nderungen werden erst nach Neustart von Ayam wirksam!"
ms_set de ayprefse_AutoResize "Soll das Hauptfenster sich der Gr��e der\
Eigenschaften anpassen?"
ms_set de ayprefse_AutoFocus "Soll der Fokus automatisch dem Fenster mit\
dem\nMauszeiger zugeordnet werden?\n�nderungen werden erst nach\
Neustart von Ayam wirksam!"
ms_set de ayprefse_TwmCompat "Ist der verwendete Fenster-Manager\
zu TWM kompatibel?"
ms_set de ayprefse_ListTypes "Sollen die Objekttypen in der Listen- bzw.\
\nBaumansicht angezeigt werden?"
ms_set de ayprefse_MarkHidden "Sollen versteckte Objekte in der\
\nBaumansicht markiert werden?"
ms_set de ayprefse_Locale "Sprache f�r Hilfetexte.\
\n�nderungen werden erst nach Neustart von Ayam wirksam!"
ms_set de ayprefse_AutoSavePrefs "Sollen die Voreinstellungen beim Beenden\
von Ayam gespeichert werden?"
ms_set de ayprefse_AddExtensions "Sollen Dateinamenserweiterungen automatisch\
angeh�ngt werden?"
ms_set de ayprefse_BakOnReplace "Sollen Sicherheitskopien von geladenen\
Szenen angefertigt werden?"
ms_set de ayprefse_LoadEnv "Soll die Arbeitsumgebung beim Start geladen\
werden?"
ms_set de ayprefse_NewLoadsEnv "Soll die Arbeitsumgebung beim Erstellen\
einer neuen Szene neu geladen werden?"
ms_set de ayprefse_EnvFile "Vollst�ndiger Dateiname der Arbeitsumgebung\
\n(erzeugt via Men� \"Special/Save Environment\")."
ms_set de ayprefse_Scripts "Eine Liste von Skripten, die beim Starten\
ausgef�hrt werden sollen.\
\n�nderungen werden erst nach Neustart von Ayam wirksam!"
ms_set de ayprefse_Plugins "Eine Liste von Verzeichnissen, in denen sich\
Plugins befinden."
ms_set de ayprefse_Docs "Eine URL, die auf die Dokumentation verweist."
ms_set de ayprefse_TmpDir "Verzeichnis f�r tempor�re Dateien."

# Modeling
ms_set de ayprefse_PickEpsilon "Gr��te erlaubte Entfernung zwischen\
\nausgew�hltem und editierbaren Punkt."
ms_set de ayprefse_HandleSize "Gr��e der editierbaren Punkte in Pixeln."
ms_set de ayprefse_LazyNotify "Sollen Elternobjekte �ber �nderungen an den\
\nKindobjekten nur am Ende einer Modellieraktion\nbenachrichtigt werden?"
ms_set de ayprefse_CompleteNotify "Wann sollen alle Objekte �ber �nderungen\
\nan Kindobjekten (inkl. Referenzen) benachrichtigt werden?"
ms_set de ayprefse_EditSnaps "Sollen editierte Punkte zun�chst zu den\
\nGitter-Koordinaten bewegt werden?"
ms_set de ayprefse_Snap3D "Soll das Bewegen von Punkten zu Gitter-Koordinaten\
\nin allen drei Dimensionen erfolgen?"
ms_set de ayprefse_FlashPoints "Sollen editierbare Punkte aufleuchten, wenn\
der\nMauszeiger �ber ihnen steht?"
ms_set de ayprefse_RationalPoints "Anzeigetyp rationaler Punkte."
ms_set de ayprefse_GlobalMark "Soll es nur eine globale Marke geben?"
ms_set de ayprefse_CreateAtMark "Sollen neue Objekte an der Marke erzeugt\
werden?"
ms_set de ayprefse_DefaultAction "Modellier-Aktion, die beim Dr�cken der\
\n<Esc>-Taste gestartet wird."
ms_set de ayprefse_PickCycle "Mit weiteren Klicks durch Auswahlkandidaten\
wechseln?"
ms_set de ayprefse_UndoLevels "Anzahl zur�cknehmbarer Modellierschritte;\
\n0 schaltet das Undo-System aus."

# Drawing
ms_set de ayprefse_Tolerance "Darstellungsqualit�t von NURBS\
Kurven und Fl�chen.\nKleinere Werte f�hren zu h�herer Qualit�t aber\
langsamerer Darstellung.\nObjekte k�nnen diesen Wert lokal anpassen."
ms_set de ayprefse_NPDisplayMode "Darstellungsmodus von Fl�chen.\
\nFl�chen k�nnen den Darstellungsmodus lokal anpassen."
ms_set de ayprefse_NCDisplayMode "Darstellungsmodus von Kurven.\
\nKurven k�nnen den Darstellungsmodus lokal anpassen."
ms_set de ayprefse_ToleranceA "Darstellungsqualit�t von NURBS\
Kurven und Fl�chen\nw�hrend Modellieraktionen."
ms_set de ayprefse_NPDisplayModeA "Darstellungsmodus von Fl�chen\
\nw�hrend Modellieraktionen."
ms_set de ayprefse_NCDisplayModeA "Darstellungsmodus von Kurven\
\nw�hrend Modellieraktionen."

ms_set de ayprefse_UseMatColor "Soll die Materialfarbe f�r schattierte\
\nObjekte benutzt werden?"
ms_set de ayprefse_Background "Farbe des Hintergrundes."
ms_set de ayprefse_Object "Farbe nicht selektierter Objekte."
ms_set de ayprefse_Selection "Farbe selektierter Objekte."
ms_set de ayprefse_Grid "Farbe des Gitters."
ms_set de ayprefse_Tag "Farbe selektierter Punkte."
ms_set de ayprefse_Shade "Farbe f�r schattierte Objekte,\
wenn UseMatColor nicht aktiviert ist oder\ndas Objekt kein Material oder\
keine Materialfarbe hat."
ms_set de ayprefse_Light "Farbe f�r nicht selektierte Lichtquellen."

# RIB-Export
ms_set de ayprefse_RIBFile "Name der RIB-Datei f�r den RIB-Export.\
\nScene: RIB-Dateiname von Pfad und Namen der Szene ableiten,\
\nScenefile: RIB-Dateiname vom Namen der Szene (ohne Pfad) ableiten,\
\nAsk: RIB-Dateinamen beim RIB-Export erfragen."
ms_set de ayprefse_Image "Name der Bilddatei, die beim Rendern der\
exportierten\nRIB-Datei erzeugt wird.\
\nRIB: Bilddatei vom Namen der RIB-Datei ableiten."
ms_set de ayprefse_ResInstances "Sollen alle Instanzen-Objekte w�hrend des\
Exportierens\nin normale Objekte umgewandelt werden?"
ms_set de ayprefse_CheckLights "Soll eine Standardlichtquelle hinzugef�gt\
werden,\nwenn keine andere (aktive) Lichtquelle existiert?"
ms_set de ayprefse_DefaultMat "Standard-Material, das f�r alle Objekte\nohne\
eigenes Material benutzt wird.\nnone: kein Standard-Material verwenden\
\nmatte: RiMatte verwenden\
\ndefault: Material mit Namen \"default\" verwenden"
ms_set de ayprefse_RIStandard "Sollen alle Attribute und Optionen,\ndie nicht\
im RenderMan Standard vorkommen,\nbeim RIB-Export ausgelassen werden?"
ms_set de ayprefse_WriteIdent "Sollen Identifikatoren, basierend auf den\
Objektnamen,\nin die RIB-Datei geschrieben werden?"
ms_set de ayprefse_ShadowMaps "Sollen Schattenkarten verwendet werden?"
ms_set de ayprefse_ExcludeHidden "Sollen alle versteckten Objekte beim\
RIB-Export ausgelassen werden?"
ms_set de ayprefse_RenderMode "Wie soll der Vorschau-Renderer\
auf den Bildschirm rendern?\n\
CommandLineArg: per Aufrufparameter (Darstellung in Extra-Fenster), s.a.\
Render;\n\
RiDisplay: per RiDisplay-Kommando (Darstellung in Extra-Fenster);\n\
Viewport: per Display-Treiber-Plugin (Darstellung in Ayam), s.a. DisplayDriver."
ms_set de ayprefse_QRenderMode "Wie soll der schnelle Vorschau-Renderer\
auf den Bildschirm rendern?\n\
CommandLineArg: per Aufrufparameter (Darstellung in Extra-Fenster), s.a.\
QRender;\n\
RiDisplay: per RiDisplay-Kommando (Darstellung in Extra-Fenster);\n\
Viewport: per Display-Treiber-Plugin (Darstellung in Ayam), s.a.\
QDisplayDriver."
ms_set de ayprefse_QRender "Kommando und Aufrufparameter des schnellen\
 Vorschau-Renderers;\n\"%s\" wird durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_QRenderUI "Soll das Rendering-Fenster f�r die schnelle\
 Vorschau aktiviert werden?"
ms_set de ayprefse_QRenderPT "Ausgabe des schnellen Vorschau-Renderers;\
\n\"%d\" ist die Position des prozentualen Fortschrittswertes,\
\nder dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_Render "Kommando und Aufrufparameter des\
 Vorschau-Renderers;\n\"%s\" wird durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_RenderUI "Soll das Rendering-Fenster f�r die Vorschau\
aktiviert werden?"
ms_set de ayprefse_RenderPT "Ausgabe des Vorschau-Renderers;\
\n\"%d\" ist die Position des prozentualen Fortschrittswertes,\
\nder dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_FRender "Kommando und Aufrufparameter des\
 Datei-Renderers;\n\"%s\" wird durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_FRenderUI "Soll das Rendering-Fenster aktiviert werden?"
ms_set de ayprefse_FRenderPT "Ausgabe des Renderers;\
\n\"%d\" ist die Position des prozentualen Fortschrittswertes,\
\nder dann im Rendering-Fenster angezeigt wird."
ms_set de ayprefse_SMRender "Kommando und Aufrufparameter des\
 Schattenkarten-Renderers;\n\"%s\" wird durch den Dateinamen des RIBs ersetzt."
ms_set de ayprefse_SMRenderUI "Soll das Rendering-Fenster f�r Schattenkarten\
aktiviert werden?"
ms_set de ayprefse_SMRenderPT "Ausgabe des Schattenkarten-Renderers;\
\n\"%d\" ist die Position des prozentualen Fortschrittswertes,\
\nder dann im Rendering-Fenster angezeigt wird."

ms_set de ayprefse_DisplayDriver "Display-Treiber f�r Vorschau-Renderer."
ms_set de ayprefse_QDisplayDriver "Display-Treiber f�r den schnellen\
 Vorschau-Renderer."
ms_set de ayprefse_FDisplay "Display f�r Rendern in eine Datei."

ms_set de ayprefse_PPRender "Renderer, der f�r die permanente Vorschau\
verwendet werden soll."

ms_set de ayprefse_SetRenderer "Renderer zum Editieren ausw�hlen."

ms_set de ayprefse_SMFileFormat "Dateiformat der Schattenkarten.\
\nRenderMan: zfile\nGelato: shadow"
ms_set de ayprefse_SMFileType "Typ der Schattenkarten.\
\nz: normale Schattenkarten (RenderMan, Gelato)\
\navgz: Woo Schattenkarten (nur Gelato!)\
\nvolz: Volumen Schattenkarten (nur Gelato!)"
ms_set de ayprefse_SMChangeShaders "Sollen Shadernamen angepasst werden\
\n(Shadow voranstellen) wenn Schattenkarten benutzt werden?"
ms_set de ayprefse_AutoCloseUI "Soll das Rendering-Fenster geschlossen werden\
wenn der Renderer fertig ist?"

# Misc
ms_set de ayprefse_RedirectTcl "Sollen alle Fehlermeldungen von Tcl auf\
die Konsole umgelenkt werden?"
ms_set de ayprefse_Logging "Sollen alle Mitteilungen in einer Logdatei\
mitgeschrieben werden?"
ms_set de ayprefse_ErrorLevel "Welche Mitteilungen sollen in der Konsole\
angezeigt werden?\
\nSilence - Keine\
\nErrors - Nur Fehler\
\nWarnings - Warnungen und Fehler\
\nAll - Warnungen, Fehler und Informationen"
ms_set de ayprefse_LogFile "Pfad und Name der Logdatei."
ms_set de ayprefse_SaveAddsMRU "Sollen die Namen abgespeicherter Szenen\
den\nzuletzt-benutzte-Dateien-Eintr�gen im\nHauptmen� hinzugef�gt werden?"
ms_set de ayprefse_ImportSetsCD "Soll ein Import das aktuelle Verzeichnis\
setzen?"
ms_set de ayprefse_ExportSetsCD "Soll ein Export das aktuelle Verzeichnis\
setzen?"
ms_set de ayprefse_SetActionMenu "Elemente des Aktionsmen�s konfigurieren!"
ms_set de ayprefse_ToolBoxTrans "Soll das Werkzeugfenster als transient\
markiert werden?"
ms_set de ayprefse_ToolBoxShrink "Soll das Werkzeugfenster sich dem\
Inhalt anpassen,\nwenn es in der Gr��e ver�ndert wird?"
ms_set de ayprefse_RGTrans "Sollen alle Rendering Fenster als transient\
markiert werden?"
ms_set de ayprefse_TclPrecision "Genauigkeit der Wandlung von\
Gleitkommazahlen von Tcl.\nHohe Werte f�hren zu exakten aber schwer\
editierbaren\nWerten in Eingabefeldern."
ms_set de ayprefse_SaveDialogGeom "Geometrie von Dialogfenstern\
f�r den n�chsten Aufruf sichern?"
ms_set de ayprefse_SMethod "Abtastmethode, die f�r NURBS zu\nPolyMesh\
Umwandlungen benutzt wird."
ms_set de ayprefse_SParam "Parameter der Abtastmethode, die f�r NURBS\
zu\nPolyMesh Umwandlungen benutzt wird."


ms_set de plb_label \
"Diesen Text doppelklicken\num die Eigenschaft zu schlie�en."

ms_set de olb_label "Diesen Text doppelklicken\num zum Baum zu wechseln."

ms_set de tree_label "Diesen Text doppelklicken\num zur Liste zu wechseln."


# Mops-Import
ms_set de mopsi_options_ResetDM "Sollen alle DisplayMode-Attribute von\
importierten Objekten\nauf \"Global\" zur�ckgesetzt werden?"
ms_set de mopsi_options_ResetST "Sollen alle SamplingTolerance-Attribute von\
importierten\nObjekten auf \"0.0\" zur�ckgesetzt werden?"


# OBJ-Import/Export
ms_set de objio_options_Selected "Exportiert nur ausgew�hlte Objekte."
ms_set de objio_options_TessPoMesh "Wandelt alle PolyMesh-Objekte zu\
Dreiecksnetzen."
ms_set de objio_options_WriteCurves "Sollen NURBS Kurven exportiert werden?"

# info dialogs
ms_set de cancel "Abbrechen"
ms_set de info_sc1 "Szene ge�ndert!"
ms_set de info_sc2 "\"OK\" - �nderungen verwerfen.\
    \n\"Cancel\" - Operation abbrechen."
ms_fixcancel de info_sc1

ms_set de info_rs1 "Neustart erforderlich!"
ms_set de info_rs2 "Ayam muss neu gestartet werden,\num die �nderungen anzuwenden."

ms_set de info_warning "Warnung!"
ms_set de info_pc1 "Diese Operation kann die aktuelle Eigenschaft zerst�ren.\
    \n\"OK\" - fortfahren,\
    \n\"Cancel\" - Operation abbrechen."
ms_fixcancel de info_pc1

ms_set de info_sm1 "Konfiguration problematisch!"
ms_set de info_sm2 "Manuelle Schattenkarten sollten eingeschaltet sein!\
    \n\"OK\" - einschalten und fortfahren.\
    \n\"Cancel\" - Operation abbrechen."
ms_fixcancel de info_sm2

ms_set de info_rc1 "Kurve korrigieren?"
ms_set de info_rc2 "Kurve zur korrekten Ebene drehen?"

ms_set de info_rp1 "Voreinstellungen zur�cksetzen?"
ms_set de info_rp2 "Bereit zum L�schen der Datei:\n\"$ay(ayamrc)\"?"
ms_set de info_rp3 "Die Voreinstellungen wurden gel�scht!\nBitte Ayam neu starten."

ms_set de info_fi1 "Renderer reagiert nicht!"
ms_set de info_fi2 "Der Renderer hat den FIFO noch nicht erzeugt\
\n(DSPY nicht gefunden oder Schattenkarten werden gerendert).\
\n\"OK\" - weiter warten.\n\"Cancel\" - Operation abbrechen."

ms_set de info_separator "\nMehrere Eintr�ge durch '<sep>' trennen."

# view menu
#ms_set de vmenu1 "Aktuelle Modellieraktion"
ms_set de vmenu2 "Modellierraum �ndern\nDurchwechseln mit <lmk>"
ms_set de vmenu3 "Anzeigemodus �ndern\nDurchwechseln mit <dmu>/<dmd>"
ms_set de vmenu4 "Gitter �ndern\nDurchwechseln mit <gu>/<gd>"


#
# fill "fr"-locale...
ms_init fr
ms_set fr Missing "Manquant de traduction!"
ms_set fr ayprefse_Shaders "Liste de chemins o� r�sident vos shaders\
compil�s."
ms_set fr ayprefse_ScanShaders "Initialise la reconstruction de la base\
interne Shader."
ms_set fr ayprefse_Locale "Langue des bulles d'aide.\
\nLe changement prendra effet apr�s le red�marrage d'Ayam!"

#ms_set fr ayprefse_SingleWindow "Switch to single toplevel window GUI?"

ms_set fr ayprefse_AutoResize "Red�finir la taille de la fen�tre principale\
selon les propri�t�s par d�faut."
#ms_set fr ayprefse_AutoFocus "Move focus to window with mouse cursor?"

ms_set fr ayprefse_TwmCompat "Votre gestionnaire de fen�tre est-il\
compatible TWM?"
ms_set fr ayprefse_ListTypes "Montrer les types d'objets dans la vue en\
arbre/liste?"
#ms_set fr ayprefse_MarkHidden "Mark hidden objects in the tree view?"
ms_set fr ayprefse_AutoSavePrefs "Sauvegarder les param�tres en quittant?"
#ms_set fr ayprefse_BakOnReplace "Make backup copies of loaded scene files?"
ms_set fr ayprefse_LoadEnv "Charger l'environnement au d�marrage?"
ms_set fr ayprefse_NewLoadsEnv "Charger l'environnement lors de\
l'ouverture/la cr�ation d'un fichier?"
ms_set fr ayprefse_EnvFile "Chemin et nom de l'environnement\
\n(cr�� par menu \"Special/Save Environment\")."
ms_set fr ayprefse_Scripts "Liste de scripts TCL � ex�cuter au d�marrage.\
\nLe changement prendra effet apr�s le red�marrage d'Ayam!"
ms_set fr ayprefse_Plugins "Liste de chemins o� se trouvent les plugins."
ms_set fr ayprefse_Docs "URL poitant vers la documentation."
ms_set fr ayprefse_TmpDir "Chemin vers le dossier de sauvegarde des fichiers\
temporaire."

ms_set fr ayprefse_PickEpsilon "Distance maximale autoris�e du point\
s�lectionn� vers le point �ditable."
ms_set fr ayprefse_HandleSize "Taille des pogn�es des points �ditables en pixels."
ms_set fr ayprefse_LazyNotify "Notifier les objets parent � propos des\
changement simplement au survol de souris?"
#ms_set fr ayprefse_CompleteNotify "When shall a complete notification be\
#carried out?"
ms_set fr ayprefse_EditSnaps "Faire correspondre les coordonn�es des points\
�dit�s avec celles de la grille?"
#ms_set fr ayprefse_Snap3D "Snap coordinate values in all three dimensions?"
#ms_set fr ayprefse_FlashPoints "Flash editable points in single point\
#editing modes?"
#ms_set fr ayprefse_RationalPoints "Display type of rational points."
#ms_set fr ayprefse_GlobalMark "Maintain a single, global mark, or many local\
#marks (one for each view)?"
#ms_set fr ayprefse_CreateAtMark "Create objects at the mark?"
#ms_set fr ayprefse_DefaultAction "Modelling action invoked, when <Esc> key\
#is pressed."
#ms_set fr ayprefse_PickCycle "Cycle through pick candidates by further clicks?"
ms_set fr ayprefse_UndoLevels "Nombre de pas d'annulation dans l'historique\
de mod�lisation;\n0 correspond � la d�sactivation de l'historique."

# Drawing
ms_set fr ayprefse_Tolerance "Tol�rance utilis�e pour afficher de courbes\
ou surfaces NURBS.\nLes petites valeurs induisent un rendu plus lent\
mais une meilleure qualit�.\nLes objets NURBS peuvent outrepasser\
localement cette configuration."
ms_set fr ayprefse_NPDisplayMode "D�termine comment les surfaces seront\
trac�es\nLes objets Surface objects peuvent outrepasser localement cette\
configuration."
ms_set fr ayprefse_NCDisplayMode "D�termine comment les courbes seront\
trac�es\nLes objets Courbe objects peuvent outrepasser localement\
cette configuration."
ms_set fr ayprefse_ToleranceA "Tol�rance utilis�e pour afficher de courbes\
ou surfaces NURBS quand une action est active."
#ms_set fr ayprefse_NPDisplayModeA "Determine how surfaces should be drawn\
#\nwhen an action is active."
#ms_set fr ayprefse_NCDisplayModeA "Determine how curves should be drawn\
#\nwhen an action is active."

ms_set fr ayprefse_UseMatColor "Utiliser les couleurs du mat�riau pour\
les vue ombr�es?"
ms_set fr ayprefse_Background "Couleur de fond."
ms_set fr ayprefse_Object "Couleur pour les objets non s�lectionn�s."
ms_set fr ayprefse_Selection "Couleur pour les objets s�lectionn�s."
ms_set fr ayprefse_Grid "Couleur pour les grilles."
ms_set fr ayprefse_Tag "Couleur pour les points (�)s�lectionn�s."
ms_set fr ayprefse_Shade "Couleur pour les vues ombr�es lorsque que\
UseMatColor\nn'est as activ� ou que l'objet n'a pas de mat�riau\
affect�\nou pas de coueur de mat�riau."
ms_set fr ayprefse_Light "Couleur pour les objets Lumi�re (s�lectionn�s)."

# RIB-Export
ms_set fr ayprefse_RIBFile "Nom du fichier RIB � cr�er lors de l'Exportation."
ms_set fr ayprefse_Image "Nom du fichier image lors du rendu du fichier\
RIB export�."
ms_set fr ayprefse_ResInstances "R�soudre toutes les instance des objets\
� des objets\nnormaux lors de l'exportation RIB?"
ms_set fr ayprefse_CheckLights "Ajouter une source lumineuse distante,\
\nsi aucune source n'a �t� d�finie?"
ms_set fr ayprefse_DefaultMat "D�finir un mat�riau par d�faut dans le fichier\
\nRIB; il sera utilis� par tous les objets sans mat�riau?"
ms_set fr ayprefse_RIStandard "Omettre tous les attributs et toutes les\
options non d�finis\nen standard dans l'Interface RenderMan?"
ms_set fr ayprefse_WriteIdent "Ecrire un identificateur d�riv�\
du nom des objets dans le fichier RIB?"
ms_set fr ayprefse_ShadowMaps "Utiliser les ShadowMaps?\nAutomatic: Oui,\
Cr�er un RIB activant tout le temps le rendu ShadowMaps.\
\nManual: Oui, mais le rendu des ShadowMaps sera activ� par l'utilisateur\
seulement\n(Menu: View/Create ShadowMaps)"
ms_set fr ayprefse_ExcludeHidden "Omettre les objets cach�s � l'exportation\
RIB?"
ms_set fr ayprefse_RenderMode "Mode de rendu d'aper�u � l'�cran?"
ms_set fr ayprefse_QRenderMode "Mode de rendu d'aper�u rapides � l'�cran?"

ms_set fr ayprefse_QRender "Commandement et param�tres du moteur de rendu � utiliser\
pour les rendus d'aper�u rapides\n\"%s\" sera remplac� par le nom\
du fichier du RIB."
ms_set fr ayprefse_QRenderUI "Activer l'interface utilisateur lors le\
rendu rapide."
ms_set fr ayprefse_QRenderPT "Mod�le pour la progression � partir de la\
sortie du moteur de rendu.\n\"%d\" montre la position du num�ro\
de progression dans la sortie."
ms_set fr ayprefse_Render "Commandement et param�tres du moteur de rendu � utiliser\
pour les rendus d'aper�u.\n\"%s\" sera remplac� par le nom du fichier du\
RIB."
ms_set fr ayprefse_RenderUI "Activer l'interface utilisateur lors le rendu."
ms_set fr ayprefse_RenderPT "Mod�le pour la progression � partir de la\
sortie du moteur de rendu.\n\"%d\" montre la position du num�ro de\
progression dans la sortie."
#ms_set fr ayprefse_FRender "Command and parameters of the renderer to use for\
#rendering to a file.\n\"%s\" will be replaced by the filename\
#of the RIB."
#ms_set en ayprefse_FRenderUI "Enable user interface for rendering to a file."
#ms_set fr ayprefse_FRenderPT "A template that helps to pick the progress\
#from the output of the renderer.\n\"%d\" denotes the position of\
#the progress number in the output."
ms_set fr ayprefse_SMRender "Commandement et param�tres du moteur de rendu � utiliser\
pour les shadow maps.\n\"%s\" sera remplac� par le nom du fichier du\
RIB."
ms_set fr ayprefse_SMRenderUI "Activer l'interface utilisateur lors le\
rendu shadow maps."
ms_set fr ayprefse_SMRenderPT "Mod�le pour la progression � partir de la\
sortie du moteur de rendu.\n\"%d\" montre la position du num�ro de\
progression dans la sortie."
#ms_set fr ayprefse_SMFileFormat "File format of shadow map.\
#\nRenderMan: zfile\nGelato: shadow"
#ms_set fr ayprefse_SMFileType "Type of shadow map.\
#\nz: normal shadow map (RenderMan, Gelato)\
#\navgz: Woo shadow map (Gelato only!)\
#\nshadowvol: volume shadow map (Gelato only!)"
#ms_set fr ayprefse_SMChangeShaders "Change shader names (prepend\
# \"shadow\")\nwhen rendering with shadow maps?"
#ms_set fr ayprefse_AutoCloseUI "Automatically close the user interface\
#when rendering is finished?"

#ms_set fr ayprefse_DisplayDriver "Display driver to use for previews."
#ms_set fr ayprefse_QDisplayDriver "Display driver to use for quick previews."
#ms_set fr ayprefse_FDisplay "Display to use for rendering to files."

ms_set fr ayprefse_PPRender "Moteur de rendu � utiliser pour l'aper�u\
permanent."

ms_set fr ayprefse_SetRenderer "S�lectionnez rendu pour modifier."

# Misc
ms_set fr ayprefse_RedirectTcl "Rediriger tous les messages d'erreur\
TCL vers la console?"
ms_set fr ayprefse_Logging "Sauvegarder tous les message dans un fichier?"
ms_set fr ayprefse_LogFile "Chemin et nom du fichier de sauvegarde des\
messages?"
#ms_set fr ayprefse_ErrorLevel "Which messages should go to the console?\
#\nSilence - None\
#\nErrors - Only Errors\
#\nWarnings - Warnings and Errors\
#\nAll - Warnings, Errors, and Informative Messages"
ms_set fr ayprefse_SaveAddsMRU "Ajouter le nom des sc�ne sauvegard�es �\
l'entr�e de menu\nMost-Recently-Used files?"
#ms_set fr ayprefse_ImportSetsCD "Shall import set the current directory?"
#ms_set fr ayprefse_ExportSetsCD "Shall export set the current directory?"
#ms_set fr ayprefse_SetActionMenu "Configure elements of the action menu!"
ms_set fr ayprefse_SaveAddsMRU "Ajouter le nom des sc�ne sauvegard�es �\
l'entr�e de menu\nMost-Recently-Used files?"
ms_set fr ayprefse_ToolBoxTrans "Rendre la fen�tre Bo�te-�-Outils transient?"
ms_set fr ayprefse_ToolBoxShrink "Rendre la fen�tre Bo�te-�-Outils ajustable\
� son contenu,\nlorsque l'utilisateur en modifie la taille?"
ms_set fr ayprefse_RGTrans "Rendre toutes les interface utilisateur transient?"
ms_set fr ayprefse_TclPrecision "Precision des math�matiques Tcl."
#ms_set fr ayprefse_SaveDialogGeom "Save geometry of dialog windows for\
#next invocation?"

#ms_set fr ayprefse_SMethod "Sampling method to be used for NURBS\nto PolyMesh\
#conversions."
#ms_set fr ayprefse_SParam "Sampling method parameter."

ms_set fr mopsi_options_ResetDM "R�initialiser � Global tous les attributs\
DisplayMode de tous\nles objets lors de l'importation depuis une sc�ne Mops?"
ms_set fr mopsi_options_ResetST "R�initialiser � 0.0 tous les attributs\
SamplingTolerance de tous\nles objets lors de l'importation depuis une sc�ne\
Mops?"

#ms_set fr plb_label "Double click this label\nto deselect property."
#ms_set fr olb_label "Double click this label\nto switch to the tree."
#ms_set fr tree_label "Double click this label\nto switch to the listbox."

# OBJ-Import/Export
#ms_set fr objio_options_Selected "Export only selected object(s)."
#ms_set fr objio_options_TessPoMesh "Tesselate all PolyMesh objects to\
#triangles."
#ms_set fr objio_options_WriteCurves "Write NURBS curves to exported file?"

# info dialogs
ms_set fr cancel "Annuler"
ms_set fr info_sc1 "Changement sc�ne!"
#ms_set fr info_sc2 "Select \"OK\" to lose all changes.\nSelect \"Cancel\" to stop operation."
#ms_fixcancel fr info_sc2

ms_set fr info_rs1 "Red�marrage n�cessaire!"
#ms_set fr info_rs2 "Changes need a restart of Ayam to take effect!"


#ms_set fr info_warning "Attention!"
#ms_set fr info_pc1 "Cette op�ration peut d�truire la propri�t� actuelle.\
#		\n\"OK\" - continue,\
#		\n\"Cancel\" - cancel operation."
#ms_fixcancel fr info_pc1

#ms_set fr info_sm1 "Configuration problematic!"
#ms_set fr info_sm2 "Manual ShadowMaps should be enabled!\
#		\nSelect \"OK\" to enable them and continue.\
#		\nSelect \"Cancel\" to stop operation."
#ms_fixcancel fr info_sm2

#ms_set fr info_rc1 "Correct Curve?"
#ms_set fr info_rc2 "Rotate curve to correct plane?"

ms_set fr info_rp1 "R�initialiser pr�f�rences?"
ms_set fr info_rp2 "Pr�t pour supprimier le fichier:\n\"$ay(ayamrc)\"?"
#ms_set fr info_rp3 "Preferences file removed!\nPlease restart Ayam now."

#ms_set fr info_fi1 "Renderer not responding!"
#ms_set fr info_fi2 "Renderer did not create the FIFO yet\
#\n(DSPY not found or still rendering ShadowMaps).\
#\nSelect \"OK\" to continue waiting.\nSelect \"Cancel\" to stop operation."

ms_set fr info_separator "\nS�parez les entr�es par '<sep>'."

# view menu
#ms_set fr vmenu1 "current modelling action"
#ms_set fr vmenu2 "change global/local mode\ncycle with <lmk>"
#ms_set fr vmenu3 "change drawing mode\ncycle with <dmu>/<dmd>"
#ms_set fr vmenu4 "change gridsize\ncycle with <gu>/<gd>"


# EOF
