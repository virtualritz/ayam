/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2017 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* bcurve.c - basis curve custom object (bicubic patchmesh curve equivalent) */

/** BCurve object type */
typedef struct bcurve_object_s
{
  int is_rat; /**< is the curve rational? */
  int closed; /**< is the curve closed? */
  int length; /**< number of control points (> 4) */
  int btype; /* basis type (AY_BT*) */
  int step; /**< step size of basis */
  double *basis; /**< basis matrix (if btype is AY_BTCUSTOM) */

  double *controlv; /**< control points [length*4] */

  ay_object *ncurve; /**< cached NURBS curve */

  int display_mode; /**< drawing mode */
  double glu_sampling_tolerance; /**< drawing quality */
} bcurve_object;


/* global variables: */
char bcurve_version_ma[] = AY_VERSIONSTR;
char bcurve_version_mi[] = AY_VERSIONSTRMI;

static char *bcurve_name = "BCurve";
static char *bcurve_arr = "BCurveAttrData";

static unsigned int bcurve_id;

static Tcl_Obj *arrobj = NULL;
static Tcl_Obj *bm[16] = {0};

/* Bezier */
static double mb[16] = {-1, 3, -3, 1,  3, -6, 3, 0,  -3, 3, 0, 0,  1, 0, 0, 0};
/* Hermite (RenderMan flavour) */
static double mh[16] = {2, -3, 0, 1,  1, -2, 1, 0,  -2, 3, 0, 0,  1, -1, 0, 0};
/* Catmull Rom */
static double mc[16] = {-0.5, 1, -0.5, 0,  1.5, -2.5, 0, 1,  -1.5, 2, 0.5, 0,
			0.5, -0.5, 0, 0};
/* B-Spline */
static double ms[16] = {-1.0/6, 3.0/6, -3.0/6, 1.0/6,  3.0/6, -1, 0, 4.0/6,
			-3.0/6, 3.0/6, 3.0/6, 1.0/6,  1.0/6, 0, 0, 0};
/* Power */
static double mp[16] = {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};


static double mbi[16];

static double msi[16];


/* prototypes of functions local to this module: */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int bcurve_Init(Tcl_Interp *interp);

int bcurve_tobasis(bcurve_object *bc, int btype, int bstep, double *basis);

int bcurve_toncurvemulti(ay_object *o, ay_object **result);

int bcurve_toncurve(ay_object *o, int btype, ay_object **result);

int bcurve_israt(bcurve_object *bc);

void bcurve_drawweights(bcurve_object *bcurve);


/* functions: */

/** bcurve_tobasis:
 * Convert a BCurve to a different basis.
 *
 * \param[in,out] bc BCurve to process (must be open and of length 4)
 * \param[in] btype target basis type
 * \param[in] bstep target step size
 * \param[in] basis target basis (may be NULL unless \a btype is AY_BTCUSTOM)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
bcurve_tobasis(bcurve_object *bc, int btype, int bstep, double *basis)
{
 int convert = AY_FALSE, i, j, k, l, i1, i2;
 double *p1, *p2, *p3, *p4;
 double mi[16], mu[16], mut[16];
 double v[4];

  if(!bc)
    return AY_ENULL;

  if((btype == AY_BTCUSTOM) && !basis)
    return AY_ERROR;

  if(bc->length != 4 || bc->closed)
    return AY_ERROR;

  if(bc->btype != btype)
    {
      convert = AY_TRUE;
    }
  else
    {
      if(bc->btype == AY_BTCUSTOM)
	{
	  if(memcmp(bc->basis, basis, 16*sizeof(double)))
	    convert = AY_TRUE;
	}
    }

  if(!convert)
    return AY_OK;

  /* create conversion matrices */
  switch(btype)
    {
    case AY_BTBSPLINE:
      memcpy(mu, msi, 16*sizeof(double));
      break;
    case AY_BTBEZIER:
      memcpy(mu, mbi, 16*sizeof(double));
      break;
    case AY_BTCATMULLROM:
      if(ay_trafo_invgenmatrix(mc, mi))
	{
	  return AY_ERROR;
	}
      memcpy(mu, mi, 16*sizeof(double));
      break;
    case AY_BTHERMITE:
      if(ay_trafo_invgenmatrix(mh, mi))
	{
	  return AY_ERROR;
	}
      memcpy(mu, mi, 16*sizeof(double));
      break;
    case AY_BTPOWER:
      ay_trafo_identitymatrix(mu);
      break;
    default:
      if(ay_trafo_invgenmatrix(basis, mi))
	{
	  return AY_ERROR;
	}
      memcpy(mu, mi, 16*sizeof(double));
      break;
    }

  switch(bc->btype)
    {
    case AY_BTBEZIER:
      ay_trafo_multmatrix(mu, mb);
      break;
    case AY_BTBSPLINE:
      ay_trafo_multmatrix(mu, ms);
      break;
    case AY_BTHERMITE:
      ay_trafo_multmatrix(mu, mh);
      break;
    case AY_BTCATMULLROM:
      ay_trafo_multmatrix(mu, mc);
      break;
    case AY_BTPOWER:
      /* use inv(basis) unchanged */
      break;
    case AY_BTCUSTOM:
      ay_trafo_multmatrix(mu, bc->basis);
      break;
    default:
      return AY_ERROR;
    }

  /* transpose conversion matrix */
  i1 = 0;
  for(i = 0; i < 4; i++)
    {
      i2 = i;
      for(j = 0; j < 4; j++)
	{
	  mut[i2] = mu[i1];

	  i1++;
	  i2 += 4;
	} /* for */
    } /* for */

  p1 = bc->controlv;
  p2 = p1 + 4;
  p3 = p2 + 4;
  p4 = p3 + 4;

  /* for each control point component (x,y,z) */
  for(k = 0; k < 3; k++)
    {
      /* get coordinates into a vector */
      for(l = 0; l < 4; l++)
	{
	  v[0] = *(p1+k);
	  v[1] = *(p2+k);
	  v[2] = *(p3+k);
	  v[3] = *(p4+k);
	}

      /* apply conversion matrix */
      ay_trafo_multvectmatrix(v, mut);

      /* copy converted coordinates back */
      for(l = 0; l < 4; l++)
	{
	  *(p1+k) = v[0];
	  *(p2+k) = v[1];
	  *(p3+k) = v[2];
	  *(p4+k) = v[3];
	}
    } /* for each component */

  if(btype == AY_BTCUSTOM)
    {
      /* copy custom basis */
      if(bc->btype != AY_BTCUSTOM)
	{
	  if(!(bc->basis = malloc(16*sizeof(double))))
	     return AY_EOMEM;
	}
      memcpy(bc->basis, basis, 16*sizeof(double));
    }

  bc->btype = btype;
  bc->step = bstep;

 return AY_OK;
} /* bcurve_tobasis */


/* bcurve_toncurvemulti:
 * helper for bcurve_toncurve() below;
 * converts a complex bcurve by moving an evaluation window
 * over the curve (according to the parameters defined by the basis type
 * and step size), converting the respective window to a single NURBS
 * curve, then concatenating the resulting curves to the final resulting
 * NURBS curve
 */
int
bcurve_toncurvemulti(ay_object *o, ay_object **result)
{
 int ay_status = AY_OK;
 double tcv[4*4], *cv = NULL;
 int l, s;
 int i, ii, a, b;
 ay_object t = {0}, *curves = NULL, **nextcurve;
 bcurve_object *bc = NULL, tbc = {0};

  if(!o || !result)
    return AY_ENULL;

  bc = (bcurve_object*)o->refine;
  cv = bc->controlv;

  /* set up temporary bcurve (t and tbc) */
  ay_object_defaults(&t);
  t.type = bcurve_id;
  t.refine = &tbc;
  tbc.length = 4;
  tbc.btype = bc->btype;
  tbc.basis = bc->basis;

  /* due to hard-coded length 4, no need to copy the steps */
  /*
  tbc.step = bc->step;
  */
  tbc.controlv = tcv;

  switch(bc->btype)
    {
    case AY_BTPOWER:
      s = 4;
      break;
    case AY_BTBEZIER:
      s = 3;
      break;
    case AY_BTHERMITE:
      s = 2;
      break;
    case AY_BTCATMULLROM:
    case AY_BTBSPLINE:
      s = 1;
      break;
    default:
      s = bc->step;
      break;
    }

  l = (bc->length-3)/s+1;

  if(bc->closed)
    {
      while(((l+1)*s) < bc->length)
	l++;
    }
  else
    {
      if(s == 1)
	l--;
    }

  nextcurve = &curves;

  for(i = 0; i < l; i++)
    {
      /* get eval window (fill tcv) */
      a = 0;
      b = s*i*4;
      for(ii = 0; ii < 4; ii++)
	{
	  /* wraparound? */
	  if(b > (bc->length*4-1))
	    b -= (bc->length*4);

	  memcpy(&(tcv[a]), &(cv[b]), 4*sizeof(double));
	  b += 4;
	  a += 4;
	}

      /* convert to NCurve */
      ay_status = bcurve_toncurve(&t, AY_BTBEZIER, nextcurve);
      if(!ay_status && *nextcurve)
	nextcurve = &((*nextcurve)->next);
      else
	goto cleanup;

    } /* for all eval windows */

  /* concatenate all curves? */
  if(curves && curves->next)
    {
      ay_nct_concatmultiple(/*closed=*/0, /*knot_type=*/1,
		    /*fillgaps=*/AY_FALSE, curves,
		    result);
    }
  else
    {
      *result = curves;
      curves = NULL;
    }

cleanup:

  if(curves)
    (void)ay_object_deletemulti(curves, AY_FALSE);

 return ay_status;
} /* bcurve_toncurvemulti */


/** bcurve_toncurve:
 * Create a NURBS curve from a BCurve.
 * This function will not work properly or crash if the curve
 * fails the validity check using bcurve_valid() below!
 * This function only handles curves of basis type AY_BTBSPLINE and
 * open curves of arbitrary basis type and length 4.
 * Other, "complex", curves are handled via bcurve_toncurvemulti()
 * above.
 *
 * \param[in] o the curve to convert
 * \param[in] btype desired basis type, must be AY_BTBSPLINE;
 *  the other valid type (AY_BTBEZIER) is used exclusively by
 *  bcurve_toncurvemulti() when calling back
 * \param[in,out] result pointer where to store the resulting NCurve
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
bcurve_toncurve(ay_object *o, int btype, ay_object **result)
{
 int ay_status = AY_OK;
 double *cv = NULL;
 int l, kt;
 int i = 0, a, b;
 ay_object *newo = NULL, *c = NULL;
 bcurve_object *bcurve;

  if(!o || !result)
    return AY_ENULL;

  bcurve = (bcurve_object*) o->refine;

  l = bcurve->length;

  if((bcurve->btype != AY_BTBSPLINE) && (bcurve->length > 4 || bcurve->closed))
    return bcurve_toncurvemulti(o, result);

  cv = bcurve->controlv;

  if(bcurve->btype != btype)
    {
      (void)ay_object_copy(o, &c);
      if(c)
	{
	  ay_status = bcurve_tobasis(c->refine, btype, /*bstep=*/1,
				     /*basis=*/NULL);
	  if(ay_status)
	    goto cleanup;
	}
      else
	return AY_ERROR;

      bcurve = c->refine;
    }
  else
    {
      if(bcurve->closed)
	l += 3;
    }

  if(btype == AY_BTBSPLINE)
    {
      kt = AY_KTBSPLINE;
    }
  else
    {
      kt = AY_KTBEZIER;
    }

  if(!(newo = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(newo);
  newo->type = AY_IDNCURVE;

  if(!(cv = malloc(l*4*sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* fill cv */
  a = 0;
  b = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      memcpy(&(cv[a]), &(bcurve->controlv[b]), 4*sizeof(double));
      a += 4;
      b += 4;
    }
  b = 0;
  if(l > bcurve->length)
    {
      for(i = 0; i < (l-bcurve->length); i++)
	{
	  memcpy(&(cv[a]), &(bcurve->controlv[b]), 4*sizeof(double));
	  a += 4;
	  b += 4;
	}
    }

  ay_status = ay_nct_create(4, l, kt, cv, NULL,
			    (ay_nurbcurve_object **)(void*)&(newo->refine));

  cv = bcurve->controlv;

  if(ay_status)
    {
      goto cleanup;
    }

  /* return result */
  *result = newo;

  /* prevent cleanup code from doing something harmful */
  cv = NULL;
  newo = NULL;

cleanup:

  if(c)
    ay_object_delete(c);
  if(cv)
    free(cv);
  if(newo)
    free(newo);

 return ay_status;
} /* bcurve_toncurve */


/** bcurve_israt:
 *  check whether any control point of BCurve \a bc
 *  uses a weight value (!= 1.0)
 *
 * \param[in] bc BCurve to check
 *
 * \returns AY_TRUE if any weight is != 1.0, else returns AY_FALSE
 */
int
bcurve_israt(bcurve_object *bc)
{
 int i;
 double *p;

  if(!bc)
    return AY_FALSE;

  p = &(bc->controlv[3]);
  for(i = 0; i < bc->length; i++)
    {
      if((fabs(*p) < (1.0-AY_EPSILON)) || (fabs(*p) > (1.0+AY_EPSILON)))
	return AY_TRUE;
      p += 4;
    } /* for */

 return AY_FALSE;
} /* bcurve_israt */


/** bcurve_valid:
 *  check basis curve for validity
 *
 * \param[in] bcurve BCurve object to process
 *
 * \returns AY_OK (0) if curve is valid
 *   else:
 *  -1: NULL pointer delivered
 *   1: too few control points (need at least 4, unless periodic Bezier)
 *   2: stepsize too small
 *   3: basistype length mismatch
 *
 */
int
bcurve_valid(bcurve_object *bcurve)
{
 int step = 0;

  if(!bcurve)
    return -1;

  if(bcurve->length < 3)
    {
      return 1;
    }

  switch(bcurve->btype)
    {
    case AY_BTBEZIER:
      step = 3;
      break;
    case AY_BTBSPLINE:
      step = 1;
      break;
    case AY_BTCATMULLROM:
      step = 1;
      break;
    case AY_BTHERMITE:
      step = 2;
      break;
    case AY_BTPOWER:
      step = 4;
      break;
    case AY_BTCUSTOM:
      step = bcurve->step;
      break;
    default:
      break;
    } /* switch */

  if(step <= 0 || step > 4)
    {
      return 2;
    }

  if(bcurve->closed)
    {
      /* periodic curve */
      if(fabs(fmod((double)bcurve->length-4+(4-step), step)) >
	 AY_EPSILON)
	{
	  return 3;
	}
    }
  else
    {
      /* non periodic/open curve */
      if(bcurve->length < 4)
	{
	  return 1;
	}
      if(fabs(fmod((double)bcurve->length-4, step)) > AY_EPSILON)
	{
	  return 3;
	}
    }

 return AY_OK;
} /* bcurve_valid */


/*
 ****
 ****
 */


/* bcurve_createcb:
 *  create callback function of bcurve object
 */
int
bcurve_createcb(int argc, char *argv[], ay_object *o)
{
 bcurve_object *bcurve = NULL;
 char fname[] = "crtbcurve";
 int a, i;

  if(!o)
    return AY_ENULL;

  if(!(bcurve = calloc(1, sizeof(bcurve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  bcurve->length = 4;
  bcurve->btype = AY_BTBEZIER;

  if(!(bcurve->controlv = malloc(4 * bcurve->length * sizeof(double))))
    {
      free(bcurve);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      bcurve->controlv[a] = i*0.25;
      bcurve->controlv[a+1] = 0;
      bcurve->controlv[a+2] = 0;
      bcurve->controlv[a+3] = 1.0;
      a += 4;
    }

  o->refine = bcurve;

  ay_notify_object(o);

 return AY_OK;
} /* bcurve_createcb */


/* bcurve_deletecb:
 *  delete callback function of bcurve object
 */
int
bcurve_deletecb(void *c)
{
 bcurve_object *bcurve = NULL;

  if(!c)
    return AY_ENULL;

  bcurve = (bcurve_object *)(c);

  if(bcurve->basis)
    {
      free(bcurve->basis);
      bcurve->basis = NULL;
    }

  /* free cached ncurve */
  ay_object_delete(bcurve->ncurve);

  free(bcurve);

 return AY_OK;
} /* bcurve_deletecb */


/* bcurve_copycb:
 *  copy callback function of bcurve object
 */
int
bcurve_copycb(void *src, void **dst)
{
  bcurve_object *bcurve = NULL, *bcurvesrc;

  if(!src || !dst)
    return AY_ENULL;

  bcurvesrc = (bcurve_object *)src;

  if(!(bcurve = calloc(1, sizeof(bcurve_object))))
    return AY_EOMEM;

  memcpy(bcurve, src, sizeof(bcurve_object));

  bcurve->controlv = NULL;
  bcurve->basis = NULL;

  bcurve->ncurve = NULL;

  /* copy controlv */
  if(!(bcurve->controlv = malloc(4 * bcurve->length * sizeof(double))))
    {
      free(bcurve);
      return AY_EOMEM;
    }
  memcpy(bcurve->controlv, bcurvesrc->controlv,
	 4 * bcurve->length * sizeof(double));

  /* copy basis */
  if(bcurvesrc->basis)
    {
      if(!(bcurve->basis = malloc(16 * sizeof(double))))
	{
	  free(bcurve->controlv);
	  free(bcurve);
	  return AY_EOMEM;
	}
      memcpy(bcurve->basis, bcurvesrc->basis, 16 * sizeof(double));
    }

  *dst = (void *)bcurve;

 return AY_OK;
} /* bcurve_copycb */


/* bcurve_drawcb:
 *  draw (display in an Ayam view window) callback function of bcurve object
 */
int
bcurve_drawcb(struct Togl *togl, ay_object *o)
{
 bcurve_object *bcurve = NULL;

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

  if(bcurve->ncurve)
    {
      ay_draw_object(togl, bcurve->ncurve, AY_TRUE);
    }

 return AY_OK;
} /* bcurve_drawcb */


/* bcurve_drawacb:
 *  draw annotations (in an Ayam view window) callback function of bcurve object
 */
int
bcurve_drawacb(struct Togl *togl, ay_object *o)
{
 bcurve_object *bcurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 GLdouble *ver = NULL;

  bcurve = (bcurve_object *) o->refine;

  if(!bcurve)
    return AY_ENULL;

  /* draw orientation arrow */
  if(0&&bcurve->ncurve)
    {
      ncurve = bcurve->ncurve->refine;
      ver = ncurve->controlv;
      ay_draw_arrow(togl, &(ver[ncurve->length*4-8]),
		    &(ver[ncurve->length*4-4]));
    }
  else
    {
      ver = bcurve->controlv;
      if(bcurve->closed)
	ay_draw_arrow(togl, &(ver[bcurve->length*4-4]), &(ver[0]));
      else
	ay_draw_arrow(togl, &(ver[bcurve->length*4-8]),
		      &(ver[bcurve->length*4-4]));
    }

 return AY_OK;
} /* bcurve_drawacb */


/* bcurve_drawweights:
 * helper for bcurve_drawhcb() below,
 * draw colored handles based on their weight values
 */
void
bcurve_drawweights(bcurve_object *bcurve)
{
 int i;
 double w, *pnts;

  pnts = bcurve->controlv;

  /* draw normal points */
  glBegin(GL_POINTS);
   if(bcurve->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < bcurve->length; i++)
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
       for(i = 0; i < bcurve->length; i++)
	 {
	   ay_nct_colorfromweight(pnts[3]);
	   glVertex3dv((GLdouble *)pnts);
	   pnts += 4;
	 }
     }
  glEnd();

  glColor3ub(255,255,255);

 return;
} /* bcurve_drawweights */


/* bcurve_drawhcb:
 *  draw handles callback function of bcurve object
 */
int
bcurve_drawhcb(struct Togl *togl, ay_object *o)
{
 bcurve_object *bcurve = NULL;
 double w, *cv = NULL;
 int i;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

  if(view->drawhandles == 3)
    {
      bcurve_drawweights(bcurve);
      return AY_OK;
    }

  if(view->drawhandles == 2)
    {
      if(bcurve->ncurve)
	{
	  glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		    (GLfloat)ay_prefs.obb);
	  ay_nct_drawbreakpoints(togl, bcurve->ncurve);
	  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		    (GLfloat)ay_prefs.seb);
	}
      return AY_OK;
    }

  cv = bcurve->controlv;

  /* draw normal points */
  glBegin(GL_POINTS);
   if(bcurve->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < bcurve->length; i++)
	 {
	   w = cv[3];
	   glVertex3d((GLdouble)(cv[0]*w),
		      (GLdouble)(cv[1]*w),
		      (GLdouble)(cv[2]*w));
	   cv += 4;
	 }
     }
   else
     {
       for(i = 0; i < bcurve->length; i++)
	 {
	   glVertex3dv((GLdouble *)cv);
	   cv += 4;
	 }
     }
  glEnd();

 return AY_OK;
} /* bcurve_drawhcb */


/* bcurve_getpntcb:
 *  get point (editing and selection) callback function of bcurve object
 */
int
bcurve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 bcurve_object *bcurve = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c, h[3];
 int i = 0, j = 0, a = 0;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->type = AY_PTRAT;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = malloc(bcurve->length * sizeof(double*))))
	return AY_EOMEM;
      if(!(pe->indices = malloc(bcurve->length * sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < bcurve->length; i++)
	{
	  pe->coords[i] = &(bcurve->controlv[a]);
	  pe->indices[i] = i;
	  a += 4;
	}

      pe->num = bcurve->length;
      break;
    case 1:
      /* selection based on a single point */
      control = bcurve->controlv;
      for(i = 0; i < bcurve->length; i++)
	{
	  if(bcurve->is_rat && ay_prefs.rationalpoints)
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
	      peindex = i;
	      min_dist = dist;
	    }

	  j += 4;
	} /* for */

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!(pe->coords = calloc(1, sizeof(double *))))
	return AY_EOMEM;

      if(!(pe->indices = calloc(1, sizeof(unsigned int))))
	return AY_EOMEM;

      pe->coords[0] = pecoord;
      pe->indices[0] = peindex;
      pe->num = 1;
      break;
    case 2:
      /* selection based on planes */
      control = bcurve->controlv;
      j = 0;
      a = 0;
      if(bcurve->is_rat && ay_prefs.rationalpoints)
	{
	  c = h;
	}
      for(i = 0; i < bcurve->length; i++)
	{
	  if(bcurve->is_rat && ay_prefs.rationalpoints)
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
	      if(!(itmp = realloc(pe->indices, (a+1)*sizeof(unsigned int))))
		return AY_EOMEM;
	      pe->indices = itmp;

	      pe->coords[a] = &(control[j]);
	      pe->indices[a] = i;
	      a++;
	    } /* if */

	  j += 4;
	} /* for */

      pe->num = a;
      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &(o->selp);
      while(pnt)
	{
	  if(pnt->index < (unsigned int)bcurve->length)
	    {
	      pnt->point = &(bcurve->controlv[pnt->index*4]);
	      pnt->type = AY_PTRAT;
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
} /* bcurve_getpntcb */


/* bcurve_setpropcb:
 *  set property (from Tcl to C context) callback function of bcurve object
 */
int
bcurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "bcurve_setpropcb";
 char *arr = "BCurveAttrData";
 Tcl_Obj *to = NULL;
 bcurve_object *bcurve = NULL;
 double dtemp, *basis;
 int j, update = AY_FALSE, new_closed, new_length, new_btype, new_step;

  if(!interp || !o)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

  to = Tcl_GetVar2Ex(interp, arr, "Closed",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_closed));

  if(new_closed != bcurve->closed)
    {
      update = AY_TRUE;
      bcurve->closed = new_closed;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Length",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_length));

  if(new_length != bcurve->length)
    {
      update = AY_TRUE;
    }

  to = Tcl_GetVar2Ex(interp, arr, "BType",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(new_btype));

  if(new_btype != bcurve->btype)
    {
      update = AY_TRUE;
    }

  if(new_btype == AY_BTCUSTOM)
    {
      update = AY_TRUE;
      if(!bcurve->basis)
	{
	  if(!(bcurve->basis = calloc(16, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      goto cleanup;
	    } /* if */
	} /* if */

      if(bcurve->btype == AY_BTCUSTOM)
	{
	  for(j = 0; j < 16; j++)
	    {
	      to = Tcl_ObjGetVar2(interp, arrobj, bm[j], TCL_LEAVE_ERR_MSG |
				  TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      bcurve->basis[j] = dtemp;
	    } /* for */
	}
      else
	{
	  /* switching from another basis to custom */
	  basis = NULL;
	  ay_pmt_getbasis(bcurve->btype, &basis);
	  if(basis)
	    {
	      memcpy(bcurve->basis, basis, 16*sizeof(double));
	    }
	}

      to = Tcl_GetVar2Ex(interp, arr, "Step",
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &(new_step));
      if(new_step <= 0 || new_step > 4)
	bcurve->step = 1;
      else
	bcurve->step = new_step;
    }
  else
    {
      if(bcurve->basis)
	free(bcurve->basis);
      bcurve->basis = NULL;
    } /* if */

  /* resize curve */
  if(new_length != bcurve->length && (new_length > 2))
    {
      ay_status = ay_npt_resizearrayw(&(bcurve->controlv), 4, bcurve->length,
				      1, new_length);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not resize curve!");
	}
      else
	{
	  (void)bcurve_getpntcb(3, o, NULL, NULL);
	  update = AY_TRUE;
	  bcurve->length = new_length;
	}
    } /* if */

  bcurve->btype = new_btype;

  to = Tcl_GetVar2Ex(interp, arr, "Tolerance",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(bcurve->glu_sampling_tolerance));

  to = Tcl_GetVar2Ex(interp, arr, "DisplayMode",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(bcurve->display_mode));

  if(update)
    {
      (void)ay_notify_object(o);

      o->modified = AY_TRUE;
      (void)ay_notify_parent();
    }

cleanup:

 return AY_OK;
} /* bcurve_setpropcb */


/* bcurve_getpropcb:
 *  get property (from C to Tcl context) callback function of bcurve object
 */
int
bcurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *arr = "BCurveAttrData";
 Tcl_Obj *to = NULL;
 bcurve_object *bcurve = NULL;
 int j;


  if(!interp || !o)
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  Tcl_SetVar2Ex(interp, arr, "Closed",
		Tcl_NewIntObj(bcurve->closed),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Length",
		Tcl_NewIntObj(bcurve->length),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "BType",
		Tcl_NewIntObj(bcurve->btype),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(bcurve->btype == AY_BTCUSTOM)
    {
      if(bcurve->basis)
	{
	  for(j = 0; j < 16; j++)
	    {
	      to = Tcl_NewDoubleObj(bcurve->basis[j]);

	      Tcl_ObjSetVar2(interp, arrobj, bm[j], to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	    } /* for */
	} /* if */

      Tcl_SetVar2Ex(interp, arr, "Step",
		    Tcl_NewIntObj(bcurve->step),
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    } /* if */

  if(bcurve->is_rat)
    to = Tcl_NewStringObj("yes", -1);
  else
    to = Tcl_NewStringObj("no", -1);
  Tcl_SetVar2Ex(interp, arr, "IsRat", to,
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tolerance",
		Tcl_NewDoubleObj(bcurve->glu_sampling_tolerance),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "DisplayMode",
		Tcl_NewIntObj(bcurve->display_mode),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return AY_OK;
} /* bcurve_getpropcb */


/* bcurve_readcb:
 *  read (from scene file) callback function of bcurve object
 */
int
bcurve_readcb(FILE *fileptr, ay_object *o)
{
 bcurve_object *bcurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(bcurve = calloc(1, sizeof(bcurve_object))))
    return AY_EOMEM;

  fscanf(fileptr, "%d\n", &bcurve->closed);
  fscanf(fileptr, "%d\n", &bcurve->length);
  fscanf(fileptr, "%d\n", &bcurve->btype);

  if(bcurve->btype == AY_BTCUSTOM)
    {
      fscanf(fileptr, "%d\n", &bcurve->step);
      if(!(bcurve->basis = malloc(16*sizeof(double))))
	{
	  free(bcurve);
	  return AY_EOMEM;
	}

      a = 0;
      for(i = 0; i < 4; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg %lg\n", &(bcurve->basis[a]),
		 &(bcurve->basis[a+1]),
		 &(bcurve->basis[a+2]),
		 &(bcurve->basis[a+3]));
	  a += 4;
	}
    }

  if(!(bcurve->controlv = calloc(bcurve->length*4, sizeof(double))))
    {
      if(bcurve->basis)
	{free(bcurve->basis);}
      free(bcurve);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      fscanf(fileptr, "%lg %lg %lg %lg\n", &(bcurve->controlv[a]),
	     &(bcurve->controlv[a+1]),
	     &(bcurve->controlv[a+2]),
	     &(bcurve->controlv[a+3]));
      a += 4;
    }

  fscanf(fileptr, "%lg\n", &(bcurve->glu_sampling_tolerance));
  fscanf(fileptr, "%d\n", &(bcurve->display_mode));

  bcurve->is_rat = bcurve_israt(bcurve);

  o->refine = bcurve;

 return AY_OK;
} /* bcurve_readcb */


/* bcurve_writecb:
 *  write (to scene file) callback function of bcurve object
 */
int
bcurve_writecb(FILE *fileptr, ay_object *o)
{
 bcurve_object *bcurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", bcurve->closed);
  fprintf(fileptr, "%d\n", bcurve->length);
  fprintf(fileptr, "%d\n", bcurve->btype);

  if(bcurve->btype == AY_BTCUSTOM)
    {
      fprintf(fileptr, "%d\n", bcurve->step);
      a = 0;
      for(i = 0; i < 4; i++)
	{
	  fprintf(fileptr, "%g %g %g %g\n", bcurve->basis[a],
		  bcurve->basis[a+1],
		  bcurve->basis[a+2],
		  bcurve->basis[a+3]);
	  a += 4;
	}
    }

  a = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      fprintf(fileptr, "%g %g %g %g\n", bcurve->controlv[a],
	      bcurve->controlv[a+1],
	      bcurve->controlv[a+2],
	      bcurve->controlv[a+3]);
      a += 4;
    }

  fprintf(fileptr, "%g\n", bcurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", bcurve->display_mode);

 return AY_OK;
} /* bcurve_writecb */


/* bcurve_bbccb:
 *  bounding box calculation callback function of bcurve object
 */
int
bcurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 bcurve_object *bcurve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

 return ay_bbc_fromarr(bcurve->controlv, bcurve->length, 4, bbox);
} /* bcurve_bbccb */


/* bcurve_notifycb:
 *  notification callback function of bcurve object
 */
int
bcurve_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 bcurve_object *bcurve = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  /* remove old NURBS curve */
  if(bcurve->ncurve)
    ay_object_delete(bcurve->ncurve);
  bcurve->ncurve = NULL;

  if(bcurve_valid(bcurve))
    {
      return AY_OK;
    }

  if(o->modified)
    {
      bcurve->is_rat = bcurve_israt(bcurve);
    }

  /* create new NURBS curve */
  ay_status = bcurve_toncurve(o, AY_BTBSPLINE, &(bcurve->ncurve));

  if(bcurve->ncurve && bcurve->ncurve->refine &&
     bcurve->ncurve->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object*)bcurve->ncurve->refine;
      nc->display_mode = bcurve->display_mode;
      nc->glu_sampling_tolerance = bcurve->glu_sampling_tolerance;
    }

 return ay_status;
} /* bcurve_notifycb */


/* bcurve_convertcb:
 *  convert callback function of bcurve object
 */
int
bcurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 bcurve_object *bcurve = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *) o->refine;

  if(!bcurve)
    return AY_ENULL;

  if(bcurve->ncurve)
    {
      ay_status = ay_object_copy(bcurve->ncurve, &new);

      if(new)
	{
	  nc = (ay_nurbcurve_object *)(new->refine);

	  /* reset display mode and sampling tolerance
	     of new curve to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
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
	      ay_status = ay_object_replace(new, o);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* bcurve_convertcb */


/* bcurve_providecb:
 *  provide callback function of bcurve object
 */
int
bcurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 bcurve_object *bcurve = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if((type == AY_IDNCURVE) || (type == UINT_MAX-AY_IDNCURVE))
	return AY_OK;
      else
	return AY_ERROR;
    }

  bcurve = (bcurve_object *) o->refine;

  if(!bcurve)
    return AY_ENULL;

  if(type == UINT_MAX-AY_IDNCURVE)
    {
      /* peek */
      if(!bcurve->ncurve)
	return AY_ERROR;
      *result = bcurve->ncurve;
      return AY_OK;
    }

  if(type == AY_IDNCURVE)
    {
      if(bcurve->ncurve)
	{
	  ay_status = ay_object_copy(bcurve->ncurve, result);
	  if(*result)
	    {
	      ay_trafo_copy(o, *result);
	    } /* if */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    } /* if */

 return ay_status;
} /* bcurve_providecb */


/** bcurve_tobasistcmd:
 *  Convert selected BCurve objects to a different basis.
 *
 *  Implements the \a toBasisBC scripting interface command.
 *
 *  \returns TCL_OK in any case.
 */
int
bcurve_tobasistcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status, tcl_status;
 ay_list_object *sel = ay_selection;
 bcurve_object *bc;
 char **basisv = NULL;
 int i = 1, j = 0, basisc = 0, btype = AY_BTBSPLINE;
 int have_step = AY_FALSE, step = 0;
 double *basis = NULL;

  if(argc > 2)
    {
      /* parse args */
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-s"))
	    {
	      have_step = AY_TRUE;
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &step);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      if(step < 1 || step > 4)
		{
		  ay_error(AY_ERROR, argv[0], "Illegal step size.");
		  goto cleanup;
		}
	    }

	  if(!strcmp(argv[i], "-t"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &btype);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      if(btype < 0 || btype > AY_BTCUSTOM)
		{
		  ay_error(AY_ERROR, argv[0], "Illegal basis matrix type.");
		  goto cleanup;
		}
	    }

	  if(!strcmp(argv[i], "-b"))
	    {
	      btype = AY_BTCUSTOM;
	      Tcl_SplitList(interp, argv[i+1], &basisc, &basisv);
	      if(basisc == 16)
		{
		  if(!(basis = malloc(16*sizeof(double))))
		    {
		      ay_error(AY_EOMEM, argv[0], NULL);
		      goto cleanup;
		    }
		  for(j = 0; j < 16; j++)
		    {
		      tcl_status = Tcl_GetDouble(interp, basisv[j], &basis[j]);
		      AY_CHTCLERRGOT(tcl_status, argv[0], interp);
		    } /* for */
		}
	      else
		{
		  ay_error(AY_ERROR, argv[0], "Basis must be 16 floats.");
		  goto cleanup;
		}

	      Tcl_Free((char *) basisv);
	      basisv = NULL;
	    }

	  i += 2;
	} /* while */
    } /* if */

  if(!have_step)
    {
      switch(btype)
	{
	case AY_BTBEZIER:
	  step = 3;
	  break;
	case AY_BTBSPLINE:
	  step = 1;
	  break;
	case AY_BTHERMITE:
	  step = 2;
	  break;
	case AY_BTCATMULLROM:
	  step = 1;
	  break;
	case AY_BTPOWER:
	  step = 4;
	  break;
	default:
	  /* AY_BTCUSTOM */
	  ay_error(AY_ERROR, argv[0], "No step specified.");
	  goto cleanup;
	}
    }

  while(sel)
    {
      if(sel->object->type == bcurve_id)
	{
	  bc = (bcurve_object*)sel->object->refine;
	  ay_status = bcurve_tobasis(bc, btype, step, basis);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Conversion failed.");
	      break;
	    }
	  sel->object->modified = AY_TRUE;
	}
      if(sel->object->modified)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);
	  (void)ay_notify_object(sel->object);
	}

      sel = sel->next;
    } /* while */

  (void)ay_notify_parent();

cleanup:

  if(basis)
    free(basis);

  if(basisv)
    free(basisv);

 return TCL_OK;
} /* bcurve_tobasistcmd */


/** bcurve_insertpntcb:
 * Insert a new control point into a BCurve.
 *
 * \param[in,out] o curve object to process
 * \param[in,out] index where to store the index of the inserted point
 * \param[in] objXYZ coordinates of point after which to insert
 * \param[in] edit interactive editing support
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
bcurve_insertpntcb(ay_object *o, int *index, double *objXYZ, int edit)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object nc = {0};
 bcurve_object *bc = NULL;

  if(!o || !index || !objXYZ)
    return AY_ENULL;

  if(o->type != bcurve_id)
    return AY_ERROR;

  bc = (bcurve_object *)o->refine;
  nc.length = bc->length;
  nc.controlv = bc->controlv;

  ay_status = ay_pact_insertnc(&nc, index, objXYZ, edit);

  if(!ay_status)
    {
      bc->length = nc.length;
      bc->controlv = nc.controlv;
    }

  if(nc.knotv)
    free(nc.knotv);

 return ay_status;
} /* bcurve_insertpntcb */


/** bcurve_deletepntcb:
 * Remove a control point from a BCurve.
 *
 * \param[in,out] o curve object to process
 * \param[in,out] index where to store the index of the removed point
 * \param[in] objXYZ coordinates of point to delete
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
bcurve_deletepntcb(ay_object *o, int *index, double *objXYZ)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object nc = {0};
 bcurve_object *bc = NULL;

  if(!o || !index || !objXYZ)
    return AY_ENULL;

  if(o->type != bcurve_id)
    return AY_ERROR;

  bc = (bcurve_object *)o->refine;
  nc.length = bc->length;
  nc.controlv = bc->controlv;

  ay_status = ay_pact_deletenc(&nc, index, objXYZ);

  if(!ay_status)
    {
      bc->length = nc.length;
      bc->controlv = nc.controlv;
    }

  if(nc.knotv)
    free(nc.knotv);

 return ay_status;
} /* bcurve_deletepntcb */


/** bcurve_genericopcb:
 * Execute generic operation (AY_OP*) on BCurve.
 *
 * \param[in,out] o curve object to process
 * \param[in] op operation designation
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
bcurve_genericopcb(ay_object *o, int op)
{
 int ay_status = AY_OK, Qwlen;
 bcurve_object *bc = NULL;
 double *Qw;

  if(!o)
    return AY_ENULL;

  if(o->type != bcurve_id)
    return AY_ERROR;

  bc = (bcurve_object *)o->refine;

  switch(op)
    {
    case AY_OPREVERT:
      (void) ay_nct_revertarr(bc->controlv, bc->length, 4);
      break;
    case AY_OPOPEN:
      bc->closed = AY_FALSE;
      break;
    case AY_OPCLOSE:
      bc->closed = AY_TRUE;
      break;
    case AY_OPREFINE:
    case AY_OPCOARSEN:
      Qw = NULL;
      if(op == AY_OPREFINE)
	ay_status = ay_nct_refinearray(bc->controlv, bc->length, 4,
				       o->selp, &Qw, &Qwlen);
      else
	ay_status = ay_nct_coarsenarray(bc->controlv, bc->length, 4,
					o->selp, &Qw, &Qwlen);
      if(!ay_status && Qw)
	{
	  free(bc->controlv);
	  bc->controlv = Qw;
	  bc->length = Qwlen;
	}
      break;
    default:
      break;
    } /* switch op */

 return ay_status;
} /* bcurve_genericopcb */


/* Bcurve_Init:
 * initializes the bcurve module/plugin by registering a new
 * object type (bcurve) and loading the accompanying Tcl script file.
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Bcurve_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "bcurve_init";
 char buf[32];
 int i, ops[5] = {AY_OPREVERT, AY_OPOPEN, AY_OPCLOSE,
		  AY_OPREFINE, AY_OPCOARSEN};

#ifdef WIN32
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#endif /* WIN32 */

  if(ay_checkversion(fname, bcurve_version_ma, bcurve_version_mi))
    {
      return TCL_ERROR;
    }

  ay_status = ay_otype_register(bcurve_name,
				bcurve_createcb,
				bcurve_deletecb,
				bcurve_copycb,
				bcurve_drawcb,
				bcurve_drawhcb,
				NULL, /* no surface */
				bcurve_setpropcb,
				bcurve_getpropcb,
				bcurve_getpntcb,
				bcurve_readcb,
				bcurve_writecb,
				NULL, /* no RIB export */
				bcurve_bbccb,
				&bcurve_id);

  ay_status += ay_draw_registerdacb(bcurve_drawacb, bcurve_id);

  ay_status += ay_notify_register(bcurve_notifycb, bcurve_id);

  ay_status += ay_convert_register(bcurve_convertcb, bcurve_id);

  ay_status += ay_provide_register(bcurve_providecb, bcurve_id);

  ay_status += ay_pact_registerinsert(bcurve_insertpntcb, bcurve_id);

  ay_status += ay_pact_registerdelete(bcurve_deletepntcb, bcurve_id);

  for(i = 0; i < 5; i++)
    {
      ay_status += ay_tcmd_registergeneric(ops[i], bcurve_genericopcb,
					   bcurve_id);
    }

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }

  /* bcurve objects may not be associated with materials */
  ay_matt_nomaterial(bcurve_id);

  /* source bcurve.tcl, it contains Tcl-code to build
     the bcurve-Attributes Property GUI */
  if((Tcl_EvalFile(interp, "bcurve.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \"bcurve.tcl\"!");
       return TCL_OK;
     }

  ay_error(AY_EOUTPUT, fname,
	   "Custom object \"BCurve\" successfully loaded.");

  /* invert Bezier basis matrix */
  (void)ay_trafo_invgenmatrix(mb, mbi);

  /* invert B-Spline basis matrix */
  (void)ay_trafo_invgenmatrix(ms, msi);

  Tcl_CreateCommand(interp, "tobasisBC", bcurve_tobasistcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  arrobj = Tcl_NewStringObj(bcurve_arr, -1);
  Tcl_IncrRefCount(arrobj);

  for(i = 0; i < 16; i++)
    {
      sprintf(buf, "Basis_%d", i);
      bm[i] = Tcl_NewStringObj(buf, -1);
      Tcl_IncrRefCount(bm[i]);
    }

 return TCL_OK;
} /* Bcurve_Init */
