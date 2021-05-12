/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* pamesh.c - PatchMesh object */

static char *ay_pamesh_name = "PatchMesh";
static char *ay_pamesh_arr = "PatchMeshAttrData";

static Tcl_Obj *arrobj = NULL;
static Tcl_Obj *bmu[16] = {0};
static Tcl_Obj *bmv[16] = {0};

/* prototypes of functions local to this module: */

int ay_pamesh_notifycb(ay_object *o);

void ay_pamesh_drawweights(ay_pamesh_object *pamesh);


/* functions: */

/* ay_pamesh_createcb:
 *  create callback function of pamesh object
 */
int
ay_pamesh_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtpamesh";
 char option_handled = AY_FALSE;
 int center = AY_FALSE, stride = 4, width = 4, height = 4;
 int clu = 0, clv = 0, ustep = 3, vstep = 3;
 int type = AY_PTBICUBIC, ubt = AY_BTBEZIER, vbt = AY_BTBEZIER;
 int optnum = 0, i = 2, j = 0, k = 0;
 int acvlen = 0, aubvlen = 0, avbvlen = 0;
 char **acv = NULL, **abv = NULL;
 double *cv = NULL, *ubv = NULL, *vbv = NULL;
 double udx = 0.25, udy = 0.0, udz = 0.0;
 double vdx = 0.0, vdy = 0.25, vdz = 0.0;
 double ext = 0.0, s[3] = {0};
 ay_pamesh_object *p = NULL;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i < argc)
    {
      if(i+1 >= argc)
	{
	  ay_error(AY_EOPT, fname, argv[i]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      tcl_status = TCL_OK;
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'w':
	      /* -width */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &width);
	      option_handled = AY_TRUE;
	      break;
	    case 'h':
	      /* -height */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &height);
	      option_handled = AY_TRUE;
	      break;
	    case 'u':
	      switch(argv[i][2])
		{
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -udx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -udy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -udz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'b':
		  switch(argv[i][3])
		    {
		    case 'a':
		      /* -ubasis */
		      if(Tcl_SplitList(ay_interp, argv[i+1], &aubvlen, &abv) ==
			 TCL_OK)
			{
			  if(ubv)
			    {
			      free(ubv);
			    }
			  if(!(ubv = calloc(aubvlen, sizeof(double))))
			    {
			      Tcl_Free((char *) abv);
			      ay_status = AY_EOMEM;
			      goto cleanup;
			    }
			  for(j = 0; j < aubvlen; j++)
			    {
			      tcl_status = Tcl_GetDouble(ay_interp,
							 abv[j], &ubv[j]);
			      if(tcl_status != TCL_OK)
				{
				  break;
				}
			    } /* for */
			  Tcl_Free((char *) abv);
			}
		      option_handled = AY_TRUE;
		      break;
		    case 't':
		      /* -ubtype */
		      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &ubt);
		      option_handled = AY_TRUE;
		      break;
		    case 'n':
		      /* -ubn */
		      if(ubv)
			{
			  free(ubv);
			  ubv = NULL;
			}
		      tcl_status = ay_tcmd_convdlist(argv[i+1], &aubvlen, &ubv);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 's':
		  /* -ustep */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &ustep);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'v':
	      switch(argv[i][2])
		{
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -vdx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -vdy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -vdz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'b':
		  switch(argv[i][3])
		    {
		    case 'a':
		      /* -vbasis */
		      if(Tcl_SplitList(ay_interp, argv[i+1], &avbvlen, &abv) ==
			 TCL_OK)
			{
			  if(vbv)
			    {
			      free(vbv);
			    }
			  if(!(vbv = calloc(avbvlen, sizeof(double))))
			    {
			      Tcl_Free((char *) abv);
			      ay_status = AY_EOMEM;
			      goto cleanup;
			    }
			  for(j = 0; j < avbvlen; j++)
			    {
			      tcl_status = Tcl_GetDouble(ay_interp,
							 abv[j], &vbv[j]);
			      if(tcl_status != TCL_OK)
				{
				  break;
				}
			    } /* for */
			  Tcl_Free((char *) abv);
			}
		      option_handled = AY_TRUE;
		      break;
		    case 't':
		      /* -vbtype */
		      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vbt);
		      option_handled = AY_TRUE;
		      break;
		    case 'n':
		      /* -vbn */
		      if(vbv)
			{
			  free(vbv);
			  vbv = NULL;
			}
		      tcl_status = ay_tcmd_convdlist(argv[i+1], &avbvlen, &vbv);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 's':
		  /* -vstep */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vstep);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 't':
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &type);
	      option_handled = AY_TRUE;
	      break;
	    case 'c':
	      switch(argv[i][2])
		{
		case 'n':
		  /* -cn */
		  if(cv)
		    {
		      free(cv);
		      cv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &acvlen, &cv);
		  option_handled = AY_TRUE;
		  break;
		case 'v':
		  /* -cv */
		  if(Tcl_SplitList(ay_interp, argv[i+1], &acvlen, &acv) ==
		     TCL_OK)
		    {
		      if(cv)
			{
			  free(cv);
			}
		      if(!(cv = calloc(acvlen, sizeof(double))))
			{
			  Tcl_Free((char *) acv);
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      for(j = 0; j < acvlen; j++)
			{
			  tcl_status = Tcl_GetDouble(ay_interp,
						     acv[j], &cv[j]);
			  if(tcl_status != TCL_OK)
			    {
			      break;
			    }
			} /* for */
		      Tcl_Free((char *) acv);
		    }
		  option_handled = AY_TRUE;
		  break;
		case 'e':
		  /* -center */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &center);
		  option_handled = AY_TRUE;
		  break;
		case 'l':
		  if(!strcmp(argv[i], "-closeu"))
		    {
		      /* -closeu */
		      tcl_status = Tcl_GetBoolean(ay_interp, argv[i+1], &clu);
		      option_handled = AY_TRUE;
		    }
		  if(!strcmp(argv[i], "-closev"))
		    {
		      /* -closev */
		      tcl_status = Tcl_GetBoolean(ay_interp, argv[i+1], &clv);
		      option_handled = AY_TRUE;
		    }
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    default:
	      break;
	    } /* switch */

	  if(option_handled && (tcl_status != TCL_OK))
	    {
	      ay_error(AY_EOPT, fname, argv[i]);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }

	  i += 2;
	}
      else
	{
	  i++;
	} /* if */

      if(!option_handled)
	{
	  ay_error(AY_EUOPT, fname, argv[optnum]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

    } /* while */

  if(type < 0 || type > 1)
    type = AY_PTBICUBIC;

  if(type == AY_PTBILINEAR)
    {
      if(width <= 1)
	{
	  width = 2;
	}

      if(height <= 1)
	{
	  height = 2;
	}
    }
  else
    {
      if(width < 4)
	{
	  width = 4;
	}
      if(height < 4)
	{
	  height = 4;
	}
    }

  if(ustep < 0 || ustep > 4)
    ustep = 3;

  if(vstep < 0 || vstep > 4)
    vstep = 3;

  if(cv)
    {
      /* check length of user provided control point array */
      if(acvlen/stride < width*height)
	{
	  if(acvlen > 0)
	    s[0] = cv[0];
	  if(acvlen > 1)
	    s[1] = cv[1];
	  if(acvlen > 2)
	    s[2] = cv[2];

	  free(cv);
	  cv = NULL;
	  center = AY_FALSE;
	}
    }
  else
    {
      if(!(cv = calloc(width*height*stride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      if(center)
	{
	  if(fabs(udx) > AY_EPSILON)
	    ext = (width-1)*udx;
	  if(fabs(vdx) > AY_EPSILON)
	    ext += (height-1)*vdx;
	  s[0] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udy) > AY_EPSILON)
	    ext = (width-1)*udy;
	  if(fabs(vdy) > AY_EPSILON)
	    ext += (height-1)*vdy;
	  s[1] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udz) > AY_EPSILON)
	    ext = (width-1)*udz;
	  if(fabs(vdz) > AY_EPSILON)
	    ext += (height-1)*vdz;
	  s[2] = -(ext/2.0);
	}

      k = 0;
      for(i = 0; i < width; i++)
	{
	  for(j = 0; j < height; j++)
	    {
	      cv[k]   = s[0] + (double)j*vdx;
	      cv[k+1] = s[1] + (double)j*vdy;
	      cv[k+2] = s[2] + (double)j*vdz;
	      cv[k+3] = 1.0;
	      k += stride;
	    }
	  s[0] += udx;
	  s[1] += udy;
	  s[2] += udz;
	}
    } /* if */

  if(ubv)
    {
      if(0/*ay_pmt_checkbasis(aubvlen, ubv)*/)
	{
	  /* basis check failed,
	     discard user delivered basis and
	     switch back knot type to AY_BTBEZIER */
	  free(ubv);
	  ubv = NULL;
	  if(ubt == AY_BTCUSTOM)
	    {
	      ubt = AY_BTBEZIER;
	    }
	}
      else
	{
	  /* basis check ok,
	     since the user delivered an own basis he probably wants the
	     knot type set to AY_BTCUSTOM in any case */
	  ubt = AY_BTCUSTOM;
	}
    }

  if(vbv)
    {
      if(0/*ay_pmt_checkbasis(avbvlen, vbv)*/)
	{
	  /* basis check failed,
	     discard user delivered basis and
	     switch back knot type to AY_BTBEZIER */
	  free(vbv);
	  vbv = NULL;
	  if(vbt == AY_BTCUSTOM)
	    {
	      vbt = AY_BTBEZIER;
	    }
	}
      else
	{
	  /* basis check ok,
	     since the user delivered an own basis he probably wants the
	     basis type set to AY_BTCUSTOM in any case */
	  vbt = AY_BTCUSTOM;
	}
    }

  if(ubt < 0 || ubt > 5 || (ubt == AY_BTCUSTOM && !ubv))
    {
      ubt = AY_BTBEZIER;
    }

  if(vbt < 0 || vbt > 5 || (vbt == AY_BTCUSTOM && !vbv))
    {
      vbt = AY_BTBEZIER;
    }

  if(!(p = calloc(1, sizeof(ay_pamesh_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  p->type = type;

  p->width = width;
  p->height = height;

  p->close_u = clu;
  p->close_v = clv;

  p->controlv = cv;

  p->btype_u = ubt;
  p->btype_v = vbt;

  p->ubasis = ubv;
  p->vbasis = vbv;
  p->ustep = ustep;
  p->vstep = vstep;

  o->refine = p;

  (void)ay_pamesh_notifycb(o);

  /* prevent cleanup code from doing something harmful */
  cv = NULL;
  ubv = NULL;
  vbv = NULL;

cleanup:

  if(cv)
    free(cv);

  if(ubv)
    free(ubv);

  if(vbv)
    free(vbv);

 return ay_status;
} /* ay_pamesh_createcb */


/* ay_pamesh_deletecb:
 *  delete callback function of pamesh object
 */
int
ay_pamesh_deletecb(void *c)
{
 ay_pamesh_object *pamesh;

  if(!c)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)(c);

  /* free controlv */
  if(pamesh->controlv)
    free(pamesh->controlv);

  /* free ubasis */
  if(pamesh->ubasis)
    free(pamesh->ubasis);

  /* free vbasis */
  if(pamesh->vbasis)
    free(pamesh->vbasis);

  /* free NURBS patch(es) */
  if(pamesh->npatch)
    (void)ay_object_delete(pamesh->npatch);

  if(pamesh->caps_and_bevels)
    (void)ay_object_deletemulti(pamesh->caps_and_bevels, AY_FALSE);

  free(pamesh);

 return AY_OK;
} /* ay_pamesh_deletecb */


/* ay_pamesh_copycb:
 *  copy callback function of pamesh object
 */
int
ay_pamesh_copycb(void *src, void **dst)
{
 ay_pamesh_object *pamesh, *pameshsrc;

  if(!src || !dst)
    return AY_ENULL;

  pameshsrc = (ay_pamesh_object *)src;

  if(!(pamesh = malloc(sizeof(ay_pamesh_object))))
    return AY_EOMEM;

  memcpy(pamesh, src, sizeof(ay_pamesh_object));

  pamesh->controlv = NULL;
  pamesh->ubasis = NULL;
  pamesh->vbasis = NULL;

  /* copy controlv */
  if(!(pamesh->controlv = malloc(4 * pamesh->width * pamesh->height *
				 sizeof(double))))
    {
      free(pamesh);
      return AY_EOMEM;
    }
  memcpy(pamesh->controlv, pameshsrc->controlv,
	 4 * pamesh->width * pamesh->height * sizeof(double));

  /* copy ubasis */
  if(pameshsrc->ubasis)
    {
      if(!(pamesh->ubasis = malloc(16 * sizeof(double))))
	{
	  free(pamesh->controlv);
	  free(pamesh);
	  return AY_EOMEM;
	}
      memcpy(pamesh->ubasis, pameshsrc->ubasis, 16 * sizeof(double));
    }

  /* copy vbasis */
  if(pameshsrc->vbasis)
    {
      if(!(pamesh->vbasis = malloc(16 * sizeof(double))))
	{
	  if(pamesh->ubasis)
	    free(pamesh->ubasis);
	  free(pamesh->controlv);
	  free(pamesh);
	  return AY_EOMEM;
	}
      memcpy(pamesh->vbasis, pameshsrc->vbasis, 16 * sizeof(double));
    }

  pamesh->npatch = NULL;
  pamesh->caps_and_bevels = NULL;

  *dst = (void *)pamesh;

 return AY_OK;
} /* ay_pamesh_copycb */


/* ay_pamesh_drawcpcb:
 *  internal helper function
 *  draw the control polygon
 */
int
ay_pamesh_drawcpcb(struct Togl *togl, ay_object *o)
{
 ay_pamesh_object *pamesh;
 double *cv;
 int i, j, a, width, height;
 GLenum uprim = GL_LINE_STRIP, vprim = GL_LINE_STRIP;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

  width = pamesh->width;
  height = pamesh->height;

  if(pamesh->close_u)
    uprim = GL_LINE_LOOP;

  if(pamesh->close_v)
    vprim = GL_LINE_LOOP;

  cv = pamesh->controlv;

  a = 0;
  if(pamesh->is_rat && ay_prefs.rationalpoints)
    {
      for(i = 0; i < width; i++)
	{
	  glBegin(uprim);
	   for(j = 0; j < height; j++)
	     {
	       glVertex3d((GLdouble )(cv[a]*cv[a+3]),
			  (GLdouble )(cv[a+1]*cv[a+3]),
			  (GLdouble )(cv[a+2]*cv[a+3]));
	       a += 4;
	     } /* for */
	  glEnd();
	} /* for */

      for(j = 0; j < height; j++)
	{
	  a = j * 4;
	  glBegin(vprim);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3d((GLdouble )(cv[a]*cv[a+3]),
			  (GLdouble )(cv[a+1]*cv[a+3]),
			  (GLdouble )(cv[a+2]*cv[a+3]));
	       a += (4 * height);
	     } /* for */
	  glEnd();
	} /* for */
     }
   else
     {
       for(i = 0; i < width; i++)
	 {
	   glBegin(uprim);
	    for(j = 0; j < height; j++)
	      {
		glVertex3dv((GLdouble *)&cv[a]);
		a += 4;
	      } /* for */
	   glEnd();
	 } /* for */


       for(j = 0; j < height; j++)
	 {
	   a = j * 4;
	   glBegin(vprim);
	    for(i = 0; i < width; i++)
	      {
		glVertex3dv((GLdouble *)&cv[a]);
		a += (4 * height);
	      } /* for */
	   glEnd();
	 } /* for */
     } /* if homogeneous */

 return AY_OK;
} /* ay_pamesh_drawcpcb */


/* ay_pamesh_drawcb:
 *  draw (display in an Ayam view window) callback function of pamesh object
 */
int
ay_pamesh_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.np_display_mode;
 ay_pamesh_object *pamesh = NULL;
 ay_object *p = NULL;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

  if((pamesh->display_mode != 0) && !view->action_state)
    {
      display_mode = pamesh->display_mode-1;
    }

  /* draw just the control polygon? */
  if(display_mode == 0)
    {
      /* Yes */
      ay_pamesh_drawcpcb(togl, o);
    }
  else
    {
      /* No, draw the NURBS patch, if present */
      if(pamesh->npatch)
	{
	  ay_draw_object(togl, pamesh->npatch, AY_FALSE);
	} /* if */
    } /* if */

  if(pamesh->caps_and_bevels)
    {
      p = pamesh->caps_and_bevels;
      while(p)
	{
	  ay_draw_object(togl, p, AY_TRUE);
	  p = p->next;
	}
    }

 return AY_OK;
} /* ay_pamesh_drawcb */


/* ay_pamesh_shadecb:
 *  shade (display in an Ayam view window) callback function of pamesh object
 */
int
ay_pamesh_shadecb(struct Togl *togl, ay_object *o)
{
 ay_pamesh_object *pamesh;
 ay_object *p = NULL;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

  if(pamesh->npatch)
    {
      ay_shade_object(togl, pamesh->npatch, AY_FALSE);
    }

  if(pamesh->caps_and_bevels)
    {
      p = pamesh->caps_and_bevels;
      while(p)
	{
	  ay_shade_object(togl, p, AY_FALSE);
	  p = p->next;
	}
    }

 return AY_OK;
} /* ay_pamesh_shadecb */


/* ay_pamesh_drawacb:
 *  draw annotations (in an Ayam view window) callback function of pamesh object
 */
int
ay_pamesh_drawacb(struct Togl *togl, ay_object *o)
{
 int width = 0, height = 0;
 ay_pamesh_object *pm;
 GLdouble *cv;

  if(!o)
    return AY_ENULL;

  pm = (ay_pamesh_object *)o->refine;

  if(!pm)
    return AY_ENULL;

  width = pm->width;
  height = pm->height;

  cv = pm->controlv;

  ay_draw_arrow(togl, &(cv[width*height*4-8]), &(cv[width*height*4-4]));

 return AY_OK;
} /* ay_pamesh_drawacb */


/* ay_pamesh_drawweights:
 * helper for ay_pamesh_drawhcb() below,
 * draw colored handles based on their weight values
 */
void
ay_pamesh_drawweights(ay_pamesh_object *pamesh)
{
 int i;
 double w, *pnts;

  pnts = pamesh->controlv;

  /* draw normal points */
  glBegin(GL_POINTS);
   if(pamesh->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < pamesh->width*pamesh->height; i++)
	 {
	   w = pnts[3];
	   ay_nct_colorfromweight(w);
	   glVertex3d((GLdouble)(pnts[0]*w),
		      (GLdouble)(pnts[1]*w),
		      (GLdouble)(pnts[2]*w));
	   pnts += 4;
	 }
     }
   else
     {
       for(i = 0; i < pamesh->width*pamesh->height; i++)
	 {
	   ay_nct_colorfromweight(pnts[3]);
	   glVertex3dv((GLdouble *)pnts);
	   pnts += 4;
	 }
     }
  glEnd();

  glColor3ub(255,255,255);

 return;
} /* ay_pamesh_drawweights */


/* ay_pamesh_drawhcb:
 *  draw handles (in an Ayam view window) callback function of pamesh object
 */
int
ay_pamesh_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_pamesh_object *pm;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double *pnts;
 int width, height, i;

  if(!o)
    return AY_ENULL;

  pm = (ay_pamesh_object *) o->refine;

  if(!pm)
    return AY_ENULL;

  if(view->drawhandles == 3)
    {
      ay_pamesh_drawweights(pm);
      return AY_OK;
    }

  width = pm->width;
  height = pm->height;

  pnts = pm->controlv;

  glBegin(GL_POINTS);
   if(pm->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < pm->width * pm->height; i++)
	 {
	   glVertex3d((GLdouble)pnts[0]*pnts[3],
		      (GLdouble)pnts[1]*pnts[3],
		      (GLdouble)pnts[2]*pnts[3]);
	   pnts += 4;
	 }
     }
   else
     {
       for(i = 0; i < width * height; i++)
	 {
	   glVertex3dv((GLdouble *)pnts);
	   pnts += 4;
	 }
     }
  glEnd();

 return AY_OK;
} /* ay_pamesh_drawhcb */


/* ay_pamesh_getpntcb:
 *  get point (editing and selection) callback function of pamesh object
 */
int
ay_pamesh_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_pamesh_object *pamesh = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp = NULL;
 double *control = NULL, *c, h[3];
 int i = 0, j = 0, a = 0, found = AY_FALSE;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)(o->refine);

  if(!pamesh)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->type = AY_PTRAT;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = malloc(pamesh->width * pamesh->height*
			       sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < pamesh->width * pamesh->height; i++)
	{
	  pe->coords[i] = &(pamesh->controlv[a]);
	  a += 4;
	}

      pe->num = pamesh->width * pamesh->height;
      break;
    case 1:
      /* selection based on a single point? */
      control = pamesh->controlv;

      for(i = 0; i < pamesh->width * pamesh->height; i++)
	{
	  if(pamesh->is_rat && ay_prefs.rationalpoints)
	    {
	      dist = AY_VLEN((p[0] - (control[j]*control[j+3])),
			     (p[1] - (control[j+1]*control[j+3])),
			     (p[2] - (control[j+2]*control[j+3])));
	    }
	  else
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));
	    }
	  if(dist < min_dist)
	    {
	      pecoord = &(control[j]);
	      min_dist = dist;
	    }

	  j += 4;
	}

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!found)
	{

	  if(!(pe->coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  pe->coords[0] = pecoord;
	  pe->num = 1;
	}
      break;
    case 2:
      /* selection based on planes */
      control = pamesh->controlv;
      j = 0;
      a = 0;
      if(pamesh->is_rat && ay_prefs.rationalpoints)
	{
	  c = h;
	}
      for(i = 0; i < pamesh->width * pamesh->height; i++)
	{
	  if(pamesh->is_rat && ay_prefs.rationalpoints)
	    {
	      h[0] = control[j]*control[j+3];
	      h[1] = control[j+1]*control[j+3];
	      h[2] = control[j+2]*control[j+3];
	    }
	  else
	    {
	      c = &(control[j]);
	    }

	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {
	      if(!(ctmp = realloc(pe->coords, (a+1)*sizeof(double *))))
		return AY_EOMEM;
	      pe->coords = ctmp;

	      pe->coords[a] = &(control[j]);
	      a++;
	    } /* if */

	  j += 4;
	} /* for */

      pe->num = a;
      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)(pamesh->width * pamesh->height))
	    {
	      pnt->point = &(pamesh->controlv[pnt->index*4]);
	      lastpnt = &(pnt->next);
	      pnt = pnt->next;
	    }
	  else
	    {
	      *lastpnt = pnt->next;
	      free(pnt);
	      pnt = *lastpnt;
	    }
	} /* while */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_pamesh_getpntcb */


/* ay_pamesh_setpropcb:
 *  set property (from Tcl to C context) callback function of pamesh object
 */
int
ay_pamesh_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *arr = "PatchMeshAttrData";
 char fname[] = "pamesh_setpropcb";
 Tcl_Obj *to = NULL;
 ay_pamesh_object *pamesh = NULL;
 int new_close_u, new_width, new_btype_u, new_step_u;
 int new_close_v, new_height, new_btype_v, new_step_v;
 int new_type, i, j, update = AY_FALSE;
 double dtemp, *basis;

  if(!interp || !o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

  to = Tcl_GetVar2Ex(interp, arr, "Type",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_type));

  to = Tcl_GetVar2Ex(interp, arr, "Width",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_width));

  to = Tcl_GetVar2Ex(interp, arr, "Height",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_height));

  to = Tcl_GetVar2Ex(interp, arr, "Close_U",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_close_u));

  to = Tcl_GetVar2Ex(interp, arr, "Close_V",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_close_v));

  to = Tcl_GetVar2Ex(interp, arr, "BType_U",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_btype_u));

  to = Tcl_GetVar2Ex(interp, arr, "BType_V",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_btype_v));

  if((pamesh->width != new_width)||(pamesh->height != new_height)||
     (pamesh->btype_u != new_btype_u)||(pamesh->btype_v != new_btype_v)||
     (pamesh->close_u != new_close_u)||(pamesh->close_v != new_close_v)||
     (pamesh->type != new_type))
    {
      update = AY_TRUE;
    }

  if(new_btype_u == AY_BTCUSTOM)
    {
      update = AY_TRUE;
      if(!pamesh->ubasis)
	{
	  if(!(pamesh->ubasis = calloc(16, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      goto cleanup;
	    } /* if */
	} /* if */
      if(pamesh->btype_u == AY_BTCUSTOM)
	{
	  for(j = 0; j < 16; j++)
	    {
	      to = Tcl_ObjGetVar2(interp, arrobj, bmu[j], TCL_LEAVE_ERR_MSG |
				  TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      pamesh->ubasis[j] = dtemp;
	    } /* for */
	}
      else
	{
	  /* switching from another basis to custom */
	  basis = NULL;
	  ay_pmt_getbasis(pamesh->btype_u, &basis);
	  if(basis)
	    {
	      memcpy(pamesh->ubasis, basis, 16*sizeof(double));
	    }
	}

      to = Tcl_GetVar2Ex(interp, arr, "Step_U", TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &(new_step_u));
      if(new_step_u <= 0 || new_step_u > 4)
	pamesh->ustep = 1;
      else
	pamesh->ustep = new_step_u;
    }
  else
    {
      if(pamesh->ubasis)
	free(pamesh->ubasis);
      pamesh->ubasis = NULL;
    } /* if */

  if(new_btype_v == AY_BTCUSTOM)
    {
      update = AY_TRUE;
      if(!pamesh->vbasis)
	{
	  if(!(pamesh->vbasis = calloc(16, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      goto cleanup;
	    } /* if */
	} /* if */
      if(pamesh->btype_v == AY_BTCUSTOM)
	{
	  for(j = 0; j < 16; j++)
	    {
	      to = Tcl_ObjGetVar2(interp, arrobj, bmv[j], TCL_LEAVE_ERR_MSG |
				  TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      pamesh->vbasis[j] = dtemp;
	    } /* for */
	}
      else
	{
	  /* switching from another basis to custom */
	  basis = NULL;
	  ay_pmt_getbasis(pamesh->btype_v, &basis);
	  if(basis)
	    {
	      memcpy(pamesh->vbasis, basis, 16*sizeof(double));
	    }
	}

      to = Tcl_GetVar2Ex(interp, arr, "Step_V", TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &(new_step_v));
      if(new_step_v <= 0 || new_step_v > 4)
	pamesh->vstep = 1;
      else
	pamesh->vstep = new_step_v;
    }
  else
    {
      if(pamesh->vbasis)
	free(pamesh->vbasis);
      pamesh->vbasis = NULL;
    } /* if */

  pamesh->type = new_type;
  pamesh->btype_u = new_btype_u;
  pamesh->btype_v = new_btype_v;

  pamesh->close_u = new_close_u;
  pamesh->close_v = new_close_v;

  to = Tcl_GetVar2Ex(interp, arr, "Tolerance",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(pamesh->glu_sampling_tolerance));

  to = Tcl_GetVar2Ex(interp, arr, "DisplayMode",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(pamesh->display_mode));

  to = Tcl_GetVar2Ex(interp, arr, "BevelsChanged",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(to)
    {
      Tcl_GetIntFromObj(interp, to, &i);
      if(i)
	{
	  update = AY_TRUE;
	  to = Tcl_NewIntObj(0);
	  Tcl_SetVar2Ex(interp, arr, "BevelsChanged", to,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	}
    }

  to = Tcl_GetVar2Ex(interp, arr, "CapsChanged",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(to)
    {
      Tcl_GetIntFromObj(interp, to, &i);
      if(i)
	{
	  update = AY_TRUE;
	  to = Tcl_NewIntObj(0);
	  Tcl_SetVar2Ex(interp, arr, "CapsChanged", to,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	}
    }

  /* apply changed values to patch */

  /* resize patch */
  if(new_width != pamesh->width && (new_width > 1))
    {
      ay_status = ay_npt_resizearrayw(&(pamesh->controlv), 4,
				      pamesh->width, pamesh->height,
				      new_width);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not resize patch!");
	}
      else
	{
	  pamesh->width = new_width;
	  if(o->selp)
	    {
	      (void)ay_pamesh_getpntcb(3, o, NULL, NULL);
	    }
	}
    } /* if */

  if(new_height != pamesh->height && (new_height > 1))
    {
      ay_status = ay_npt_resizearrayh(&(pamesh->controlv), 4,
				      pamesh->width, pamesh->height,
				      new_height);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not resize patch!");
	}
      else
	{
	  pamesh->height = new_height;
	  if(o->selp)
	    {
	      (void)ay_pamesh_getpntcb(3, o, NULL, NULL);
	    }
	}
    } /* if */

  if(update)
    {
      (void)ay_notify_object(o);

      o->modified = AY_TRUE;
      (void)ay_notify_parent();
    }

cleanup:

 return AY_OK;
} /* ay_pamesh_setpropcb */


/* ay_pamesh_getpropcb:
 *  get property (from C to Tcl context) callback function of pamesh object
 */
int
ay_pamesh_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *arr = "PatchMeshAttrData";
 ay_pamesh_object *pamesh = NULL;
 Tcl_Obj *to = NULL;
 int j;

  if(!interp || !o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)(o->refine);

  if(!pamesh)
    return AY_ENULL;

  Tcl_SetVar2Ex(interp, arr, "Type",
		Tcl_NewIntObj(pamesh->type),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Width",
		Tcl_NewIntObj(pamesh->width),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Height",
		Tcl_NewIntObj(pamesh->height),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Close_U",
		Tcl_NewIntObj(pamesh->close_u),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Close_V",
		Tcl_NewIntObj(pamesh->close_v),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "BType_U",
		Tcl_NewIntObj(pamesh->btype_u),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "BType_V",
		Tcl_NewIntObj(pamesh->btype_v),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      if(pamesh->ubasis)
	{
	  for(j = 0; j < 16; j++)
	    {
	      to = Tcl_NewDoubleObj(pamesh->ubasis[j]);
	      Tcl_ObjSetVar2(interp, arrobj, bmu[j], to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	    } /* for */
	} /* if */

      Tcl_SetVar2Ex(interp, arr, "Step_U",
		    Tcl_NewIntObj(pamesh->ustep),
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    } /* if */

  if(pamesh->btype_v == AY_BTCUSTOM)
    {
      if(pamesh->vbasis)
	{
	  for(j = 0; j < 16; j++)
	    {
	      to = Tcl_NewDoubleObj(pamesh->vbasis[j]);
	      Tcl_ObjSetVar2(interp, arrobj, bmv[j], to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	    } /* for */
	} /* if */

      Tcl_SetVar2Ex(interp, arr, "Step_V",
		    Tcl_NewIntObj(pamesh->vstep),
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    } /* if */

  if(pamesh->is_rat)
    to = Tcl_NewStringObj("yes", 3);
  else
    to = Tcl_NewStringObj("no", 2);
  Tcl_SetVar2Ex(interp, arr, "IsRat", to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tolerance",
		Tcl_NewDoubleObj(pamesh->glu_sampling_tolerance),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "DisplayMode",
		Tcl_NewIntObj(pamesh->display_mode),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return AY_OK;
} /* ay_pamesh_getpropcb */


/* ay_pamesh_readcb:
 *  read (from scene file) callback function of pamesh object
 */
int
ay_pamesh_readcb(FILE *fileptr, ay_object *o)
{
 ay_pamesh_object *pamesh;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(pamesh = calloc(1, sizeof(ay_pamesh_object))))
    return AY_EOMEM;

  fscanf(fileptr, "%d\n", &pamesh->width);
  fscanf(fileptr, "%d\n", &pamesh->height);
  fscanf(fileptr, "%d\n", &pamesh->close_u);
  fscanf(fileptr, "%d\n", &pamesh->close_v);
  fscanf(fileptr, "%d\n", &pamesh->type);
  fscanf(fileptr, "%d\n", &pamesh->btype_u);
  fscanf(fileptr, "%d\n", &pamesh->btype_v);
  fscanf(fileptr, "%d\n", &pamesh->ustep);
  fscanf(fileptr, "%d\n", &pamesh->vstep);

  if(ay_read_version < 15)
    {
      /* Prior to Ayam 1.21: */
      if(pamesh->btype_u == 4)
	pamesh->btype_u = AY_BTCUSTOM;
      if(pamesh->btype_v == 4)
	pamesh->btype_v = AY_BTCUSTOM;
    }

  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      if(!(pamesh->ubasis = calloc(16, sizeof(double))))
	{
	  free(pamesh);
	  return AY_EOMEM;
	}
      for(i = 0; i < 16; i++)
	{
	  fscanf(fileptr, "%lg\n", &(pamesh->ubasis[i]));
	}
    }

  if(pamesh->btype_v == AY_BTCUSTOM)
    {
      if(!(pamesh->vbasis = calloc(16, sizeof(double))))
	{
	  if(pamesh->ubasis)
	    free(pamesh->ubasis);
	  free(pamesh);
	  return AY_EOMEM;
	}
      for(i = 0; i < 16; i++)
	{
	  fscanf(fileptr, "%lg\n", &(pamesh->vbasis[i]));
	}
    }

  if(!(pamesh->controlv = calloc(pamesh->width*pamesh->height*4,
				 sizeof(double))))
    {
      if(pamesh->ubasis)
	free(pamesh->ubasis);
      if(pamesh->vbasis)
	free(pamesh->vbasis);
      free(pamesh);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < pamesh->width * pamesh->height; i++)
    {
      fscanf(fileptr, "%lg %lg %lg %lg\n", &(pamesh->controlv[a]),
	     &(pamesh->controlv[a+1]),
	     &(pamesh->controlv[a+2]),
	     &(pamesh->controlv[a+3]));
      a += 4;
    }

  fscanf(fileptr, "%lg\n", &(pamesh->glu_sampling_tolerance));
  fscanf(fileptr, "%d\n", &(pamesh->display_mode));

  pamesh->is_rat = ay_pmt_israt(pamesh);

  /* Prior to 1.19 Ayam used pre-multiplied rational coordinates... */
  if(pamesh->is_rat && (ay_read_version < 14))
    {
      a = 0;
      for(i = 0; i < pamesh->width*pamesh->height; i++)
	{
	  pamesh->controlv[a]   /= pamesh->controlv[a+3];
	  pamesh->controlv[a+1] /= pamesh->controlv[a+3];
	  pamesh->controlv[a+2] /= pamesh->controlv[a+3];
	  a += 4;
	}
    }

  o->refine = pamesh;

 return AY_OK;
} /* ay_pamesh_readcb */


/* ay_pamesh_writecb:
 *  write (to scene file) callback function of pamesh object
 */
int
ay_pamesh_writecb(FILE *fileptr, ay_object *o)
{
 ay_pamesh_object *pamesh;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)(o->refine);

  if(!pamesh)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", pamesh->width);
  fprintf(fileptr, "%d\n", pamesh->height);
  fprintf(fileptr, "%d\n", pamesh->close_u);
  fprintf(fileptr, "%d\n", pamesh->close_v);
  fprintf(fileptr, "%d\n", pamesh->type);
  fprintf(fileptr, "%d\n", pamesh->btype_u);
  fprintf(fileptr, "%d\n", pamesh->btype_v);
  fprintf(fileptr, "%d\n", pamesh->ustep);
  fprintf(fileptr, "%d\n", pamesh->vstep);

  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      for(i = 0; i < 16; i++)
	{
	  fprintf(fileptr, "%g\n", pamesh->ubasis[i]);
	}
    }
  if(pamesh->btype_v == AY_BTCUSTOM)
    {
      for(i = 0; i < 16; i++)
	{
	  fprintf(fileptr, "%g\n", pamesh->vbasis[i]);
	}
    }

  a = 0;
  for(i = 0; i < pamesh->width * pamesh->height; i++)
    {
      fprintf(fileptr,"%g %g %g %g\n", pamesh->controlv[a],
	      pamesh->controlv[a+1],
	      pamesh->controlv[a+2],
	      pamesh->controlv[a+3]);
      a += 4;
    }

  fprintf(fileptr, "%g\n", pamesh->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", pamesh->display_mode);

 return AY_OK;
} /* ay_pamesh_writecb */


/* ay_pamesh_wribcb:
 *  RIB export callback function of pamesh object
 */
int
ay_pamesh_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pamesh_object *patch = NULL;
 ay_object *p;
 RtInt nu, nv;
 RtToken uwrap = RI_NONPERIODIC, vwrap = RI_NONPERIODIC, type;
 RtInt ustep, vstep;
 RtBasis *ubasisptr, *vbasisptr;
 RtBasis ubasis, vbasis;
 RtFloat *controls = NULL;
 RtToken *tokens = NULL;
 RtPointer *parms = NULL;
 int i = 0, j = 0, a = 0, b = 0, n = 0, pvc = 0;
 unsigned int ci = 1;

  if(!o)
    return AY_OK;

  patch = (ay_pamesh_object*)(o->refine);

  if(!patch)
    return AY_ENULL;

  if(patch->type == AY_PTBICUBIC)
    {
      type = RI_BICUBIC;
    }
  else
    {
      type = RI_BILINEAR;
    }

  nu = (RtInt)patch->width;
  if(patch->close_u)
    uwrap = RI_PERIODIC;
  nv = (RtInt)patch->height;
  if(patch->close_v)
    vwrap = RI_PERIODIC;

  if((controls = malloc(nu*nv*4*sizeof(RtFloat))) == NULL)
    return AY_EOMEM;

  a = 0;
  /* RenderMan expects u-major order! */
  for(i = 0; i < nv; i++)
    {
      b = i*4;
      for(j = 0; j < nu; j++)
	{
	  controls[a] = (RtFloat)patch->controlv[b];
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+1];
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+2];
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+3];
	  a++;
	  b += (nv*4);
	}
    }

  switch(patch->btype_u)
    {
    case AY_BTBEZIER:
      ubasisptr = &(RiBezierBasis);
      ustep = RI_BEZIERSTEP;
      break;
    case AY_BTBSPLINE:
      ubasisptr = &(RiBSplineBasis);
      ustep = RI_BSPLINESTEP;
      break;
    case AY_BTCATMULLROM:
      ubasisptr = &(RiCatmullRomBasis);
      ustep = RI_CATMULLROMSTEP;
      break;
    case AY_BTHERMITE:
      ubasisptr = &(RiHermiteBasis);
      ustep = RI_HERMITESTEP;
      break;
    case AY_BTPOWER:
      ubasisptr = &(RiPowerBasis);
      ustep = RI_POWERSTEP;
      break;
    case AY_BTCUSTOM:
      ubasisptr = &(ubasis);
      ustep = (RtInt)patch->ustep;
      for(i = 0; i < 4; i++)
	{
	  for(j = 0; j < 4; j++)
	    {
	      ubasis[i][j] = (RtFloat)patch->ubasis[i*4+j];
	    } /* for */
	} /* for */
      break;
    default:
      ubasisptr = &(RiBezierBasis);
      ustep = RI_BEZIERSTEP;
      break;
    } /* switch */

  switch(patch->btype_v)
    {
    case AY_BTBEZIER:
      vbasisptr = &(RiBezierBasis);
      vstep = RI_BEZIERSTEP;
      break;
    case AY_BTBSPLINE:
      vbasisptr = &(RiBSplineBasis);
      vstep = RI_BSPLINESTEP;
      break;
    case AY_BTCATMULLROM:
      vbasisptr = &(RiCatmullRomBasis);
      vstep = RI_CATMULLROMSTEP;
      break;
    case AY_BTHERMITE:
      vbasisptr = &(RiHermiteBasis);
      vstep = RI_HERMITESTEP;
      break;
    case AY_BTPOWER:
      vbasisptr = &(RiPowerBasis);
      vstep = RI_POWERSTEP;
      break;
    case AY_BTCUSTOM:
      vbasisptr = &(vbasis);
      vstep = (RtInt)patch->vstep;
      for(i = 0; i < 4; i++)
	{
	  for(j = 0; j < 4; j++)
	    {
	      vbasis[i][j] = (RtFloat)patch->vbasis[i*4+j];
	    } /* for */
	} /* for */
      break;
    default:
      vbasisptr = &(RiBezierBasis);
      vstep = RI_BEZIERSTEP;
      break;
    } /* switch */

  if((ubasisptr != &RiBezierBasis) ||
     (vbasisptr != &RiBezierBasis))
    {
      RiBasis(*ubasisptr, ustep, *vbasisptr, vstep);
    }

  /* Do we have any primitive variables? */
  if(!(pvc = ay_pv_count(o)))
    {
      /* No */
      RiPatchMesh(type, nu, uwrap, nv, vwrap, "Pw", controls, NULL);
    }
  else
    {
      /* Yes, we have primitive variables. */
      if(!(tokens = calloc(pvc+1, sizeof(RtToken))))
	{free(controls); return AY_EOMEM;}

      if(!(parms = calloc(pvc+1, sizeof(RtPointer))))
	{free(controls); free(tokens); return AY_EOMEM;}

      tokens[0] = "Pw";
      parms[0] = (RtPointer)controls;

      n = 1;
      ay_pv_filltokpar(o, AY_TRUE, 1, &n, tokens, parms);

      RiPatchMeshV(type, nu, uwrap, nv, vwrap, (RtInt)n, tokens, parms);

      for(i = 1; i < n; i++)
	{
	  free(tokens[i]);
	  free(parms[i]);
	}

      free(tokens);
      free(parms);
    } /* if */

  free(controls);

  p = patch->caps_and_bevels;
  while(p)
    {
      ay_wrib_caporbevel(file, o, p, ci);
      ci++;
      p = p->next;
    }

 return ay_status;
} /* ay_pamesh_wribcb */


/* ay_pamesh_bbccb:
 *  bounding box calculation callback function of pamesh object
 */
int
ay_pamesh_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_pamesh_object *pamesh;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

 return ay_bbc_fromarr(pamesh->controlv, pamesh->width*pamesh->height,
		       4, bbox);
} /* ay_pamesh_bbccb */


/* ay_pamesh_notifycb:
 *  notification callback function of pamesh object
 */
int
ay_pamesh_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_pamesh_object *pamesh = NULL;
 ay_nurbpatch_object *np = NULL;
 ay_object *bevel = NULL, **nextcb;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

  nextcb = &(pamesh->caps_and_bevels);

  if(pamesh->npatch)
    {
      (void)ay_object_delete(pamesh->npatch);
      pamesh->npatch = NULL;
    }

  if(pamesh->caps_and_bevels)
    {
      (void)ay_object_deletemulti(pamesh->caps_and_bevels, AY_FALSE);
      pamesh->caps_and_bevels = NULL;
    }

  if(o->modified)
    {
      pamesh->is_rat = ay_pmt_israt(pamesh);
    }

  if(ay_pmt_valid(pamesh))
    {
      return AY_OK;
    }

  ay_status = ay_pmt_tonpatch(o, AY_BTBSPLINE, &(pamesh->npatch));

  if(pamesh->npatch)
    {
      np = (ay_nurbpatch_object *)pamesh->npatch->refine;
      np->display_mode = pamesh->display_mode;
      np->glu_sampling_tolerance = pamesh->glu_sampling_tolerance;

      /* get bevel and cap parameters */
      if(o->tags)
	{
	  ay_bevelt_parsetags(o->tags, &bparams);
	  ay_capt_parsetags(o->tags, &cparams);
	}

      /* create/add caps */
      if(cparams.has_caps)
	{
	  ay_status = ay_capt_addcaps(&cparams, &bparams, pamesh->npatch,
				      nextcb);
	  if(ay_status)
	    goto cleanup;

	  while(*nextcb)
	    nextcb = &((*nextcb)->next);
	}

      /* create/add bevels */
      if(bparams.has_bevels)
	{
	  bparams.dirs[1] = !bparams.dirs[1];
	  bparams.dirs[2] = !bparams.dirs[2];
	  bparams.radii[2] = -bparams.radii[2];

	  ay_status = ay_bevelt_addbevels(&bparams, &cparams, pamesh->npatch,
					  nextcb);
	  if(ay_status)
	    goto cleanup;
	}

      bevel = pamesh->caps_and_bevels;
      while(bevel)
	{
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_sampling_tolerance =
	    pamesh->glu_sampling_tolerance;
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->display_mode = pamesh->display_mode;
	  bevel = bevel->next;
	}
    } /* if */

cleanup:

 return ay_status;
} /* ay_pamesh_notifycb */


/* ay_pamesh_providecb:
 *  provide callback function of pamesh object
 */
int
ay_pamesh_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_pamesh_object *pm = NULL;

  if(!o)
    return AY_ENULL;

  pm = (ay_pamesh_object *) o->refine;

  if(!pm)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, pm->npatch, pm->caps_and_bevels, result);
} /* ay_pamesh_providecb */


/* ay_pamesh_convertcb:
 *  convert callback function of pamesh object
 */
int
ay_pamesh_convertcb(ay_object *o, int in_place)
{
 ay_pamesh_object *pm = NULL;

  if(!o)
    return AY_ENULL;

  pm = (ay_pamesh_object *) o->refine;

  if(!pm)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, pm->npatch, pm->caps_and_bevels, in_place);
} /* ay_pamesh_convertcb */


/* ay_pamesh_init:
 *  initialize the pamesh object module
 */
int
ay_pamesh_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 int i;
 char buf[32];

  ay_status = ay_otype_registercore(ay_pamesh_name,
				    ay_pamesh_createcb,
				    ay_pamesh_deletecb,
				    ay_pamesh_copycb,
				    ay_pamesh_drawcb,
				    ay_pamesh_drawhcb,
				    ay_pamesh_shadecb,
				    ay_pamesh_setpropcb,
				    ay_pamesh_getpropcb,
				    ay_pamesh_getpntcb,
				    ay_pamesh_readcb,
				    ay_pamesh_writecb,
				    ay_pamesh_wribcb,
				    ay_pamesh_bbccb,
				    AY_IDPAMESH);

  ay_status += ay_draw_registerdacb(ay_pamesh_drawacb, AY_IDPAMESH);

  ay_status += ay_notify_register(ay_pamesh_notifycb, AY_IDPAMESH);

  ay_status += ay_convert_register(ay_pamesh_convertcb, AY_IDPAMESH);

  ay_status += ay_provide_register(ay_pamesh_providecb, AY_IDPAMESH);

  arrobj = Tcl_NewStringObj(ay_pamesh_arr, -1);
  Tcl_IncrRefCount(arrobj);

  for(i = 0; i < 16; i++)
    {
      sprintf(buf,"Basis_U_%d",i);
      bmu[i] = Tcl_NewStringObj(buf, -1);
      Tcl_IncrRefCount(bmu[i]);
      sprintf(buf,"Basis_V_%d",i);
      bmv[i] = Tcl_NewStringObj(buf, -1);
      Tcl_IncrRefCount(bmv[i]);
    }

 return ay_status;
} /* ay_pamesh_init */

