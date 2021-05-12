#ifndef __aycore_h__
#define __aycore_h__
/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* aycore.h - prototypes of core functions */


/* bbc.c */

/** calculate the bounding box of object o
 */
int ay_bbc_get(ay_object *o, double *bbox);

/** bounding box calculation from control point array
 */
int ay_bbc_fromarr(double *arr, int len, int stride, double *bbox);

/** bounding box calculation from list of objects
 */
int ay_bbc_fromlist(ay_object *o, int update, double *bbox);

/** Tcl command to get the bounding box of the selected object(s).
 */
int ay_bbc_gettcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

/* clear.c */

/** remove all objects from the scene
 */
int ay_clear_scene(void);

/** Tcl command to remove all objects from the scene.
 */
int ay_clear_scenetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);


/* clevel.c */

/** set object on top of current level stack
 */
void ay_clevel_set(ay_object *o);

/** find object o in the hierarchy beneath object c
 */
int ay_clevel_find(ay_object *c, ay_object *o, int *found);

/** push object o onto current level stack
 */
int ay_clevel_add(ay_object *o);

/** pop current level stack
 */
void ay_clevel_del(void);

/** clear current level stack
 */
void ay_clevel_delall(void);

/** change current level to the top level
 */
void ay_clevel_gotop();

/** Tcl command to change the current level to the top level.
 */
int ay_clevel_gotoptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to change the current level to the parent level.
 */
int ay_clevel_gouptcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Tcl command to change the current level to the child level of the
 *  first selected object.
 */
int ay_clevel_godowntcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to get a string specification of the current level.
 */
int ay_clevel_gettcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Tcl command to change the current level
 */
int ay_clevel_cltcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);


/* clipb.c */

/** Properly clears the clipboard.
 */
int ay_clipb_clear();

/** Tcl command to copy selected objects to the object clipboard.
 */
int ay_clipb_copytcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Tcl command to move selected objects to the object clipboard.
 */
int ay_clipb_cuttcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/** Tcl command to copy/move the objects from the object clipboard to the scene.
 */
int ay_clipb_pastetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Tcl command to replace the selected objects with the objects from the
 *  object clipboard.
 */
int ay_clipb_replacetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to move the selected objects around in the current level.
 */
int ay_clipb_hmovtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** prepend objects to the clipboard
 */
void ay_clipb_prepend(ay_object *o, char *fname);


/* comp.c */

/** compare two strings
 */
int ay_comp_strcase(const char *s1, const char *s2);

/** register a comparison callback
 */
int ay_comp_register(ay_comparecb  *compcb, unsigned int type_id);

/** initialize object comparison module
 */
int ay_comp_init(void);

/** compare two objects
 */
int ay_comp_objects(ay_object *o1, ay_object *o2);

/** compare the transformation attributes of two objects
 */
int ay_comp_trafos(ay_object *o1, ay_object *o2);

/** compare the tags of two objects
 */
int ay_comp_tags(ay_object *o1, ay_object *o2);


/* convert.c */

/** register a conversion callback
 */
int ay_convert_register(ay_convertcb  *convcb, unsigned int type_id);

/** convert an object
 */
int ay_convert_object(ay_object *o, int in_place);

/** Tcl command to convert the selected objects.
 */
int ay_convert_objecttcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** helper for tool objects that convert to NURBS surfaces with caps/bevels
 */
int ay_convert_nptoolobj(ay_object *o, ay_object *p, ay_object *cb,
			 int in_place);


/* draw.c */

/** draw an object
 */
void ay_draw_object(struct Togl *togl, ay_object *o, int selected);

/** draw a view
 */
int ay_draw_view(struct Togl *togl, int draw_offset);

/** draw the grid of a view
 */
void ay_draw_grid(struct Togl *togl);

/** draw an arrow (to signify a vector or curve orientation)
 */
void ay_draw_arrow(struct Togl *togl, double *from, double *to);

/** draw the background image
 */
void ay_draw_bgimage(struct Togl *togl);

/** decide whether to redraw
 */
void ay_draw_needredraw(ay_list_object *oldsel, ay_list_object *newsel,
			int *result);

/** draw a view of type Trim
 */
void ay_draw_trimview(void);

/** draw an rectangle in image space
 */
void ay_draw_rectangle(int winwidth, int winheight,
		       double xmin, double ymin,
		       double xmax, double ymax);

/** draw a coordinate system (three color coded axes)
 */
void ay_draw_cs(struct Togl *togl, int mode);

/** draw the mark
 */
void ay_draw_mark(struct Togl *togl);

/** register draw annotations callback
 */
int ay_draw_registerdacb(ay_drawcb  *dacb, unsigned int type_id);

/** draw silhouette image
 */
void ay_draw_silhouettes(struct Togl *togl, unsigned char *silimg);

/** draw selected multiple points
 */
void ay_draw_selmp(ay_object *o, int is_rat, ay_mpoint *mp);

/** draw a rhombus (break point)
 */
void ay_draw_rhombus(double *pnt);

/** draw a 2D linestrip
 */
void ay_draw_linestrip(struct Togl *togl, int n, int stride, double *cv);


/* error.c */

/** report an error to the user
 */
void ay_error(int code, const char *where, const char *what);

/** Tcl command to report an error to the user.
 */
int ay_error_tcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[]);

/** GLU error callback
 */
void ay_error_glucb(GLenum err);

/** Format and print double range error message.
 */
int ay_error_reportdrange(char *fname, char *pname, double lb, double ub);

/** Format and print integer range error message.
 */
int ay_error_reportirange(char *fname, char *pname, int lb, int ub);

/** Format and print object path name as error message.
 */
int ay_error_reportobject(int ecode, char *fname, ay_object *o);

/** Format and print NaN error message.
 */
void ay_error_reportnan(char *fname, char *pname);

/** Get the current GL error state and report it to the user.
 */
int ay_error_printglerrortcmd(ClientData clientData, Tcl_Interp *interp,
			      int argc, char *argv[]);


/* geom.c */

/** calculate intersection of two 3D lines
 */
int ay_geom_intersectlines3D(double *p1, double *t1,
			     double *p2, double *t2,
			     double *p);

/** calculate intersection of two 2D lines
 */
int ay_geom_intersectlines2D(double *p1, double *t1,
			     double *p2, double *t2,
			     double *p);

/** convert three points to plane equation coefficients
 */
void ay_geom_pointstoplane(double x1, double y1, double z1,
			   double x2, double y2, double z2,
			   double x3, double y3, double z3,
			   double *A, double *B, double *C, double *D);

/** calculate normal from three points
 */
void ay_geom_normalfrom3pnts(double *p1, double *p2, double *p3, double *n);

/** Extract middle point.
 */
int ay_geom_extractmiddlepoint(int mode, double *cv, int cvlen, int cvstride,
			       double **tcv, double *result);

/** Extract mean normal.
 */
int ay_geom_extractmeannormal(double *cv, int cvlen, int cvstride,
			      double *m, double *result);

/** apply scale around cog for a number of points
 */
void ay_geom_scalecog(double scale, double *cv, int len, int stride);


/* instt.c */

/** Build OID hash table (OID->master).
 */
int ay_instt_createoidht(ay_object *o);

/** Connect instance objects to master objects based on OID tag information.
 */
void ay_instt_connect(ay_object *o, ay_object **last);

/** Create OID string.
 */
int ay_instt_createoid(char **dest);

/** Create OID tags for all referenced objects.
 */
int ay_instt_createorigids(ay_object *o);

/** Create OID tags for all instance objects.
 */
int ay_instt_createinstanceids(ay_object *o);

/** Remove all OID tags from the scene.
 */
int ay_instt_clearoidtags(ay_object *o);

/** remove instances of object m
 */
void ay_instt_removeinstances(ay_object **o, ay_object *m);

/** Check if there are instances in the clipboard, remove them.
 */
void ay_instt_clearclipboard(ay_object *o);

/** RIB export of instances to RI archives
 */
int ay_instt_wribiarchives(char *file, ay_object *o);

/** Tcl command to resolve selected instances to normal objects.
 */
int ay_instt_resolvetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** DnD helper.
 */
int ay_instt_checkinstance(ay_object *o, ay_object *target,
			   ay_object *instance, int *check_this_tree);

/** DnD helper.
 */
int ay_instt_check(ay_object *o, ay_object *target);

/** Tcl command to find the master of the selected instance.
 */
int ay_instt_getmastertcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

/** find instances of a master and count them
 */
void ay_instt_countrefs(ay_object *o, ay_object *m, unsigned int *refs);

/** find instances of a master and count them
 */
void ay_instt_countrefslist(ay_list_object *l, ay_object *m,
			    unsigned int *refs);

/** initialize instance tool module
 */
void ay_instt_init(Tcl_Interp *interp);


/* interpol.c */

/** interpolate between two 2D arrays of homogenous 3D coordinates
 */
int ay_interpol_2DA4DIC(ay_nurbpatch_object *np, int w, int h,
			double *st, double *en, double *ta);

/** interpolate between two arrays of homogenous 3D coordinates
 */
int ay_interpol_1DA4DIC(ay_nurbcurve_object *nc, int len, double *st,
			double *en, double *ta);

/** interpolate between two arrays of homogenous 3D coordinates
 */
int ay_interpol_1DA4D(double p, int len, double *st, double *en, double *ta);

/** interpolate between two arrays of doubles
 */
int ay_interpol_1DA1D(double p, int len, double *st, double *en, double *ta);

/** interpolate between two sets of transformation attributes
 */
void ay_interpol_trafos(double p, ay_object *o1, ay_object *o2, ay_object *ta);

/** interpolate between two curves
 */
int ay_interpol_ncurves(double p, ay_object *c1, ay_object *c2, ay_object *ic,
			ay_object **ta);

/** Tcl command to interpolate between two curves
 */
int ay_interpol_curvestcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);


/** interpolate between two surfaces
 */
int ay_interpol_npatches(double r, ay_object *p1, ay_object *p2,
			 ay_object *ic, ay_object **ta);

/** Tcl command to interpolate between two surfaces
 */
int ay_interpol_surfacestcmd(ClientData clientData, Tcl_Interp *interp,
			     int argc, char *argv[]);


/* notify.c */

/** register notify callback
 */
int ay_notify_register(ay_notifycb *notcb, unsigned int type_id);

/** call notification callbacks of all parent objects (of current level)
 */
int ay_notify_parent(void);

/** call notification callback of object o
 */
int ay_notify_object(ay_object *o);

/** call notification callback of parent of object o
 */
int ay_notify_parentof(ay_object *o, int silent);

/** Tcl command to enforce notification of selected objects.
 */
int ay_notify_objecttcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** find parents
 */
int ay_notify_findparents(ay_object *o, ay_object *r,
			  ay_list_object **parents);

/** do complete notification for object r
 */
int ay_notify_complete(ay_object *r);

/** manage blocking of automatic notifications
 */
void ay_notify_block(int scope, int block);

/** initialize notification module
 */
void ay_notify_init(Tcl_Interp *interp);

/** Tcl command to enforce notification of selected objects (safe interp).
 */
int ay_notify_objectsafetcmd(ClientData clientData, Tcl_Interp *interp,
			     int argc, char *argv[]);

/* ns.c */

/** restrict processing of Tk events
 */
Tk_RestrictAction ay_ns_restrictall(ClientData clientData,
				    XEvent *eventPtr);

/** execute script from ANS/BNS tag
 */
void ay_ns_execute(ay_object *o, ay_tag *tag);

/** disable a ANS/BNS tag
 */
int ay_ns_disable(ay_tag *tag);

/** initialize notify script tag module
 */
void ay_ns_init(Tcl_Interp *interp);


/* oact.c */

/** helper function to parse args
 */
int ay_oact_parseargs(struct Togl *togl, int argc, char *argv[], char *fname,
		      double *winx, double *winy,
		      double *owinx, double *owiny);

/** move object modelling action
 */
int ay_oact_movetcb(struct Togl *togl, int argc, char *argv[]);

/** rotate object modelling action
 */
int ay_oact_rottcb(struct Togl *togl, int argc, char *argv[]);

/** rotate about mark object modelling action
 */
int ay_oact_rotatcb(struct Togl *togl, int argc, char *argv[]);

/** scale 1D X object modelling action
 */
int ay_oact_sc1DXcb(struct Togl *togl, int argc, char *argv[]);

/** scale 1D X about mark object modelling action
 */
int ay_oact_sc1DXAcb(struct Togl *togl, int argc, char *argv[]);

/** scale 1D Y object modelling action
 */
int ay_oact_sc1DYcb(struct Togl *togl, int argc, char *argv[]);

/** scale 1D Y about mark object modelling action
 */
int ay_oact_sc1DYAcb(struct Togl *togl, int argc, char *argv[]);

/** scale 1D Z object modelling action
 */
int ay_oact_sc1DZcb(struct Togl *togl, int argc, char *argv[]);

/** scale 1D Z about mark object modelling action
 */
int ay_oact_sc1DZAcb(struct Togl *togl, int argc, char *argv[]);

/** scale 2D object modelling action
 */
int ay_oact_sc2Dcb(struct Togl *togl, int argc, char *argv[]);

/** scale 3D object modelling action
 */
int ay_oact_sc3Dcb(struct Togl *togl, int argc, char *argv[]);

/** stretch 2D object modelling action
 */
int ay_oact_str2Dcb(struct Togl *togl, int argc, char *argv[]);

/** scale 2D about mark object modelling action
 */
int ay_oact_sc2DAcb(struct Togl *togl, int argc, char *argv[]);

/** stretch 2D about mark object modelling action
 */
int ay_oact_str2DAcb(struct Togl *togl, int argc, char *argv[]);

/** scale 3D about mark object modelling action
 */
int ay_oact_sc3DAcb(struct Togl *togl, int argc, char *argv[]);


/* object.c */

/** set attributes of freshly allocated object o to defaults
 */
void ay_object_defaults(ay_object *o);

/** place object o at the current mark
 */
void ay_object_placemark(ay_object *o);

/** create an object
 */
int ay_object_create(unsigned int index, ay_object **o);

/** create an object with parameters
 */
int ay_object_createargs(unsigned int index, int argc, char **argv,
			 ay_object **o);

/** Tcl command to create a new object and link it to the current level.
 */
int ay_object_createtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** delete an objet
 */
int ay_object_delete(ay_object *o);

/** delete multiple (connected) objects
 */
int ay_object_deletemulti(ay_object *o, int force);

/** Tcl command to delete selected objects.
 */
int ay_object_deletetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** link object to the scene hierarchy
 */
void ay_object_link(ay_object *o);

/** unlink object from the scene hierarchy
 */
void ay_object_unlink(ay_object *o);

/** get type name
 */
char *ay_object_gettypename(unsigned int index);

/** get name of object o
 */
char *ay_object_getname(ay_object *o);

/** Tcl command to set the name of the selected objects.
 */
int ay_object_setnametcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** copy an object
 */
int ay_object_copy(ay_object *src, ay_object **dst);

/** copy multiple (connected) objects
 */
int ay_object_copymulti(ay_object *src, ay_object **dst);

/** Tcl command to inquire whether an object has certain properties.
 */
int ay_object_ishastcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to get the type or name of an object.
 */
int ay_object_gettypeornametcmd(ClientData clientData, Tcl_Interp *interp,
				int argc, char *argv[]);

/** create level terminator object
 */
int ay_object_crtendlevel(ay_object **o);

/** replace object dst with src
 */
int ay_object_replace(ay_object *src, ay_object *dst);

/** count objects beneath o
 */
unsigned int ay_object_count(ay_object *o);

/** can objects in o be deleted?
 */
int ay_object_candelete(ay_object *p, ay_object *o);

/** can objects in l be deleted?
 */
int ay_object_candeletelist(ay_list_object *l, ay_object *o);

/** get complete object path name
 */
int ay_object_getpathname(ay_object *o, ay_object *h,
			  size_t *totallen, int *found, char **result);

/** find object in scene hierarchy
 */
int ay_object_find(ay_object *o, ay_object *h);


/* otype.c */

/** register core object type (with known type id)
 */
int ay_otype_registercore(char *name,
			  ay_createcb  *crtcb,
			  ay_deletecb  *delcb,
			  ay_copycb    *copycb,
			  ay_drawcb    *drawcb,
			  ay_drawcb    *drawhcb,
			  ay_drawcb    *shadecb,
			  ay_propcb    *setpropcb,
			  ay_propcb    *getpropcb,
			  ay_getpntcb  *getpntcb,
			  ay_readcb    *readcb,
			  ay_writecb   *writecb,
			  ay_wribcb    *wribcb,
			  ay_bbccb     *bbccb,
			  unsigned int type_index);

/** register object type (from plugin, create a type id)
 */
int ay_otype_register(char *name,
		      ay_createcb  *crtcb,
		      ay_deletecb  *delcb,
		      ay_copycb    *copycb,
		      ay_drawcb    *drawcb,
		      ay_drawcb    *drawhcb,
		      ay_drawcb    *shadecb,
		      ay_propcb    *setpropcb,
		      ay_propcb    *getpropcb,
		      ay_getpntcb  *getpntcb,
		      ay_readcb    *readcb,
		      ay_writecb   *writecb,
		      ay_wribcb    *wribcb,
		      ay_bbccb     *bbccb,
		      unsigned int *type_index);

/** create pointer to ID integer value for Tcl hash table value
 */
unsigned int *ay_otype_getpointer(unsigned int id);

/** initialize object type module
 */
int ay_otype_init();


/* matt.c */

/** initialize material tools module
 */
void ay_matt_init(Tcl_Interp *interp);

/** register a material
 */
int ay_matt_registermaterial(char *name, ay_mat_object *mat);

/** de-register material
 */
int ay_matt_deregister(char *name);

/** get registered material object from name
 */
int ay_matt_getmaterial(char *name, ay_mat_object **material);

/** remove all material references
 */
void ay_matt_removeallrefs(ay_object *o);

/** remove all references to given material
 */
void ay_matt_removerefs(ay_object *o, ay_mat_object *material);

/** remove material references from clipboard
 */
void ay_matt_removecliprefs(ay_object *o);

/** Connect objects to the appropriate material objects (using MI tags).
 */
void ay_matt_connect(ay_object *o);

/** Creates MI tags for all objects with a material.
 */
int ay_matt_creatematerialids(ay_object *o);

/** Removes MI tags.
 */
int ay_matt_clearmaterialids(ay_object *o);

/** RIB export of material information
 */
int ay_matt_wrib(ay_mat_object *m);

/** objects of type may have no material
 */
void ay_matt_nomaterial(unsigned int type);

/** Check whether objects of given type may have a material.
 */
int ay_matt_mayhavematerial(unsigned int type);

/** Remove all shaders from a material.
 */
void ay_matt_clearshaders(ay_mat_object *material);

/** Tcl command to get all registered material names.
 */
int ay_matt_getregisteredtcmd(ClientData clientData, Tcl_Interp *interp,
			      int argc, char *argv[]);


/* pact.c */

/** helper to clear a pointedit structure
 */
void ay_pact_clearpointedit(ay_pointedit *pe);

/** helper to get minimum scale of current level
 */
double ay_pact_getminlevelscale();

/** select points
 */
int ay_pact_getpoint(int mode, ay_object *o, double *obj, ay_pointedit *pe);

/** pick single point
 */
int ay_pact_pickpoint(ay_object *o, ay_view_object *view,
		      double minlevelscale, double *obj, ay_pointedit *pe);

/** tag/select point action
 */
int ay_pact_seltcb(struct Togl *togl, int argc, char *argv[]);

/** tag/select boundary action
 */
int ay_pact_selboundtcb(struct Togl *togl, int argc, char *argv[]);

/** insert point action
 */
int ay_pact_insertptcb(struct Togl *togl, int argc, char *argv[]);

/** delete point action
 */
int ay_pact_deleteptcb(struct Togl *togl, int argc, char *argv[]);

/** start point action helper
 */
int ay_pact_startpetcb(struct Togl *togl, int argc, char *argv[]);

/** numerically edit coordinate values of points action
 */
int ay_pact_pentcb(struct Togl *togl, int argc, char *argv[]);

/** helper to snap points to the grid
 */
void ay_pact_griddify(double *n, double grid);

/** helper to flash points
 */
void ay_pact_flashpoint(int ignore_old, int multiple, double *pnt, ay_object *o);

/** direct point edit action
 */
int ay_pact_petcb(struct Togl *togl, int argc, char *argv[]);

/** edit weights action
 */
int ay_pact_wetcb(struct Togl *togl, int argc, char *argv[]);

/** reset all weights action
 */
int ay_pact_wrtcb(struct Togl *togl, int argc, char *argv[]);

/** snap selected points to the grid action
 */
int ay_pact_snaptogridcb(struct Togl *togl, int argc, char *argv[]);

/** snap selected points to the mark action
 */
int ay_pact_snaptomarkcb(struct Togl *togl, int argc, char *argv[]);

/** multiply selected points action
 */
int ay_pact_multiptcb(struct Togl *togl, int argc, char *argv[]);

/** insert point into approximating curve
 */
int ay_pact_insertac(ay_acurve_object *curve, int *index,
		     double *objXYZ, int edit);

/** insert point into NURBS curve
 */
int ay_pact_insertnc(ay_nurbcurve_object *curve, int *index,
		     double *objXYZ, int edit);

/** delete point from approximating curve
 */
int ay_pact_deleteac(ay_acurve_object *acurve, int *index,
		     double *objXYZ);

/** delete point from NURBS curve
 */
int ay_pact_deletenc(ay_nurbcurve_object *curve, int *index, double *objXYZ);

/** register insert point callback
 */
int ay_pact_registerinsert(ay_inspntcb *inscb, unsigned int type_id);

/** register delete point callback
 */
int ay_pact_registerdelete(ay_delpntcb *delcb, unsigned int type_id);

/** initialize point action module
 */
int ay_pact_init(Tcl_Interp *interp);


/* peek.c */

/** register a peek callback
 */
int ay_peek_register(ay_peekcb *peekcb, unsigned int type_id);

/** provide objects of desired type and their transformations
 */
int ay_peek_object(ay_object *o, unsigned int type, ay_object ***objrefs,
		   double **transforms);

/** set a peek tag
 */
int ay_peek_settag(ay_object *o, ay_object *p);


/* pomesht.c */

/** delete a polymesh object
 */
int ay_pomesht_destroy(ay_pomesh_object *pomesh);

/** tesselate polymesh object (for drawing/shading purposes)
 */
int ay_pomesht_tesselate(ay_pomesh_object *pomesh);

/** merge polymesh objects
 */
int ay_pomesht_merge(int merge_pv_tags, ay_list_object *list,
		     ay_object **result);

/** Tcl command to merge the selected polymesh objects.
 */
int ay_pomesht_mergetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** optimize polymesh object
 */
int ay_pomesht_optimizecoords(ay_pomesh_object *pomesh, double normal_epsilon,
			      ay_point *selp,
			      unsigned int *ois, unsigned int *oislen);

/** Tcl command to optimize the selected polymesh objects.
 */
int ay_pomesht_optimizetcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

/** convert polymesh object to subdision mesh object
 */
int ay_pomesht_tosdmesh(ay_pomesh_object *pomesh, ay_sdmesh_object **sdmesh);

/** split off a face from a polymesh object
 */
int ay_pomesht_splitface(ay_pomesh_object *pomesh, unsigned int f,
			 ay_pomesh_object *target);

/** split off a part from a polymesh object
 */
int ay_pomesht_split(ay_pomesh_object *pomesh, ay_point *pnts,
		     ay_pomesh_object **result);

/** Tcl command to split selected faces from selected polymesh objects.
 */
int ay_pomesht_splittcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** generate face normals for a polymesh object
 */
int ay_pomesht_genfacenormals(ay_pomesh_object *po, double **result);

/** generate smooth normals for a polymesh object
 */
int ay_pomesht_gensmoothnormals(ay_pomesh_object *po, double **result);

/** remove smooth normals from a polymesh object
 */
int ay_pomesht_remsmoothnormals(ay_pomesh_object *po);

/** Tcl command to create smooth vertex normals or face normal PV tags
 *  for the selected polymesh objects.
 */
int ay_pomesht_gennormtcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

/** flip/reverse all loops of a polymesh object
 */
void ay_pomesht_fliploops(ay_pomesh_object *po);

/** flip/reverse all vertex normals of a polymesh object
 */
void ay_pomesht_flipnormals(ay_pomesh_object *po);

/** Tcl command to connect polymesh object edges.
 */
int ay_pomesht_connecttcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

/** select all points of a boundary of a polymesh object
 */
int ay_pomesht_selectbound(ay_pomesh_object *po, ay_point *selp);

/** apply transformations to control points and normals
 */
int ay_pomesht_applytrafo(ay_object *o);

/** See if there are only ngons in the mesh.
 */
int ay_pomesht_hasonlyngons(ay_pomesh_object *po, unsigned int n);


/* prefs.c */

/** Tcl command to get the preferences (C => Tcl)
 */
int ay_prefs_gettcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/** Tcl command to set the preferences (Tcl => C)
 */
int ay_prefs_settcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);


/* prop.c */

/** Tcl command to get the property of an object
 */
int ay_prop_gettcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** Tcl command to set the property of an object
 */
int ay_prop_settcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** Tcl command to get the transformations property
 */
int ay_prop_gettrafotcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to set the transformations property
 */
int ay_prop_settrafotcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to get the attributes property
 */
int ay_prop_getattrtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to set the attributes property
 */
int ay_prop_setattrtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to get the material property
 */
int ay_prop_getmattcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Tcl command to set the material property
 */
int ay_prop_setmattcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** generate nurbs patch configuration info string
 */
int ay_prop_getnpinfo(Tcl_Interp *interp, char *n1, ay_object *o);

/** generate nurbs curve configuration info string
 */
int ay_prop_getncinfo(Tcl_Interp *interp, char *n1, ay_object *o);

/** parse and apply simple one operand expression
 */
int ay_prop_parseexpression(Tcl_Obj *to, char *fname, double oldval,
			    double *newval);


/* provide.c */

/** register a provide callback
 */
int ay_provide_register(ay_providecb  *provcb, unsigned int type_id);

/** provide an object of desired type
 */
int ay_provide_object(ay_object *o, unsigned int type, ay_object **result);

/** helper for tool objects that provide NURBS surfaces with caps/bevels
 */
int ay_provide_nptoolobj(ay_object *o, unsigned int type,
			 ay_object *npatch, ay_object *cb,
			 ay_object **result);


/* pv.c */

/** parse all PV tags to RtToken/RtPointer arrays
 */
int ay_pv_filltokpar(ay_object *o, int declare, int start,
		     int *added, RtToken tokens[], RtPointer parms[]);

/** add PV tag to an object
 */
int ay_pv_add(ay_object *o,
	      const char *name, const char *detail, const char *type,
	      int datalen, int stride, void *data);

/** merge two PV tags into one
 */
int ay_pv_merge(ay_tag *t1, ay_tag *t2, ay_tag **mt);

/** merge two PV tags into one
 */
int ay_pv_mergeinto(ay_tag *t1, ay_tag *t2);

/** compare two PV tags name, detail, and data type
 */
int ay_pv_cmpndt(ay_tag *t1, ay_tag *t2);

/** check a PV tags name, detail, and data type
 */
int ay_pv_checkndt(ay_tag *t, const char *name, const char *detail,
		   const char *type);

/** return a PV tags detail
 */
int ay_pv_getdetail(ay_tag *t, char **detail);

/** return a PV tags type
 */
int ay_pv_gettype(ay_tag *t);

/** convert PV tag data to value array
 */
int ay_pv_convert(ay_tag *tag, int type, unsigned int *datalen, void **data);

/** get texture coordinates from PV tag
 */
int ay_pv_getst(ay_object *o, char *mys, char *myt, void **data);

/** get vertex colors from object
 */
int ay_pv_getvc(ay_object *o, char *myc, int stride,
		unsigned int *datalen, void **data);

/** count PV tags of object o
 */
int ay_pv_count(ay_object *o);

/** fix/correct number of elements in a PV tag
 */
void ay_pv_fixnumelems(char *buf, unsigned int numelems);

/** initialize pv module
 */
void ay_pv_init(Tcl_Interp *interp);


/* quat.c */

/** convert rotation from axis/angle to quaternion
 */
void ay_quat_axistoquat(double a[3], double phi, double q[4]);

/** normalize a quaternion
 */
void ay_quat_norm(double q[4]);

/** add two quaternions
 */
void ay_quat_add(double q1[4], double q2[4], double dest[4]);

/** convert quaternion to rotation matrix
 */
void ay_quat_torotmatrix(double q[4], double m[16]);

/** convert quaternion to euler angles
 */
void ay_quat_toeuler(double q[4], double euler[3]);

/** interpolate between two quaternions
 */
void ay_quat_slerp(double t, double q1[4], double q2[4], double r[4]);

/** calculate dot product of two quaternions
 */
double ay_quat_dot(double q1[4], double q2[4]);

/** calculate inverted quaternion
 */
void ay_quat_inv(double q[4]);


/* read.c */

/** read string from Ayam scene file
 */
int ay_read_string(FILE *fileptr, char **result);

/** read UNICODE string from Ayam scene file
 */
int ay_read_unistring(FILE *fileptr, Tcl_UniChar **result);

/** skip to next object
 */
int ay_read_skip(FILE *fileptr);

/** read header info from Ayam scene file
 */
int ay_read_header(FILE *fileptr);

/** read standard object attributes from Ayam scene file
 */
int ay_read_attributes(FILE *fileptr, ay_object *o);

/** read object tags from Ayam scene file
 */
int ay_read_tags(FILE *fileptr, ay_object *o);

/** read shader from Ayam scene file
 */
int ay_read_shader(FILE *fileptr, ay_shader **result);

/** read object from Ayam scene file
 */
int ay_read_object(FILE *fileptr);

/** read scene from Ayam scene file
 */
int ay_read_scene(Tcl_Interp *interp, char *filename, int insert);

/** Tcl command to open (replace) a scene file
 */
int ay_read_replacetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to open (insert) a scene file
 */
int ay_read_inserttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);


/* riattr.c */

/** RIB export of RiAttr tag data
 */
int ay_riattr_wrib(ay_object *o);

/** initialize riattr module
 */
void ay_riattr_init(Tcl_Interp *interp);


/* riopt.c */

/** RIB export of RiOpt tag data
 */
int ay_riopt_wrib(ay_object *o);

/** initialize riopt module
 */
void ay_riopt_init(Tcl_Interp *interp);


/* sdmesht.c */

/** tesselate a SDMesh (for drawing/shading purposes)
 */
int ay_sdmesht_tesselate(ay_sdmesh_object *sdmesh);

/** convert a SDMesh to a PolyMesh
 */
int ay_sdmesht_topolymesh(ay_sdmesh_object *sdmesh, ay_pomesh_object **pomesh);


/* sel.c */

/** clear the selection
 */
void ay_sel_free(int clear_selflag);

/** add object to selection
 */
int ay_sel_add(ay_object *o, int set_selflag);

/** Tcl command to set the selection.
 */
int ay_sel_selobtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/** Tcl command to get the selection.
 */
int ay_sel_getseltcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Tcl command to silently select the last object in current level.
 */
int ay_sel_hsltcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

/** Tcl command to silently select the parent object.
 */
int ay_sel_hsptcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

/** helper to clear selected flags from object hierarchies
 */
void ay_sel_clearselflag(ay_object *o);

/** clean selection from removed objects
 */
void ay_sel_clean();


/* selp.c */

/** clear point selection of an object
 */
void ay_selp_clear(ay_object *o);

/** copy list of selected points
 */
void ay_selp_copy(ay_point *pnt, ay_point **res);

/** select all points of an object
 */
int ay_selp_selall(ay_object *o);

/** Tcl command to apply object transformations to points.
 */
int ay_selp_applytrafotcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

/** invert point selection of an object
 */
int ay_selp_invert(ay_object *o);

/** Tcl command to invert the point selection of an object.
 */
int ay_selp_inverttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** center selected points
 */
int ay_selp_center(ay_point *p, int dim, double *center);

/** get center of selected points
 */
int ay_selp_getcenter(ay_point *p, int mode, double *center);

/** Tcl command to center all points of an object.
 */
int ay_selp_centertcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** add points to point selection
 */
int ay_selp_sel(ay_object *o, unsigned int indiceslen, unsigned int *indices);

/** Tcl command to manipulate the point selection.
 */
int ay_selp_seltcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** repair selected points indizes after point removal
 */
int ay_selp_rem(ay_object *o, unsigned int index);

/** repair selected points indizes after point insertion
 */
int ay_selp_ins(ay_object *o, unsigned int index);

/** getpnts callback helper
 */
int ay_selp_getpnts(int mode, ay_object *o, double *p, ay_pointedit *pe,
		    int readonly, int arrlen, int stride, int ishom,
		    double *arr);

/** properly select multiple points of a NURBS curve
 */
void ay_selp_selectmpnc(ay_object *o, int select_all);

/** normalize selected points
 */
void ay_selp_normalize(ay_object *o, int digits);

/** Tcl command to collapse selected points of selected NURBS
 *  curves/patches to multiple points.
 */
int ay_selp_collapsetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to explode selected multiple points of selected NURBS
 *  curves/patches to single points.
 */
int ay_selp_explodetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Compile array of points from a list of objects.
 */
void ay_selp_managelist(ay_object *o, unsigned int *lenptr, double **pntsptr);

/** Change the point selection to minimum/maximum.
 */
int ay_selp_reducetominmax(ay_object *o);

/** Update selection state of multiple points.
 */
void ay_selp_updatempselection(unsigned int n, ay_point *selp, ay_mpoint *mp);

/** Check whether a given point is among a list of selected points.
 */
int ay_selp_find(ay_point *selp, double *point);


/* shade.c */

/** shade an object
 */
void ay_shade_object(struct Togl *togl, ay_object *o, int push_name);

/** shade complete scene
 */
int ay_shade_view(struct Togl *togl);


/* shader.c */

#ifdef AYUSESLCARGS
/** scan a SLC shader argument
 */
int ay_shader_scanslcsarg(SLC_VISSYMDEF *symbol, Tcl_DString *ds);
#endif


/** Tcl command to scan a SLC (BMRT) shader
 */
int ay_shader_scanslctcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);
#ifdef AYUSESLXARGS
/** scan a SLX shader argument
 */
int ay_shader_scanslxsarg(SLX_VISSYMDEF *symbol, Tcl_DString *ds);
#endif

/** Tcl command to scan a SLX (Aqsis) shader
 */
int ay_shader_scanslxtcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** delete a shader
 */
void ay_shader_free(ay_shader *shader);

/** copy a shader argument
 */
int ay_shader_copyarg(ay_shader_arg *source, ay_shader_arg **dest);

/** copy a shader
 */
int ay_shader_copy(ay_shader *source, ay_shader **dest);

/** RIB export of a shader
 */
int ay_shader_wrib(ay_shader *shader, int type, RtLightHandle *light_handle);

/** Tcl command to get a shader property
 */
int ay_shader_gettcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Tcl command to set a shader property
 */
int ay_shader_settcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);


/* table.c */

/** initialize callback table
 */
int ay_table_initftable(ay_ftable *table);

/** add item to callback table
 */
int ay_table_addcallback(ay_ftable *table, ay_voidfp newitem, unsigned int index);

/** initialize object table
 */
int ay_table_init(ay_otable *table);

/** add item to object table
 */
int ay_table_additem(ay_otable *table, void *newitem, unsigned int index);


/* tags.c */

/** remove all tags from an object
 */
void ay_tags_delall(ay_object *o);

/** remove tags from an object
 */
void ay_tags_delete(ay_object *o, unsigned int type);

/** delete a single tag
 */
void ay_tags_free(ay_tag *tag);

/** copy a single tag
 */
int ay_tags_copy(ay_tag *source, ay_tag **dest);

/** copy all tags from object src to object dst
 */
int ay_tags_copyall(ay_object *src, ay_object *dst);

/** append tag to object
 */
void ay_tags_append(ay_object *o, ay_tag *tag);

/** register a tag type
 */
int ay_tags_register(char *name, unsigned int *result);

/** Tcl command to set the tags property
 */
int ay_tags_settcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** Tcl command to add tags to an object
 */
int ay_tags_addtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** Tcl command to get the tags property
 */
int ay_tags_gettcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** check for existence of a tag
 */
int ay_tags_hastag(const ay_object *o, const unsigned int tagtype);

/** Tcl command to check for existence of a tag
 */
int ay_tags_hastcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** get first tag of given type
 */
void ay_tags_getfirst(const ay_object *o, const unsigned int tagtype,
		      ay_tag **result);

/** Tcl command to delete tags from an object
 */
int ay_tags_deletetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** parse PV tags for RIB export
 */
int ay_tags_parseplist(char *str, int declare, RtInt *argc, RtToken **tokensr,
		       RtPointer **valuesr);

/** reconnect already existing tags after late tag type registration
 */
void ay_tags_reconnect(ay_object *o, unsigned int tagtype, char *tagname);

/** add NO/NM tags
 */
int ay_tags_addnonm(ay_object *o, ay_object *m);

/** remove NO/NM tags
 */
void ay_tags_remnonm(ay_object *o, ay_object *m);

/** copy selected tags
 */
int ay_tags_copyselected(ay_object *src, ay_object *dst,
			 unsigned int *types, int typeslen);

/** convert newline characters
 */
void ay_tags_nltovt(char *str);

/** convert newline characters
 */
void ay_tags_vttonl(char *str);

/** Tcl command to register a new tag type */
int ay_tags_registertcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);


/* tc.c */

/** write the first TC tag from object o to RIB
 */
int ay_tc_wrib(ay_object *o);

/** initialize tc module by registering the TC tag type
 */
void ay_tc_init(Tcl_Interp *interp);


/* tcmd.c */

/** convert a Tcl list of doubles to a C array of doubles
 */
int ay_tcmd_convdlist(char *vname, int *dllen, double **dl);

/** Tcl command to revert curves
 */
int ay_tcmd_reverttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** show/hide all objects below o
 */
void ay_tcmd_showhideall(ay_object *o, int val);

/** Tcl command to show (un-hide) all objects
 */
int ay_tcmd_showhidetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to get the Ayam version
 */
int ay_tcmd_getversionstcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

/** get bilinear patch coordinate address from index
 */
int ay_tcmd_getbppntfromindex(ay_bpatch_object *patch, int index,
			      double **p);

/** Tcl command to get the coordinates of points
 */
int ay_tcmd_getpointtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to set the coordinates of points
 */
int ay_tcmd_setpointtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

#ifdef AYENABLEWAIT
/** Tcl command to wait for a spawned process in order to avoid zombies
 */
int ay_tcmd_waitpidtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);
#endif /* AYENABLEWAIT */

#ifdef AYENABLEFEXIT
/** Tcl command to exit Ayam
 */
int ay_tcmd_fastexittcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);
#endif /* AYENABLEFEXIT */

/** Tcl command to execute command for a single object
 */
int ay_tcmd_withobtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Manage scripting language IDs
 */
int ay_tcmd_registerlang(char *name, unsigned int *result);

/** get string from Tcl variable
 */
int ay_tcmd_getstring(Tcl_Interp *interp, char *arr, char *var, char **result);

/** convert string to unsigned int
 */
int ay_tcmd_getuint(char *str, unsigned int *uint);

/** Tcl command to set action state from menu state
 */
int ay_tcmd_menustatetcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** Tcl command to get plane normal from e.g. a planar curve
 */
int ay_tcmd_getplanenormaltcmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[]);

/** Tcl command to open/close a curve
 */
int ay_tcmd_openclosetcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** Tcl command to refine/coarsen a curve
 */
int ay_tcmd_refinecoarsentcmd(ClientData clientData, Tcl_Interp *interp,
			      int argc, char *argv[]);

/** register generic operation callback
 */
int ay_tcmd_registergeneric(int op, ay_genericcb *cb, unsigned int type_id);

/** initialize tcmd module
 */
int ay_tcmd_init(Tcl_Interp *interp);

/** Tcl command to create a tool object from selected boundaries
 */
int ay_tcmd_crttoolobjtcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

/* tgui.c */

/** initialize tesselation GUI module
 */
void ay_tgui_init(Tcl_Interp *interp);


/* tmp.c */

/** Tcl command to get a temporary filename
 */
int ay_tmp_gettcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);


/* toglcb.c */

/** create Togl callback
 */
void ay_toglcb_create(struct Togl *togl);

/** destroy Togl callback
 */
void ay_toglcb_destroy(struct Togl *togl);

/** reshape Togl callback (called when window is resized or exposed)
 */
void ay_toglcb_reshape(struct Togl *togl);

/** display Togl callback
 */
void ay_toglcb_display(struct Togl *togl);


/* trafo.c */

/** apply trafo to point (3D)
 */
void ay_trafo_apply3(double *c, double *m);

/** apply trafo to vector of points (3D)
 */
void ay_trafo_apply3v(double *c, unsigned int clen, unsigned int stride,
		      double *m);

/** apply trafo to point (4D)
 */
void ay_trafo_apply4(double *c, double *m);

/** apply trafo to vector of points (4D)
 */
void ay_trafo_apply4v(double *c, unsigned int clen, unsigned int stride,
		      double *m);

/** accumulate all parent transformations
 */
void ay_trafo_getparent(ay_list_object *lo, double *tm);

/** accumulate all inverse parent transformations
 */
void ay_trafo_getparentinv(ay_list_object *lo, double *tm);

/** accumulate some parent transformations
 */
void ay_trafo_getsomeparent(ay_list_object *lo, int what, double *tm);

/** accumulate some inverse parent transformations
 */
void ay_trafo_getsomeparentinv(ay_list_object *lo, int what, double *tm);

/** concatenate all trafos onto GL matrix stack
 */
void ay_trafo_concatparent(ay_list_object *lo);

/** get object and parent transformations
 */
void ay_trafo_getall(ay_list_object *lo, ay_object *o, double *tm);

/** delegate trafos to children
 */
int ay_trafo_delegate(ay_object *o);

/** Tcl command to delegate the transformations to the children
 */
int ay_trafo_delegatetcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** apply transformations to a point (object to world)
 */
void ay_trafo_applyall(ay_list_object *lo, ay_object *o, double *p);

/** inversely apply transformations to a point (world to object)
 */
void ay_trafo_applyalli(ay_list_object *lo, ay_object *o, double *p);

/** copy transformation attributes
 */
void ay_trafo_copy(ay_object *src, ay_object *dst);

/** add transformation attributes
 */
void ay_trafo_add(ay_object *src, ay_object *dst);

/** initialize transformation attributes of object o to identity
 */
void ay_trafo_defaults(ay_object *o);

/** Tcl command to move an object or the selected points.
 */
int ay_trafo_movtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/** Tcl command to scale an object or the selected points
 */
int ay_trafo_scaltcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Tcl command to rotate an object or the selected points
 */
int ay_trafo_rottcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/** multiply two transformation matrices
 */
void ay_trafo_multmatrix(double *m1, double *m2);

/** multiply vector with matrix
 */
void ay_trafo_multvectmatrix(double *v, double *m);

/** invert transformation matrix
 */
int ay_trafo_invmatrix(double *m, double *mi);

/** invert generic matrix
 */
int ay_trafo_invgenmatrix(double *m, double *mi);

/** initialize transformation matrix from object transformation attributes
 */
void ay_trafo_creatematrix(ay_object *o, double *m);

/** decompose transformation matrix to object transformation attributes
 */
void ay_trafo_decomposematrix(double *m, ay_object *o);

/** initialize transformation matrix m to identity
 */
void ay_trafo_identitymatrix(double *m);

/** check transformation matrix m for identity
 */
int ay_trafo_isidentitymatrix(double *m);

/** add a translation to a transformation matrix
 */
void ay_trafo_translatematrix(double x, double y, double z, double *m);

/** add a scale transformation to a transformation matrix
 */
void ay_trafo_scalematrix(double x, double y, double z, double *m);

/** add a rotation to a transformation matrix
 */
void ay_trafo_rotatematrix(double angle, double x, double y, double z,
			   double *m);

/** normalize the transformation attributes of an object
 */
void ay_trafo_normalize(ay_object *o, int digits);

/** Tcl command to normalize the transformation attributes.
 */
int ay_trafo_normalizetcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

/** round a value to aspecific number of digits
 */
double ay_trafo_round(double value, int digits);


/* undo.c */

/** initialize undo system
 */
int ay_undo_init(int buffer_size);

/** clear all undo states
 */
void ay_undo_clear(void);

/** clear all undo states of object o
 */
void ay_undo_clearobj(ay_object *o);

/** Tcl command to interface to the undo system
 */
int ay_undo_undotcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);


/* vact.c */

/** move/pan view modelling action
 */
int ay_vact_movetcb(struct Togl *togl, int argc, char *argv[]);

/** zoom view modelling action
 */
int ay_vact_zoomtcb(struct Togl *togl, int argc, char *argv[]);

/** move (along z) view modelling action
 */
int ay_vact_moveztcb(struct Togl *togl, int argc, char *argv[]);


/* viewt.c */

/** set up the camera projection of a view
 */
void ay_viewt_setupprojection(struct Togl *togl);

/** rotate a view
 */
void ay_viewt_rotate(ay_view_object *view, double rotx, double roty,
		     double rotz);

/** convert window coordinates to object space
 */
void ay_viewt_wintoobj(struct Togl *togl, ay_object *o,
		       double winX, double winY,
		       double *objX, double *objY, double *objZ);

/** convert rectangle in window coordinates to object space
 */
void ay_viewt_winrecttoobj(struct Togl *togl, ay_object *o,
			   double winX, double winY,
			   double winX2, double winY2,
			   double *obj);

/** convert window coordinates to world space
 */
void ay_viewt_wintoworld(struct Togl *togl, double winX, double winY,
			 double *worldX, double *worldY, double *worldZ);

/** zoom a view to an object
 */
int ay_viewt_zoomtoobj(struct Togl *togl, int argc, char *argv[]);

/** align a view to an object
 */
int ay_viewt_align(struct Togl *togl, int argc, char *argv[]);

/** align a view
 */
void ay_viewt_alignlocal(void);

/** redraw all views
 */
void ay_viewt_redrawall(void);

/** make view (the associated GL context) current
 */
int ay_viewt_makecurtcb(struct Togl *togl, int argc, char *argv[]);

/** change the type of a view (Front->Side)
 */
void ay_viewt_changetype(ay_view_object *view, int type);

/** update view after size changed
 */
int ay_viewt_reshapetcb(struct Togl *togl, int argc, char *argv[]);

/** re-draw a view
 */
int ay_viewt_redrawtcb(struct Togl *togl, int argc, char *argv[]);

/** configure a view
 */
int ay_viewt_setconftcb(struct Togl *togl, int argc, char *argv[]);

/** calculate new mark window coordinates
 */
int ay_viewt_updatemark(struct Togl *togl, int local);

/** print new mark coordinates to console
 */
void ay_viewt_printmark(ay_view_object *view);

/** manage the global mark after change in view togl
 */
void ay_viewt_updateglobalmark(struct Togl *togl);

/** get the global mark
 */
void ay_viewt_getglobalmark(double **m);

/** copy from/to/up from the selected camera object to view togl
 */
int ay_viewt_fromcamtcb(struct Togl *togl, int argc, char *argv[]);

/** copy camera settings of view togl to the selected camera object
 */
int ay_viewt_tocamtcb(struct Togl *togl, int argc, char *argv[]);

/** update view property
 */
void ay_viewt_uprop(ay_view_object *view, int notify);

/** snap winx and winy to current grid
 */
int ay_viewt_griddify(struct Togl *togl, double *winx, double *winy);

/** an object has been dropped onto a view window
 */
int ay_viewt_droptcb(struct Togl *togl, int argc, char *argv[]);

/** setup internal view (while reading from scene file)
 */
void ay_viewt_setupintview(int viewnum, ay_object *o, ay_view_object *vtemp);

/** set mark from selected objects cog
 */
int ay_viewt_markfromsel(struct Togl *togl);

/** set mark from selected points cog
 */
int ay_viewt_markfromselp(struct Togl *togl, int mode);

/** set mark from window coordinates
 */
int ay_viewt_markfromwin(struct Togl *togl, int argc, char *argv[]);

/** save TIFF image file
 */
int ay_viewt_saveimgtcb(struct Togl *togl, int argc, char *argv[]);

/** warp mouse pointer to new position
 */
void ay_viewt_warpmouse(struct Togl *togl, double *coord, ay_object *o,
			double *newwinx, double *newwiny);

/** get rolled up vector
 */
void ay_viewt_getrolledup(ay_view_object *view, double *u);

/** project point from world to window space
 */
void ay_viewt_worldtowin(double *world, double *win);

/** read rendered image from FIFO to texture
 */
int ay_viewt_rendertoviewportcb(struct Togl *togl, int argc, char *argv[]);

/** convert object space rectangle to plane coefficients
 */
void ay_viewt_objrecttoplanes(double *rect, double *plcs);


/* w32t.c */
#ifdef WIN32

/** Tcl command to kill processes on the Win32 platform
 */
int ay_w32t_w32killtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** open a named pipe (FIFO surrogate) on the Win32 platform
 */
FILE *ay_w32t_openpipe(char *pipe);


/* As long as we do not use the return value...
 */
#define snprintf sprintf_s

#endif


/* wrib.c */

/** check, whether o should be considered a CSG primitive
 */
int ay_wrib_isprimitive(ay_object *o);

/** set up camera for RIB export
 */
void ay_wrib_aimz(RtPoint direction);

/** set up camera for RIB export
 */
void ay_wrib_placecamera(RtPoint position, RtPoint direction, double roll);

/** export the RiOptions to a RIB file
 */
void ay_wrib_rioptions(int searchpathsonly);

/** look through the scene whether there are any lights switched on
 */
int ay_wrib_checklights(ay_object *o);

/** export the lights to a RIB file
 */
int ay_wrib_alllights(char *file);

/** export the root object shaders to a RIB file
 */
int ay_wrib_rootsh(int imager);

/** export the default material to a RIB file
 */
int ay_wrib_defmat(char *file);

/** export transformations of an object to a RIB file
 */
void ay_wrib_trafos(ay_object *o);

/** export an object to a RIB file
 */
int ay_wrib_object(char *file, ay_object *o);

/** export a sub-object from a tool object to a RIB file
 */
int ay_wrib_toolobject(char *file, ay_object *o, ay_object *t);

/** export a cap or bevel from a tool object to a RIB file
 */
int ay_wrib_caporbevel(char *file, ay_object *o, ay_object *c, unsigned int ci);

/** generate instance archive file name
 */
char *ay_wrib_geniafilename(char *base, char *oi);

/** export a referenced object to a RIB file
 */
int ay_wrib_refobject(char *file, ay_object *o);

/** export the scene to a RIB file
 */
int ay_wrib_scene(char *file, char *image, char *driver, int temp, int target,
		  double *from, double *to,
		  double roll, double zoom, double nearp, double farp,
		  int width, int height, int type);

/** Togl callback for exporting the scene to a RIB file
 */
int ay_wrib_viewtcb(struct Togl *togl, int argc, char *argv[]);

/** export RiDisplay tags to a RIB file
 */
void ay_wrib_displaytags(int have_ridisplay);

/** export RiHider tags to a RIB file
 */
void ay_wrib_hidertags(void);

/** Tcl command to export the scene to a RIB file
 */
int ay_wrib_tcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[]);

#ifdef AYENABLEPPREV

/** redraw permanent preview window
 */
int ay_wrib_pprevdraw(ay_view_object *view);

/** open permanent preview window
 */
int ay_wrib_pprevopen(ay_view_object *view);

/** close permanent preview window
 */
void ay_wrib_pprevclose();
#endif

/** initialize RIB export module
 */
void ay_wrib_init(Tcl_Interp *interp);


/* write.c */

/** write the Ayam scene file format header to a scene file
 */
int ay_write_header(FILE *fileptr);

/** save object attributes to a scene file
 */
int ay_write_attributes(FILE *fileptr, ay_object *o);

/** save object tags to a scene file
 */
int ay_write_tags(FILE *fileptr, ay_object *o);

/** save a shader to a scene file
 */
void ay_write_shader(FILE *fileptr, ay_shader *shader);

/** save an object to a scene file
 */
int ay_write_object(FILE *fileptr, ay_object *o);

/** save the scene to a scene file
 */
int ay_write_scene(char *fname, int selected);

/** Tcl command to save the scene to a scene file
 */
int ay_write_scenetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/* ayam.c */

/** compare plugin compilation and Ayam runtime versions
 */
int ay_checkversion(char *fname, char *version_ma, char *version_mi);

/*! \file aycore.h \brief Ayam Core API */

#endif /* __aycore_h__ */
