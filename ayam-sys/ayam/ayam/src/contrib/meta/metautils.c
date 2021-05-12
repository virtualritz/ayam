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

/*
** metautils.c:
**  the stuff for calculating the metacomponents
**  2001 Frank Pagels
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "meta.h"
#include "ayam.h"

#define POS(p) (w->mgrid[p.x * w->aktcubes * w->aktcubes + p.y * w->aktcubes + p.z])

static unsigned int component_id;

/* calculate the effect for all components in list */
double
meta_calcall(double x1, double y1, double z1, meta_world *w)
{
 double effect, dist, radius, tmpeffect;
 meta_blob *tmp;
 ay_object *o;
#if 0
 ay_nurbcurve_object *nc;
 int i, j;
 double *cv, v[3];
#endif
 double x, y, z;
 Tcl_Obj *to = NULL;
 Tcl_Interp *interp = ay_safeinterp;
 static Tcl_Obj *tox = NULL, *toy = NULL, *toz = NULL;

#ifdef AYNOSAFEINTERP
  interp = ay_interp;
#endif

  if(!tox)
    {
      tox = Tcl_ObjSetVar2(interp, Tcl_NewStringObj("x", 1), NULL,
			   Tcl_NewDoubleObj(0.0),
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);;
      Tcl_IncrRefCount(tox);

      toy = Tcl_ObjSetVar2(interp, Tcl_NewStringObj("y", 1), NULL,
			   Tcl_NewDoubleObj(0.0),
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);;
      Tcl_IncrRefCount(toy);

      toz = Tcl_ObjSetVar2(interp, Tcl_NewStringObj("z", 1), NULL,
			   Tcl_NewDoubleObj(0.0),
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);;
      Tcl_IncrRefCount(toz);
    }

  effect = 0;
  dist = 0;

  o = w->o;

  while(o->next != NULL)
    {
      if(o->type == component_id)
	{
	  tmp = (meta_blob *) o->refine;
	  radius = tmp->r * tmp->r;

	  /* rotate and scale */
	  x = (tmp->rm[0] * x1 + tmp->rm[4] * y1 + tmp->rm[8] * z1 +
	       tmp->rm[12] * 1.0);
	  y = (tmp->rm[1] * x1 + tmp->rm[5] * y1 + tmp->rm[9] * z1 +
	       tmp->rm[13] * 1.0);
	  z = (tmp->rm[2] * x1 + tmp->rm[6] * y1 + tmp->rm[10] * z1 +
	       tmp->rm[14] * 1.0);

	  if (!((tmp->formula == META_BALL) && (w->version == 1)))
	    {
	      x *= tmp->scalex;
	      y *= tmp->scaley;
	      z *= tmp->scalez;
	    }


	  /* the normal metaball */
	  if (tmp->formula == META_BALL)
	    {
	      if(w->version == 1)
		dist = META_DIST(x, y, z, tmp->cp.x, tmp->cp.y, tmp->cp.z);
	      else
		dist = META_DIST2(x, y, z, tmp->cp.x, tmp->cp.y, tmp->cp.z);

	      if (dist <= radius)
		{
		  tmpeffect = tmp->a * META_CUB(dist) / META_CUB(radius) +
		    tmp->b * META_SQ(dist) / META_SQ(radius) +
		    tmp->c * dist / radius + 1.0;

		  if (tmp->negativ)
		    {
		      effect -= tmpeffect;
		    }
		  else
		    {
		      effect += tmpeffect;
		    }
		}
	    } /* if ball */


	  /* a cube */
	  if (tmp->formula == META_CUBE)
	    {

	      tmpeffect = (pow(META_ABS(x - tmp->cp.x),tmp->ex) +
			   pow(META_ABS(y - tmp->cp.y),tmp->ey) +
			   pow(META_ABS(z - tmp->cp.z),tmp->ez)) * 9000.0;

	      tmpeffect = 1.0/(tmpeffect < 0.00001 ? 0.00001 : tmpeffect);

	      if (tmp->negativ)
		{
		  effect -= tmpeffect;
		}
	      else
		{
		  effect += tmpeffect;
		}
	    } /* if cube */


	  /* a torus */
	  if (tmp->formula == META_TORUS)
	    {
	      if (tmp->rot)
		{
		  tmpeffect = META_SQ(META_SQ(x - tmp->cp.x) +
				      META_SQ(y - tmp->cp.y) +
				      META_SQ(z - tmp->cp.z) +
                                      tmp->Ro * tmp->Ro - tmp->Ri * tmp->Ri) -
		                      4.0 * META_SQ(tmp->Ro) *
                             (META_SQ(z - tmp->cp.z) + META_SQ(y - tmp->cp.y));
		}
	      else
		{
		  tmpeffect = META_SQ(META_SQ(x - tmp->cp.x) +
				      META_SQ(y - tmp->cp.y) +
				      META_SQ(z - tmp->cp.z) +
				      tmp->Ro * tmp->Ro - tmp->Ri * tmp->Ri) -
		                      4.0 * META_SQ(tmp->Ro) *
		             (META_SQ(x - tmp->cp.x) + META_SQ(y - tmp->cp.y));
		}

	      if (tmp->negativ)
		{
		  effect -= 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) *
		    0.006;
		}
	      else
		{
		  effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) *
		    0.006;
		}
	    } /* if torus */


          /* a heart */
	  if (tmp->formula == META_HEART)
	    {

	      tmpeffect = META_CUB (2 * META_SQ (x - tmp->cp.x) +
		    META_SQ (y - tmp->cp.y) + META_SQ (z - tmp->cp.z) - 1) -
		    (0.1 * META_SQ (x - tmp->cp.x) + META_SQ (y - tmp->cp.y)) *
		    META_CUB (z - tmp->cp.z);

	      if (tmp->negativ)
		{
	          effect -= 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) *
		    0.002;
		}
	      else
		{
		  effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) *
		    0.002;
		}
	    } /* if heart */


	  /* a custom formula */
	  if (tmp->formula == META_CUSTOM)
	    {
	      tox->internalRep.doubleValue = x - tmp->cp.x;
	      toy->internalRep.doubleValue = y - tmp->cp.y;
	      toz->internalRep.doubleValue = z - tmp->cp.z;

	      if(tmp->expression)
		{
		  Tcl_GlobalEvalObj(interp, tmp->expression);
		}

	      to = Tcl_GetObjResult(interp);

	      tmpeffect = to->internalRep.doubleValue;

	      if(tmp->negativ)
		{
		  effect -= 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect);
		}
	      else
		{
		  effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect);
		}
	    } /* if custom */
	} /* if is meta component */
#if 0
      if(o->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object *)o->refine;
	  cv = nc->controlv;
	  j = 0;
	  tmpeffect = 0;
	  y = 2000;
	  for(i = 0; i < nc->length; i++)
	    {
	      v[0] = x1-cv[0];
	      v[1] = y1-cv[1];
	      v[2] = z1-cv[2];
	      x = AY_V3LEN(v);
	      if(x > 0.0 && x < 1)
		{
		  if(y > x)
		    y = x;
		  j++;
		}
	      cv += 4;
	    }


	  tmpeffect += 1.0/y;

	  /*
	  if(j > 0)
	    tmpeffect /= j;
	  */
	  effect += tmpeffect;
	}
#endif
      o = o->next;
    } /* while */

 return effect;
} /* meta_calcall */


void
meta_getstart (meta_blob * b, meta_intxyz * p, meta_world * w)
{

  p->x = (int) (b->cp.x / w->edgelength) + w->aktcubes / 2;

  if (b->formula == META_TORUS)
    p->y = (int) ((b->cp.y + b->Ro) / w->edgelength) + w->aktcubes / 2;
  else
    p->y = (int) (b->cp.y / w->edgelength) + w->aktcubes / 2;

  p->z = (int) (b->cp.z / w->edgelength) + w->aktcubes / 2;

  if (p->x < 0)
    p->x = 0;

  if (p->x > w->aktcubes - 1)
    p->x = (w->aktcubes - 1);

  if (p->y < 0)
    p->y = 0;

  if (p->y > w->aktcubes - 1)
    p->y = (w->aktcubes - 1);

  if (p->z < 0)
    p->z = 0;

  if (p->z > w->aktcubes - 1)
    p->z = (w->aktcubes - 1);

 return;
} /* meta_getstart */

int
meta_initcubestack (meta_world * w)
{

  if ((w->stack = (meta_gridcell *) calloc (1, sizeof (meta_gridcell) * 2000)))
    {
      w->stackpos = 0;
      w->maxstack = 2000;
      return AY_OK;
    }
  else
    {
      return AY_EOMEM;
    }
} /* meta_initcubestack */

int
meta_freecubestack (meta_world * w)
{
  if (w->stack)
    free (w->stack);

 return AY_OK;
} /* meta_freecubestack */

void
meta_pushcube (meta_gridcell * cube, meta_world * w)
{
  if (w->stackpos == w->maxstack)
    {
      w->stack = realloc (w->stack, sizeof (meta_gridcell) *
			  (w->maxstack + 1000));
      w->maxstack += 1000;
    }

  w->stack[w->stackpos] = *cube;
  w->stackpos++;

 return;
} /* meta_pushcube */


meta_gridcell
meta_popcube (meta_world * w)
{

  w->stackpos--;

 return (w->stack[w->stackpos]);
} /* meta_popcube */


void
meta_initstartcube (meta_world * w, meta_gridcell * cube, meta_intxyz * p)
{

#define length w->edgelength

  cube->p[0].x = p->x * length - w->unisize / 2;
  cube->p[0].y = p->y * length - w->unisize / 2;
  cube->p[0].z = p->z * length - w->unisize / 2;
  cube->val[0] = meta_calcall (cube->p[0].x, cube->p[0].y, cube->p[0].z, w);

  cube->p[1].x = cube->p[0].x + length;
  cube->p[1].y = cube->p[0].y;
  cube->p[1].z = cube->p[0].z;
  cube->val[1] = meta_calcall (cube->p[1].x, cube->p[1].y, cube->p[1].z, w);

  cube->p[2].x = cube->p[1].x;
  cube->p[2].y = cube->p[0].y;
  cube->p[2].z = cube->p[0].z + length;
  cube->val[2] = meta_calcall (cube->p[2].x, cube->p[2].y, cube->p[2].z, w);

  cube->p[3].x = cube->p[0].x;
  cube->p[3].y = cube->p[0].y;
  cube->p[3].z = cube->p[2].z;
  cube->val[3] = meta_calcall (cube->p[3].x, cube->p[3].y, cube->p[3].z, w);

  cube->p[4].x = cube->p[0].x;
  cube->p[4].y = cube->p[0].y + length;
  cube->p[4].z = cube->p[0].z;
  cube->val[4] = meta_calcall (cube->p[4].x, cube->p[4].y, cube->p[4].z, w);

  cube->p[5].x = cube->p[1].x;
  cube->p[5].y = cube->p[4].y;
  cube->p[5].z = cube->p[0].z;
  cube->val[5] = meta_calcall (cube->p[5].x, cube->p[5].y, cube->p[5].z, w);

  cube->p[6].x = cube->p[1].x;
  cube->p[6].y = cube->p[4].y;
  cube->p[6].z = cube->p[2].z;
  cube->val[6] = meta_calcall (cube->p[6].x, cube->p[6].y, cube->p[6].z, w);

  cube->p[7].x = cube->p[0].x;
  cube->p[7].y = cube->p[4].y;
  cube->p[7].z = cube->p[2].z;
  cube->val[7] = meta_calcall (cube->p[7].x, cube->p[7].y, cube->p[7].z, w);

#undef length

 return;
} /* meta_initstartcube */


void
meta_addneighbors (meta_gridcell * cube, meta_world * w)
{
 int edgecode;
 int square;
 int pos;
 meta_gridcell tmpcube;

#define act w->aktcubes

  square = act * act;

  edgecode = w->edgecode;

  /* check back */
  if (!(edgecode & 4080) || (edgecode & 1 << 9) || (edgecode & 1 << 4)
      || (edgecode & 1 << 8))
    {
      if (cube->pos.z > 0)
	{
	  pos = cube->pos.x * square + act * cube->pos.y + (cube->pos.z - 1);

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveback (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;
	    }
	}
    }

  /* check right */
  if ((edgecode & 1) || (edgecode & 1 << 9) || (edgecode & 1 << 5)
      || (edgecode & 1 << 10))
    {
      if (cube->pos.x < act - 1)
	{
	  pos = (cube->pos.x + 1) * square + act * cube->pos.y + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveright (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;
	    }
	}
    }

  /* check front */
  if ((edgecode & 1 << 2) || (edgecode & 1 << 6) || (edgecode & 1 << 10)
      || (edgecode & 1 << 11))
    {
      if (cube->pos.z < act - 1)
	{
	  pos = cube->pos.x * square + act * cube->pos.y + (cube->pos.z + 1);

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_movefront (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;
	    }
	}
    }

  /* check left */
  if ((edgecode & 1 << 3) || (edgecode & 1 << 7) || (edgecode & 1 << 8)
      || (edgecode & 1 << 11))
    {
      if (cube->pos.x > 0)
	{

	  pos = (cube->pos.x - 1) * square + act * cube->pos.y + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveleft (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;
	    }
	}
    }

  /* check up */
  if ((edgecode & 1 << 4) || (edgecode & 1 << 5) || (edgecode & 1 << 6)
      || (edgecode & 1 << 7))
    {
      if (cube->pos.y < act - 1)
	{
	  pos = cube->pos.x * square + act * (cube->pos.y + 1) + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveup (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;
	    }
	}
    }

  /* check down */
  if (!(edgecode & 4080) || (edgecode & 1) || (edgecode & 1 << 2)
      || (edgecode & 1 << 3))
    {
      if (cube->pos.y > 0)
	{
	  pos = cube->pos.x * square + act * (cube->pos.y - 1) + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_movedown (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;
	    }
	}
    }

#undef act

 return;
} /* meta_addneighbors */


int
meta_searchcube (meta_gridcell * cube, meta_intxyz * p, meta_world * w)
{
 int code;

  code = 0;

  /* search up */
  do
    {
      if (p->y < w->aktcubes - 1)
	{
	  p->y++;
	  /* move one position up */
	  meta_moveup (cube, w);

	  code = meta_polygonise (w, cube, w->isolevel);
	}
      else
	break;
    }
  while (code == 0);

  /* search down */
  if (code == 0)
    {
      do
	{
	  if (p->y > 0)
	    {
	      p->y--;
	      /* move one position down */
	      meta_movedown (cube, w);

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);
    }

  /* search left */
  if (code == 0)
    {
      do
	{
	  if (p->x > 0)
	    {
	      p->x--;
	      /* move one position left */
	      meta_moveleft (cube, w);

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);
    }

  /* search right */
  if (code == 0)
    {
      do
	{
	  if (p->x < w->aktcubes - 1)
	    {
	      p->x++;
	      /* move one position up */
	      meta_moveright (cube, w);

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);
    }

  /* search front */
  if (code == 0)
    {
      do
	{
	  if (p->z < w->aktcubes - 1)
	    {
	      p->z++;
	      /* move one position to the front */
	      meta_movefront (cube, w);

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);
    }

  /* search back */
  if (code == 0)
    {
      do
	{
	  if (p->z > 0)
	    {
	      p->z--;
	      /* move one position to the back */
	      meta_moveback (cube, w);

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);
    }

 return (code);
} /* meta_searchcube */


int
meta_calceffect (meta_world * w)
{
 meta_blob *b;
 meta_intxyz p;
 int code;
 ay_object *o;
 meta_gridcell cube;
 double *t;

  o = w->o;

  w->lastmark++;
  w->stackpos = 0;

#if META_USEVERTEXARRAY
  /* Reset Hash */
  memset(w->vhash,0,(sizeof (int) * ((w->tablesize-1) + (w->tablesize/10 -1) + (w->tablesize/100 -1))));
  w->h1 = w->tablesize/10;
  w->h2 = w->tablesize/100;
  w->actindex = 0;
  w->indexnum = 0;
#endif

  while (o->next != NULL)
    {
      if(o->type == component_id)
	{
	  b = (meta_blob *) o->refine;

	  /* get startcube for component */
	  meta_getstart (b, &p, w);

    	  /* fill first cube with values */
	  meta_initstartcube (w, &cube, &p);

	  /* mark that cube is visited */
	  POS (p) = w->lastmark;

	  code = meta_polygonise (w, &cube, w->isolevel);

	  /* search for the first cube */
	  code = meta_searchcube (&cube, &p, w);

	  /* component already calculated ? */
	  if (POS (p) == w->lastmark)
	    {
	      o = o->next;
	      /* test next component */
	      continue;
	    }

	  /* mark position of the cube */
	  cube.pos = p;

	  /* addneighbors cubes to stack */
	  meta_addneighbors (&cube, w);

	  while (w->stackpos > 0)
	    {
	      /* get next cubepos */
	      w->stackpos--;

	      cube = w->stack[w->stackpos];

	      if (w->currentnumpoly+150 >= (w->maxpoly))
	    	{
		  if (! (t = realloc (w->vertex,
			 sizeof (double) * 3 * 3 * (w->maxpoly + 10000 + 20))))
		    {
		      return AY_EOMEM;
		    }
		  else
		    {
		      w->vertex = t;
		    }

		  if (! (t = realloc (w->nvertex,
			 sizeof (double) * 3 * 3 * (w->maxpoly + 10000 + 20))))
		    {
		      return AY_EOMEM;
		    }
		  else
		    {
		      w->nvertex = t;
		    }

		  w->maxpoly += 10000;
		}

	      code = meta_polygonise (w, &cube, w->isolevel);

	      /* mark that cube is visited */
	      POS (cube.pos) = w->lastmark;

	      if ((code != 0) || (code == 300))
		{
		  /* add neighbors cubes to stack */
		  meta_addneighbors (&cube, w);
		}
	    } /* while stack */
	} /* if is meta component */
      o = o->next;
    } /* while */

 return AY_OK;
} /* meta_calceffect */


void
meta_getnormal (meta_world * w, meta_xyz * p, meta_xyz * normal)
{
 double xn, yn, zn, old, scale;
 double d;

  d = (w->edgelength / 500);  /**w->scale;*/

  xn = (meta_calcall(p->x+d, p->y, p->z,w) -
	meta_calcall(p->x-d, p->y, p->z,w))/(2*d);
  yn = (meta_calcall(p->x, p->y+d, p->z,w) -
	meta_calcall(p->x, p->y-d, p->z,w))/(2*d);
  zn = (meta_calcall(p->x, p->y, p->z+d,w) -
	meta_calcall(p->x, p->y, p->z-d,w))/(2*d);

/*
  xn = (meta_calcall (p->x + d, p->y, p->z, w) - f) / d;
  yn = (meta_calcall (p->x, p->y + d, p->z, w) - f) / d;
  zn = (meta_calcall (p->x, p->y, p->z + d, w) - f) / d;
*/

  old = sqrt (META_SQ (xn) + META_SQ (yn) + META_SQ (zn));

  if (old != 0.0)
    {
      scale = 1.0 / old;
      normal->x = xn * scale;
      normal->y = yn * scale;
      normal->z = zn * scale;
    }
  else
    {
      normal->x = xn;
      normal->y = yn;
      normal->z = zn;
    }

 return;
} /* meta_getnormal */


void
metautils_init(unsigned int cid)
{

  component_id = cid;

 return;
} /* metautils_init */

