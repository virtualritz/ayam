[readme.txt - 28. Mar 2021]

This directory contains a collection of various plugins for Ayam.

This file gives just an overview about what is here; for more complete
documentation please refer to the main Ayam documentation:
http://ayam.sourceforge.net/docs/ayam-8.html#plugov
.

The plugins can be grouped in six categories:
1) shader parsing plugins,
2) import/export plugins,
3) object type plugins,
4) scripting language plugins
5) modelling helper plugins, and
6) rendering helper plugins.

Shader Parsing Plugins
----------------------

aygso - parse Gelato shaders
aysdr - parse Pixie shaders
ayslb - parse AIR shaders
ayslc - parse BMRT shaders
ayslo - parse Photorealistic RenderMan shaders
ayslo3d - parse 3Delight shaders
ayslx - parse Aqsis shaders
ayso - parse RDC shaders


Import/Export Plugins
---------------------

dxfio - AutoCAD DXF import/export
mfio - Apple 3DMF import/export
mopsi - The Mops import
objio - Wavefront OBJ import/export
onio - Rhino OpenNURBS 3DM import/export
rrib - RenderMan RIB import
x3dio - X3D import/export


Object Type Plugins
-------------------

csphere - example custom object
metaobj - model with implicit surfaces (meta balls)
sdnpatch - model with Subdivision NURBS
sfcurve - model with super formula curves
bcurve - model with basis curves
sdcurve - model with subdivision curves


Scripting Language Plugins
--------------------------

jsinterp - JavaScript plugin
luainterp - Lua plugin


Modelling Helper Plugins
------------------------

aycsg - CSG preview using OpenCSG
idr - importance driven rendering
aydnd - inter-application drag and drop support
subdiv - subdivision surfaces support
printps - print OpenGL drawings as SVG/EPS


Rendering Helper Plugins
------------------------

fifodspy - RenderMan display driver for viewport rendering
pixiefifodspy - Pixie display driver for viewport rendering
