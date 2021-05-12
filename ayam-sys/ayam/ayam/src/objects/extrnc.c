/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* extrnc.c - ExtrNC (extract NURBS curve) object */

static char *ay_extrnc_name = "ExtrNC";


/* prototypes of functions local to this module: */

int ay_extrnc_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);


/* functions: */

/* ay_extrnc_createcb:
 *  create callback function of extrnc object
 */
int
ay_extrnc_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtextrnc";
 ay_extrnc_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_extrnc_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->hide_children = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_extrnc_createcb */


/* ay_extrnc_deletecb:
 *  delete callback function of extrnc object
 */
int
ay_extrnc_deletecb(void *c)
{
 ay_extrnc_object *extrnc = NULL;

  if(!c)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(c);

  if(extrnc->ncurve)
    (void)ay_object_delete(extrnc->ncurve);

  free(extrnc);

 return AY_OK;
} /* ay_extrnc_deletecb */


/* ay_extrnc_copycb:
 *  copy callback function of extrnc object
 */
int
ay_extrnc_copycb(void *src, void **dst)
{
 ay_extrnc_object *extrnc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(extrnc = malloc(sizeof(ay_extrnc_object))))
    return AY_EOMEM;

  memcpy(extrnc, src, sizeof(ay_extrnc_object));

  extrnc->ncurve = NULL;

  *dst = (void *)extrnc;

 return AY_OK;
} /* ay_extrnc_copycb */


/* ay_extrnc_drawcb:
 *  draw (display in an Ayam view window) callback function of extrnc object
 */
int
ay_extrnc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_extrnc_object *extrnc = NULL;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(!extrnc)
    return AY_ENULL;

  if(extrnc->ncurve)
    ay_draw_object(togl, extrnc->ncurve, AY_TRUE);

 return AY_OK;
} /* ay_extrnc_drawcb */


/* ay_extrnc_shadecb:
 *  shade (display in an Ayam view window) callback function of extrnc object
 */
int
ay_extrnc_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_extrnc_shadecb */


/* ay_extrnc_drawacb:
 *  draw annotations (in an Ayam view window) callback function of extrnc object
 */
int
ay_extrnc_drawacb(struct Togl *togl, ay_object *o)
{
 ay_extrnc_object *extrnc = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(!extrnc)
    return AY_ENULL;

  if(extrnc->ncurve)
    {
      /* get NURBS curve */
      nc = (ay_nurbcurve_object *)extrnc->ncurve->refine;

      /* draw arrow */
      ay_draw_arrow(togl, &(nc->controlv[nc->length*4-8]),
		    &(nc->controlv[nc->length*4-4]));
    }

 return AY_OK;
} /* ay_extrnc_drawacb */


/* ay_extrnc_drawhcb:
 *  draw handles (in an Ayam view window) callback function of extrnc object
 */
int
ay_extrnc_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_extrnc_object *extrnc;
 ay_nurbcurve_object *nc;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(!extrnc)
    return AY_ENULL;

  if(extrnc->ncurve)
    {
      /* read only points => switch to blue */
      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      if(view->drawhandles == 2)
	{
	  ay_nct_drawbreakpoints(togl, extrnc->ncurve);
	}
      else
	{
	  /* get NURBS curve */
	  nc = (ay_nurbcurve_object *)extrnc->ncurve->refine;

	  /* get and draw read only points */
	  pnts = nc->controlv;

	  glBegin(GL_POINTS);
	   if(ay_prefs.rationalpoints)
	     {
	       for(i = 0; i < nc->length; i++)
		 {
		   glVertex3d((GLdouble)pnts[0]*pnts[3],
			      (GLdouble)pnts[1]*pnts[3],
			      (GLdouble)pnts[2]*pnts[3]);
		   pnts += 4;
		 }
	     }
	   else
	     {
	       for(i = 0; i < nc->length; i++)
		 {
		   glVertex3dv((GLdouble *)pnts);
		   pnts += 4;
		 }
	     }
	  glEnd();
	}

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    } /* if have ncurve */

 return AY_OK;
} /* ay_extrnc_drawhcb */


/* ay_extrnc_getpntcb:
 *  get point (editing and selection) callback function of extrnc object
 */
int
ay_extrnc_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 int ay_status = AY_ERROR;
 ay_nurbcurve_object *curve = NULL;
 ay_extrnc_object *extrnc = NULL;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(!extrnc)
    return AY_ENULL;

  if(extrnc->ncurve)
    {
      curve = (ay_nurbcurve_object *)extrnc->ncurve->refine;
      if(pe && pe->type == AY_PTKNOT && curve->breakv)
	{
	  ay_status = ay_selp_getpnts(mode, o, p, pe, 1, (int)curve->breakv[0],
				      4, AY_FALSE, &(curve->breakv[1]));
	  pe->type = AY_PTKNOT;
	}
      else
	return ay_selp_getpnts(mode, o, p, pe, 1, curve->length, 4,
			       ay_prefs.rationalpoints, curve->controlv);
    }

 return ay_status;
} /* ay_extrnc_getpntcb */


/* ay_extrnc_setpropcb:
 *  set property (from Tcl to C context) callback function of extrnc object
 */
int
ay_extrnc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *arr = "ExtrNCAttrData";
 /* char fname[] = "extrnc_setpropcb";*/
 Tcl_Obj *to = NULL;
 ay_extrnc_object *extrnc = NULL;

  if(!interp || !o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(!extrnc)
    return AY_ENULL;

  to = Tcl_GetVar2Ex(interp, arr, "Side",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(extrnc->side));

  to = Tcl_GetVar2Ex(interp, arr, "Parameter",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(extrnc->parameter));

  to = Tcl_GetVar2Ex(interp, arr, "PatchNum",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(extrnc->pnum));

  to = Tcl_GetVar2Ex(interp, arr, "Revert",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(extrnc->revert));

  to = Tcl_GetVar2Ex(interp, arr, "Extract",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(extrnc->extractnt));

  to = Tcl_GetVar2Ex(interp, arr, "Relative",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(extrnc->relative));

  to = Tcl_GetVar2Ex(interp, arr, "Tolerance",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(extrnc->glu_sampling_tolerance));

  to = Tcl_GetVar2Ex(interp, arr, "DisplayMode",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(extrnc->display_mode));

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_extrnc_setpropcb */


/* ay_extrnc_getpropcb:
 *  get property (from C to Tcl context) callback function of extrnc object
 */
int
ay_extrnc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *arr = "ExtrNCAttrData";
 ay_object *trim = NULL, *npatch = NULL, *pobject = NULL;
 ay_extrnc_object *extrnc = NULL;
 int pnum;

  if(!interp || !o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(o->refine);

  if(!extrnc)
    return AY_ENULL;

  Tcl_SetVar2Ex(interp, arr, "Side",
		Tcl_NewIntObj(extrnc->side),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Parameter",
		Tcl_NewDoubleObj(extrnc->parameter),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PatchNum",
		Tcl_NewIntObj(extrnc->pnum),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Revert",
		Tcl_NewIntObj(extrnc->revert),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Extract",
		Tcl_NewIntObj(extrnc->extractnt),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Relative",
		Tcl_NewIntObj(extrnc->relative),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tolerance",
		Tcl_NewDoubleObj(extrnc->glu_sampling_tolerance),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "DisplayMode",
		Tcl_NewIntObj(extrnc->display_mode),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ay_prop_getncinfo(interp, arr, extrnc->ncurve);

  /* parse trims for GUI selection */
  if(o->down &&  o->down->next)
    {
      pnum = extrnc->pnum - 1;
      npatch = o->down;
      if(npatch->type != AY_IDNPATCH)
	{
	  (void)ay_provide_object(npatch, AY_IDNPATCH, &pobject);
	  if(!pobject)
	    {
	      goto cleanup;
	    }
	  else
	    {
	      if(pnum > 0)
		{
		  while(pobject && pnum)
		    {
		      pobject = pobject->next;
		      pnum--;
		    }
		  if(pobject)
		    {
		      npatch = pobject;
		    }
		  else
		    {
		      /* not enough patches for pnum! */
		      /* report error? */
		      goto cleanup;
		    }
		}
	      else
		{
		  npatch = pobject;
		}
	    } /* if */
	} /* if child is not npatch */

      trim = npatch->down;
      while(trim && trim->next)
	{
	  Tcl_SetVar2Ex(interp, arr, "trims",
			Tcl_NewStringObj(ay_object_getname(trim), -1),
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY |
			TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	  trim = trim->next;
	}
    } /* if have child */

cleanup:

  /* remove provided object(s) */
  if(pobject)
    {
      (void)ay_object_deletemulti(pobject, AY_FALSE);
    }

 return AY_OK;
} /* ay_extrnc_getpropcb */


/* ay_extrnc_readcb:
 *  read (from scene file) callback function of extrnc object
 */
int
ay_extrnc_readcb(FILE *fileptr, ay_object *o)
{
 ay_extrnc_object *extrnc = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(extrnc = calloc(1, sizeof(ay_extrnc_object))))
    return AY_EOMEM;

  fscanf(fileptr, "%d\n", &extrnc->side);
  fscanf(fileptr, "%lg\n", &extrnc->parameter);
  fscanf(fileptr, "%d\n", &extrnc->display_mode);
  fscanf(fileptr, "%lg\n", &extrnc->glu_sampling_tolerance);

  if(ay_read_version >= 8)
    {
      /* since 1.9 */
      fscanf(fileptr, "%d\n", &extrnc->pnum);
    }

  if(ay_read_version >= 10)
    {
      /* since 1.13 */
      fscanf(fileptr, "%d\n", &extrnc->revert);
    }

  if(ay_read_version >= 12)
    {
      /* since 1.15 */
      fscanf(fileptr, "%d\n", &extrnc->relative);
    }

  if(ay_read_version >= 14)
    {
      /* since 1.19 */
      fscanf(fileptr, "%d\n", &extrnc->extractnt);
    }

  o->refine = extrnc;

 return AY_OK;
} /* ay_extrnc_readcb */


/* ay_extrnc_writecb:
 *  write (to scene file) callback function of extrnc object
 */
int
ay_extrnc_writecb(FILE *fileptr, ay_object *o)
{
 ay_extrnc_object *extrnc = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(o->refine);

  if(!extrnc)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", extrnc->side);
  fprintf(fileptr, "%g\n", extrnc->parameter);
  fprintf(fileptr, "%d\n", extrnc->display_mode);
  fprintf(fileptr, "%g\n", extrnc->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", extrnc->pnum);
  fprintf(fileptr, "%d\n", extrnc->revert);
  fprintf(fileptr, "%d\n", extrnc->relative);
  fprintf(fileptr, "%d\n", extrnc->extractnt);

 return AY_OK;
} /* ay_extrnc_writecb */


/* ay_extrnc_wribcb:
 *  RIB export callback function of extrnc object
 */
int
ay_extrnc_wribcb(char *file, ay_object *o)
{

 return AY_OK;
} /* ay_extrnc_wribcb */


/* ay_extrnc_bbccb:
 *  bounding box calculation callback function of extrnc object
 */
int
ay_extrnc_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_extrnc_object *extrnc = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(!extrnc)
    return AY_ENULL;

  if(extrnc->ncurve)
    {
      ay_bbc_get(extrnc->ncurve, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    } /* if */

 return AY_OK;
} /* ay_extrnc_bbccb */


/* ay_extrnc_notifycb:
 *  notification callback function of extrnc object
 */
int
ay_extrnc_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_extrnc_object *extrnc = NULL;
 ay_object *npatch = NULL, *pobject = NULL;
 ay_object *ncurve = NULL;
 ay_tag *newtag = NULL;
 ay_btval *newval = NULL;
 int i, mode, pnum, provided = AY_FALSE, stride = 3, freepvnt = AY_TRUE;
 double t[3] = {0}, *pvnt = NULL, tolerance;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(o->refine);

  if(!extrnc)
    return AY_ENULL;

  pnum = extrnc->pnum - 1;
  mode = extrnc->display_mode;
  tolerance = extrnc->glu_sampling_tolerance;

  /* remove old objects */
  if(extrnc->ncurve)
    (void)ay_object_delete(extrnc->ncurve);
  extrnc->ncurve = NULL;

  /* get patch to extract the ncurve from */
  if(!o->down)
    goto cleanup;

  if(!o->down->next)
    goto cleanup;

  npatch = o->down;
  if(npatch->type != AY_IDNPATCH || pnum > 0)
    {
      if(npatch->type == AY_IDINSTANCE)
	{
	  ay_status = ay_provide_object(npatch->refine, AY_IDNPATCH, &pobject);
	  if(pobject && (!npatch->tags || !ay_instance_hasrptrafo(npatch)))
	    ay_trafo_add(npatch, pobject);
	}
      else
	ay_status = ay_provide_object(npatch, AY_IDNPATCH, &pobject);

      if(!pobject)
	{
	  goto cleanup;
	}
      else
	{
	  if(pnum > 0)
	    {
	      while(pobject && pnum)
		{
		  pobject = pobject->next;
		  pnum--;
		}
	      if(pobject)
		{
		  npatch = pobject;
		}
	      else
		{
		  /* not enough patches for pnum! */
		  ay_status = AY_ERROR;
		  goto cleanup;
		}
	    }
	  else
	    {
	      npatch = pobject;
	    }
	  provided = AY_TRUE;
	} /* if */
    } /* if */

  /* extract the curve */
  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;

  ay_status = ay_npt_extractnc(npatch, extrnc->side, extrnc->parameter,
			       extrnc->relative, AY_FALSE, extrnc->extractnt,
			       &pvnt,
			  (ay_nurbcurve_object **)(void*)(&(ncurve->refine)));

  if(ay_status || !ncurve->refine)
    {
      goto cleanup;
    }

  if(extrnc->revert)
    {
      ay_status = ay_nct_revert((ay_nurbcurve_object *)(ncurve->refine));
    }

  if(pvnt)
    {
      if(extrnc->revert)
	{
	  (void) ay_nct_revertarr(pvnt,
			   ((ay_nurbcurve_object *)(ncurve->refine))->length,
			   extrnc->extractnt > 1 ? 9 : 3);
	}
      if(extrnc->extractnt > 1)
	{
	  if(!(newtag = calloc(1, sizeof(ay_tag))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  if(!(newval = calloc(1, sizeof(ay_btval))))
	    {
	      free(newtag);
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  newval->size =
	    ((ay_nurbcurve_object *)(ncurve->refine))->length * 9 *
	    sizeof(double);
	  newval->payload = pvnt;
	  newtag->type = ay_nt_tagtype;
	  newtag->is_binary = AY_TRUE;
	  newtag->val = newval;
	  if(extrnc->side == 2 || extrnc->side == 3 || extrnc->side == 5)
	    {
	      for(i = 0;
		  i < ((ay_nurbcurve_object *)(ncurve->refine))->length;
	          i++)
		{
		  memcpy(t, &(pvnt[i*9+6]), 3*sizeof(double));
		  memcpy(&(pvnt[i*9+6]), &(pvnt[i*9+3]), 3*sizeof(double));
		  memcpy(&(pvnt[i*9+3]), t, 3*sizeof(double));
		}
	    }
	  newtag->next = ncurve->tags;
	  ncurve->tags = newtag;
	  freepvnt = AY_FALSE;
	}
      else
	{
	  stride = 3;
	  ay_pv_add(ncurve, ay_prefs.normalname, "varying", "n",
		    ((ay_nurbcurve_object *)(ncurve->refine))->length,
		    stride, (void*)pvnt);
	}
    } /* if pvnt */

  extrnc->ncurve = ncurve;

  /* copy transformation attributes over to extrnc object
     (the extracted curve is always at the same place as
     the surface) */
  ay_trafo_copy(npatch, o);

  /* copy sampling tolerance/mode over to new object */
  ((ay_nurbcurve_object *)ncurve->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbcurve_object *)ncurve->refine)->display_mode =
    mode;

  /* prevent cleanup code from doing something harmful */
  ncurve = NULL;

cleanup:

  if(pvnt && freepvnt)
    free(pvnt);

  if(ncurve)
    {
      (void)ay_object_delete(ncurve);
    }

  /* remove provided object(s) */
  if(provided)
    {
      (void)ay_object_deletemulti(pobject, AY_FALSE);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(extrnc->ncurve)
	ay_extrnc_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_extrnc_notifycb */


/* ay_extrnc_providecb:
 *  provide callback function of extrnc object
 */
int
ay_extrnc_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "extrnc_providecb";
 ay_extrnc_object *s = NULL;
 ay_object *new = NULL, **t = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if((type == AY_IDNCURVE) || (type == UINT_MAX-AY_IDNCURVE))
	return AY_OK;
      else
	return AY_ERROR;
    } /* if */

  s = (ay_extrnc_object *) o->refine;

  if(!s)
    return AY_ENULL;

  if(type == UINT_MAX-AY_IDNCURVE)
    {
      /* peek */
      if(!s->ncurve)
	return AY_ERROR;
      *result = s->ncurve;
      return AY_OK;
    }

  if(type == AY_IDNCURVE)
    {
      t = &(new);

      if(!s->ncurve)
	return AY_ERROR;

      /* copy extrnc */
      ay_status = ay_object_copy(s->ncurve, t);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  return AY_ERROR;
	}

      ay_trafo_copy(o, *t);

      /*t = &((*t)->next);*/

      *result = new;
    } /* if */

 return ay_status;
} /* ay_extrnc_providecb */


/* ay_extrnc_convertcb:
 *  convert callback function of extrnc object
 */
int
ay_extrnc_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_extrnc_object *r = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */
  r = (ay_extrnc_object *) o->refine;

  if(!r)
    return AY_ENULL;

  if(r->ncurve)
    {
      ay_status = ay_object_copy(r->ncurve, &new);

      /* second, link new object, or replace old object with it */
      if(new)
	{
	  /* reset display mode and sampling tolerance
	     of new curve to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      nc = (ay_nurbcurve_object *)(new->refine);
	      nc->display_mode = 0;
	      nc->glu_sampling_tolerance = 0.0;
	    }

	  ay_trafo_copy(o, new);

	  if(!in_place)
	    {
	      ay_object_link(new);
	    }
	  else
	    {
	      ay_object_replace(new, o);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_extrnc_convertcb */


/* ay_extrnc_init:
 *  initialize the extrnc object module
 */
int
ay_extrnc_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_extrnc_name,
				    ay_extrnc_createcb,
				    ay_extrnc_deletecb,
				    ay_extrnc_copycb,
				    ay_extrnc_drawcb,
				    ay_extrnc_drawhcb,
				    NULL/*ay_extrnc_shadecb*/,
				    ay_extrnc_setpropcb,
				    ay_extrnc_getpropcb,
				    ay_extrnc_getpntcb,
				    ay_extrnc_readcb,
				    ay_extrnc_writecb,
				    ay_extrnc_wribcb,
				    ay_extrnc_bbccb,
				    AY_IDEXTRNC);

  ay_status += ay_draw_registerdacb(ay_extrnc_drawacb, AY_IDEXTRNC);

  ay_status += ay_notify_register(ay_extrnc_notifycb, AY_IDEXTRNC);

  ay_status += ay_convert_register(ay_extrnc_convertcb, AY_IDEXTRNC);

  ay_status += ay_provide_register(ay_extrnc_providecb, AY_IDEXTRNC);

  /* extrnc objects may not be associated with materials */
  ay_matt_nomaterial(AY_IDEXTRNC);

 return ay_status;
} /* ay_extrnc_init */

