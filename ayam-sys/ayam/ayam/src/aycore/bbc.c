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

/* bbc.c - bounding box calculation */

/* ay_bbc_get:
 *  changes to this function also need to be applied to:
 *  objects/instance.c/ay_instance_bbccb()
 *  flag values in bbc callback mean:
 *  0 - normal bounding box
 *  1 - exclusive bounding box, discard children bounding box (e.g. NURBSPatch)
 *  2 - no own bounding box, but children have one (e.g. Level)
 *  3 - normal bounding box, but discard transformations (e.g. Instance)
 */
int
ay_bbc_get(ay_object *o, double *bbox)
{
 int ay_status = AY_OK;
 char fname[] = "bbc_get";
 ay_object *d = NULL;
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double bbt[24] = {0};
 int i, a, flags = 0;
 ay_voidfp *arr = NULL;
 ay_bbccb *cb = NULL;
 double m[16] = {0};
 int have_child_bb = AY_FALSE, have_trafo = AY_FALSE;

  if(!o || !bbox)
    return AY_ENULL;

  /* get transformations */
  if(AY_ISTRAFO(o))
    {
      ay_trafo_creatematrix(o, m);
      have_trafo = AY_TRUE;
    }

  /* get bounding boxes of children */
  if(o->down)
    {
      d = o->down;
      while(d->next)
	{
	  ay_status = ay_bbc_get(d, bbt);
	  if(!ay_status)
	    {
	      /* apply transformations */
	      if(o->inherit_trafos && have_trafo)
		{
		  ay_trafo_apply3v(bbt, 8, 3, m);
		}

	      a = 0;
	      for(i = 0; i < 8; i++)
		{
		  if(bbt[a] < xmin)
		    xmin = bbt[a];
		  if(bbt[a] > xmax)
		    xmax = bbt[a];
		  a += 3;
		}

	      a = 1;
	      for(i = 0; i < 8; i++)
		{
		  if(bbt[a] < ymin)
		    ymin = bbt[a];
		  if(bbt[a] > ymax)
		    ymax = bbt[a];
		  a += 3;
		}

	      a = 2;
	      for(i = 0; i < 8; i++)
		{
		  if(bbt[a] < zmin)
		    zmin = bbt[a];
		  if(bbt[a] > zmax)
		    zmax = bbt[a];
		  a += 3;
		}
	      have_child_bb = AY_TRUE;
	    } /* if */

	  d = d->next;
	} /* while */
    } /* if */

  if(o)
    {
      arr = ay_bbccbt.arr;
      cb = (ay_bbccb *)(arr[o->type]);
      if(cb)
	ay_status = cb(o, bbt, &flags);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "bbc callback failed");
	  return AY_ERROR;
	}
    }

  if(flags == 1)
    { /* exclusive bounding box, discard child(ren) bounding box(es) */
      xmin = DBL_MAX; xmax = -DBL_MAX; ymin = DBL_MAX;
      ymax = -DBL_MAX; zmin = DBL_MAX; zmax = -DBL_MAX;
    }

  if(flags != 2)
    { /* bounding box of object o is not marked invalid/non-existent */
      /* thus, merge bounding box of object o with child(ren) bounding box */

      if(flags != 3 && have_trafo)
	{
	  /* apply transformations */
	  ay_trafo_apply3v(bbt, 8, 3, m);
	}

      a = 0;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < xmin)
	    xmin = bbt[a];
	  if(bbt[a] > xmax)
	    xmax = bbt[a];
	  a += 3;
	}

      a = 1;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < ymin)
	    ymin = bbt[a];
	  if(bbt[a] > ymax)
	    ymax = bbt[a];
	  a += 3;
	}

      a = 2;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < zmin)
	    zmin = bbt[a];
	  if(bbt[a] > zmax)
	    zmax = bbt[a];
	  a += 3;
	}
      }
  else
    { /* o marks its bb as nonexistent */

      /* check, if we have child bb's to use */
      if(!have_child_bb)
	{
	  /* no, so we have _no_ bb at all */
	  return AY_ERROR;
	} /* if */
    } /* if */

  /* fill in results */
  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* ay_bbc_get */


/* ay_bbc_fromarr:
 *  bounding box calculation from control point array
 */
int
ay_bbc_fromarr(double *arr, int len, int stride, double *bbox)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 int i, a;

  if(!arr || !bbox)
    return AY_ENULL;

  xmin = arr[0];
  xmax = xmin;
  ymin = arr[1];
  ymax = ymin;
  zmin = arr[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < len; i++)
    {
      if(arr[a] < xmin)
	xmin = arr[a];
      if(arr[a] > xmax)
	xmax = arr[a];

      if(arr[a+1] < ymin)
	ymin = arr[a+1];
      if(arr[a+1] > ymax)
	ymax = arr[a+1];

      if(arr[a+2] < zmin)
	zmin = arr[a+2];
      if(arr[a+2] > zmax)
	zmax = arr[a+2];

      a += stride;
    }

  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* ay_bbc_fromarr */


/* ay_bbc_fromlist:
 *  bounding box calculation from list of objects
 */
int
ay_bbc_fromlist(ay_object *o, int update, double *bbox)
{
 int ay_status = AY_OK;
 int i, a;
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double bbt[24] = {0};

  if(!o || !bbox)
    return AY_ENULL;

  if(update)
    {
      xmin = bbox[0];
      xmax = bbox[6];
      ymin = bbox[13];
      ymax = bbox[1];
      zmin = bbox[5];
      zmax = bbox[2];
    }

  while(o)
    {
      ay_status = ay_bbc_get(o, bbt);

      if(ay_status)
	{
	  o = o->next;
	  continue;
	}

      a = 0;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < xmin)
	    xmin = bbt[a];
	  if(bbt[a] > xmax)
	    xmax = bbt[a];
	  a += 3;
	} /* for */

      a = 1;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < ymin)
	    ymin = bbt[a];
	  if(bbt[a] > ymax)
	    ymax = bbt[a];
	  a += 3;
	} /* for */

      a = 2;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < zmin)
	    zmin = bbt[a];
	  if(bbt[a] > zmax)
	    zmax = bbt[a];
	  a += 3;
	} /* for */

      o = o->next;
    } /* while */

  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* ay_bbc_fromlist */


/** ay_bbc_gettcmd:
 *  Get the bounding box of the selected objects;
 *  Implements the \a getBB scripting interface command.
 *  See also the corresponding section in the \ayd{scgetbb}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_bbc_gettcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
 int ay_status = AY_OK;
 int i, a;
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double bbt[24] = {0};
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 Tcl_Obj *res = NULL, *to = NULL;

  while(sel)
    {
      o = sel->object;

      ay_status = ay_bbc_get(o, bbt);

      if(ay_status)
	{
	  sel = sel->next;
	  continue;
	}

      a = 0;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < xmin)
	    xmin = bbt[a];
	  if(bbt[a] > xmax)
	    xmax = bbt[a];
	  a += 3;
	} /* for */

      a = 1;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < ymin)
	    ymin = bbt[a];
	  if(bbt[a] > ymax)
	    ymax = bbt[a];
	  a += 3;
	} /* for */

      a = 2;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < zmin)
	    zmin = bbt[a];
	  if(bbt[a] > zmax)
	    zmax = bbt[a];
	  a += 3;
	} /* for */

      sel = sel->next;
    } /* while sel */

  res = Tcl_NewListObj(0, NULL);
  if(res)
    {
      to = Tcl_NewDoubleObj(xmin);
      Tcl_ListObjAppendElement(interp, res, to);
      to = Tcl_NewDoubleObj(ymin);
      Tcl_ListObjAppendElement(interp, res, to);
      to = Tcl_NewDoubleObj(zmin);
      Tcl_ListObjAppendElement(interp, res, to);

      to = Tcl_NewDoubleObj(xmax);
      Tcl_ListObjAppendElement(interp, res, to);
      to = Tcl_NewDoubleObj(ymax);
      Tcl_ListObjAppendElement(interp, res, to);
      to = Tcl_NewDoubleObj(zmax);
      Tcl_ListObjAppendElement(interp, res, to);

      Tcl_SetObjResult(interp, res);
    }

 return TCL_OK;
} /* ay_bbc_gettcmd */

