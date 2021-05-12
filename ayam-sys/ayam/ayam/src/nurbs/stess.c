/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2007 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* stess.c simple NURB tesselators */

/* local preprocessor definitions: */
#define AY_STESSEPSILON 0.000001
/*#define AY_STESSDBG 1*/

/* prototypes of functions local to this module: */
void ay_stess_FindMultiplePoints(int n, int p, double *U, double *P,
				 int dim, int is_rat, int stride,
				 int *m, double **V);

int ay_stess_IntersectLines2D(double *p1, double *p2, double *p3, double *p4,
			      double *ip);

void ay_stess_TessTrimCurve(ay_object *o, double **tts, int *tls, int *tds,
			    int *i, double qf);

void ay_stess_TessLinearTrimCurve(ay_object *o, double **tts, int *tls,
				  int *tds, int *i);

int ay_stess_MergeUVectors(ay_stess_uvp *a, ay_stess_uvp *b);

int ay_stess_MergeVVectors(ay_stess_uvp *a, ay_stess_uvp *b);

void ay_stess_SortIntersections(ay_stess_uvp *list, int u);

int ay_stess_TessTrimmedNPU(ay_object *o, int qf, int numtrims,
			    double **tcs, int *tcslens, int *tcsdirs,
			    int *reslen, ay_stess_uvp ***result);

int ay_stess_TessTrimmedNPV(ay_object *o, int qf, int numtrims,
			    double **tcs, int *tcslens, int *tcsdirs,
			    int *reslen, ay_stess_uvp ***result);

int ay_stess_AddBoundaryTrim(ay_object *o);


/* local variables: */
ay_object *ay_stess_boundarytrim = NULL;


/* functions: */

/* ay_stess_destroy:
 *  properly destroy an stess patch object
 */
void
ay_stess_destroy(ay_stess_patch *stess)
{
 ay_voidfp *arr = NULL;
 ay_deletecb *cb = NULL;
 ay_stess_uvp *p = NULL;
 int i;

  if(!stess)
    return;

  if(stess->tessv)
    free(stess->tessv);

  if(stess->ups)
    {
      for(i = 0; i < stess->upslen; i++)
	{
	  while(stess->ups[i])
	    {
	      p = stess->ups[i]->next;
	      free(stess->ups[i]);
	      stess->ups[i] = p;
	    }
	}
      free(stess->ups);
    }

  if(stess->vps)
    {
      for(i = 0; i < stess->vpslen; i++)
	{
	  while(stess->vps[i])
	    {
	      p = stess->vps[i]->next;
	      free(stess->vps[i]);
	      stess->vps[i] = p;
	    }
	}
      free(stess->vps);
    }

  if(stess->tcslens)
    free(stess->tcslens);

  if(stess->tcspnts)
    free(stess->tcspnts);

  if(stess->pomesh)
    {
      arr = ay_deletecbt.arr;
      cb = (ay_deletecb *)(arr[AY_IDPOMESH]);
      if(cb)
	(void)cb(stess->pomesh);
    }

  memset(stess, 0, sizeof(ay_stess_patch));

 return;
} /* ay_stess_destroy */


/* ay_stess_GetQF:
 *  calculate stess quality factor (QF) from GLU sampling tolerance (GST)
 *
 */
int
ay_stess_GetQF(double gst)
{
 static double oldgst = 0.0;
 static int oldqf;
 int qf = 1;
 double base = 50.0;

  if(gst == 0.0)
    {
      oldgst = 0.0;
      return 0;
    }

  if(gst == oldgst)
    {
      return oldqf;
    }

  while(gst < base)
    {
      base /= 2.0;
      qf *= 2;
    }

  oldqf = qf;
  oldgst = gst;

 return qf;
} /* ay_stess_GetQF */


/* ay_stess_FindMultiplePoints:
 *  find all consecutive multiple (equal) control points in the
 *  NURBS curve <n>, <p>, <U>, <P> of dimensionality <dim> (2/3),
 *  rational coordinates <is_rat> (0/1), and coordinate size <stride> (4);
 *  returns corresponding parametric values in <V[<m>]>
 */
void
ay_stess_FindMultiplePoints(int n, int p, double *U, double *P,
			    int dim, int is_rat, int stride,
			    int *m, double **V)
{
 int i, j, eq;
 double *p1, *p2, *t;

  if(!U || !P || !m || !V)
    return;

  *m = 0;
  *V = NULL;

  p1 = P;

  for(i = 0; i < n-p; i++)
    {
      eq = AY_TRUE;

      p2 = p1 + stride;
      for(j = 0; j < p-1; j++)
	{
	  if((fabs(p1[0] - p2[0]) > AY_EPSILON) ||
	     ((dim > 1) && (fabs(p1[1] - p2[1]) > AY_EPSILON)) ||
	     ((dim > 2) && (fabs(p1[2] - p2[2]) > AY_EPSILON)))
	    {
	      eq = AY_FALSE;
	      break;
	    }
	  if(eq && is_rat)
	    {
	      if(fabs(p1[3]-p2[3]) > AY_EPSILON)
		{
		  eq = AY_FALSE;
		  break;
		}
	    } /* if */

	  p2 += stride;
	} /* for */

      if(eq)
	{
	  if(!(t = realloc(*V, ((*m)+1)*sizeof(double))))
	    {
	      if(*V)
		free(*V);
	      *m = 0; *V = NULL;
	      return;
	    }
	  *V = t;
	  (*V)[(*m)] = U[i + p + 1];
	  (*m)++;
	  p1 += ((p-1)*stride);
	  i += (p-1);
	}
      else
	{
	  p1 += stride;
	} /* if */
    } /* for */

 return;
} /* ay_stess_FindMultiplePoints */


/* ay_stess_CurvePoints2D:
 *   calculate all points of a 2D curve (TrimCurve)
 */
int
ay_stess_CurvePoints2D(int n, int p, double *U, double *Pw, int stride,
		       int is_rat, int qf,
		       int *Clen, double **C)
{
 int span, j, k, l, m, mc = 0, vi, incu, mc1 = 0;
 double *N = NULL, Cw[3], *Ct = NULL, u, ud, u1, *V = NULL;

  if(!U || !Pw || !Clen || !C)
    return AY_ENULL;

  if(!(N = calloc(3*(p+1), sizeof(double))))
    return AY_EOMEM;

  ay_stess_FindMultiplePoints(n, p, U, Pw, 2, is_rat, stride, &mc, &V);

  *Clen = ((4 + n) * qf);

  if(*C)
    {
      free(*C);
      *C = NULL;
    }

  if(!(Ct = calloc((*Clen + mc) * 2, sizeof(double))))
    {
      free(N);
      if(V)
	free(V);
      return AY_EOMEM;
    }
  m = 0;
  ud = (U[n]-U[p])/((*Clen)-1);
  u = U[p];
  vi = 0;
  if(is_rat)
    {
      /* curve is rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;

	  if(u1 > U[n])
	    u1 = U[n];

	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u? (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */
	  span = ay_nb_FindSpan(n-1, p, u1, U);

	  ay_nb_BasisFunsM(span, u1, p, U, N);

	  memset(Cw, 0, 3*sizeof(double));

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*stride;
	      Cw[0] = Cw[0] + N[j]*(Pw[k]*Pw[k+2]);
	      Cw[1] = Cw[1] + N[j]*(Pw[k+1]*Pw[k+2]);
	      Cw[2] = Cw[2] + N[j]*Pw[k+2];
	    }

	  Ct[m]	  = Cw[0]/Cw[2];
	  Ct[m+1] = Cw[1]/Cw[2];

#ifdef AY_STESSDBG
	  if(Ct[m] != Ct[m] || Ct[m+1] != Ct[m+1])
	    {
	      printf("NAN, at u %lg!\n",u);
	    }
#endif

	  m += 2;

	  if(incu)
	    u += ud;
	} /* for */
    }
  else
    {
      /* curve is not rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;

	  if(u1 > U[n])
	    u1 = U[n];

	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{ /* yes */
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */

	  span = ay_nb_FindSpan(n-1, p, u1, U);

	  ay_nb_BasisFunsM(span, u1, p, U, N);

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*stride;
	      Ct[m]   = Ct[m]	+ N[j]*Pw[k];
	      Ct[m+1] = Ct[m+1] + N[j]*Pw[k+1];
	    }

#ifdef AY_STESSDBG
	  if(Ct[m] != Ct[m] || Ct[m+1] != Ct[m+1])
	    {
	      printf("NAN, at u %lg!\n",u);
	    }
#endif

	  m += 2;

	  if(incu)
	    u += ud;
	} /* for */

    } /* if */

  *C = Ct;
  *Clen += mc1;

  free(N);
  if(V)
    free(V);

 return AY_OK;
} /* ay_stess_CurvePoints2D */


/* ay_stess_CurvePoints3D:
 *   calculate all points of a 3D curve
 */
int
ay_stess_CurvePoints3D(int n, int p, double *U, double *Pw, int is_rat, int qf,
		       int *Clen, double **C)
{
 int span, j, k, l, m, mc = 0, vi, incu, mc1 = 0;
 double *N = NULL, Cw[4], *Ct = NULL, u, ud, u1, *V;

  if(!U || !Pw || !Clen || !C)
    return AY_ENULL;

  if(!(N = calloc(3*(p+1), sizeof(double))))
    return AY_EOMEM;

  ay_stess_FindMultiplePoints(n, p, U, Pw, 3, is_rat, 4, &mc, &V);

  *Clen = ((4 + n) * qf);

  if(!(Ct = calloc((*Clen + mc) * 3, sizeof(double))))
    {
      free(N);
      if(V)
	free(V);
      return AY_EOMEM;
    }

  m = 0;
  ud = (U[n]-U[p])/((*Clen)-1);
  u = U[p];
  vi = 0;
  if(is_rat)
    {
      /* curve is rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;
	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u? (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */

	  span = ay_nb_FindSpan(n-1, p, u1, U);

	  ay_nb_BasisFunsM(span, u1, p, U, N);

	  memset(Cw, 0, 4*sizeof(double));

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*4;
	      Cw[0] = Cw[0] + N[j]*Pw[k]*Pw[k+3];
	      Cw[1] = Cw[1] + N[j]*Pw[k+1]*Pw[k+3];
	      Cw[2] = Cw[2] + N[j]*Pw[k+2]*Pw[k+3];
	      Cw[3] = Cw[3] + N[j]*Pw[k+3];
	    }

	  Ct[m]	  = Cw[0]/Cw[3];
	  Ct[m+1] = Cw[1]/Cw[3];
	  Ct[m+2] = Cw[2]/Cw[3];

	  m += 3;
	  if(incu)
	    u += ud;
	} /* for */
    }
  else
    {
      /* curve is not rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;
	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{ /* yes */
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */

	  span = ay_nb_FindSpan(n-1, p, u1, U);

	  ay_nb_BasisFunsM(span, u1, p, U, N);

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*4;
	      Ct[m]   = Ct[m]	+ N[j]*Pw[k];
	      Ct[m+1] = Ct[m+1] + N[j]*Pw[k+1];
	      Ct[m+2] = Ct[m+2] + N[j]*Pw[k+2];
	    }

	  m += 3;
	  if(incu)
	    u += ud;
	} /* for */
    } /* if */

  *C = Ct;
  *Clen += mc1;

  free(N);
  if(V)
    free(V);

 return AY_OK;
} /* ay_stess_CurvePoints3D */


/* ay_stess_SurfacePoints3D:
 *   calculate all points of an untrimmed NURBS surface
 */
int
ay_stess_SurfacePoints3D(int n, int m, int p, int q, double *U, double *V,
			 double *P, int qf, int *Cn, int *Cm, double **C)
{
 int ay_status = AY_OK;
 int spanu = 0, spanv = 0, j = 0;
 int a, b;
 double u, v, ud, vd;
 double temp[3] = {0}, *Ct = NULL, *fder = NULL, *fd1, *fd2;
 int *spanus = NULL, *spanvs = NULL;

  *Cn = (4 + n) * qf;
  ud = (U[n] - U[p]) / ((*Cn) - 1);

  *Cm = (4 + m) * qf;
  vd = (V[m] - V[q]) / ((*Cm) - 1);

  if(!(Ct = calloc((*Cn)*(*Cm)*6, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(spanus = calloc((*Cn)+(*Cm), sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  spanvs = spanus + (*Cn);

  if(!(fder = malloc(ay_nb_FirstDerSurf3DMSize(p, q)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  fd1 = &(fder[3]);
  fd2 = &(fder[6]);

  /* employ linear variants of FindSpan() as they are much faster
     than a binary search; especially, since we calculate
     spans for all parameters in order */

  u = U[p];
  spanu = p;
  for(a = 0; a < (*Cn)-1; a++)
    {
      /*
	if(u < U[p+1])
	{
	spanus[a] = p;
	}
	else
	{
	while(u >= U[spanu])
	{
	spanu++;
	}
	spanus[a] = spanu-1;
	}
      */

      if(u > U[p+1])
	{
	  while(u > U[spanu+1])
	    {
	      spanu++;
	    }
	}
      spanus[a] = spanu;

      u += ud;
    }
  spanus[a] = spanus[a-1];

  v = V[q];
  spanv = q;
  for(a = 0; a < (*Cm)-1; a++)
    {
      /*
	if(v < V[q+1])
	{
	spanvs[a] = q;
	}
	else
	{
	while(v > V[spanv])
	{
	spanv++;
	}
	spanvs[a] = spanv-1;
	}
      */

      if(v > V[q+1])
	{
	  while(v > V[spanv+1])
	    {
	      spanv++;
	    }
	}
      spanvs[a] = spanv;
      v += vd;
    }
  spanvs[a] = spanvs[a-1];

  u = U[p];
  for(a = 0; a < (*Cn); a++)
    {
      spanu = spanus[a];
      v = V[q];
      for(b = 0; b < (*Cm); b++)
	{
	  spanv = spanvs[b];

	  /* calculate point and normal */
	  fder[0] = (double)spanu;
	  fder[1] = (double)spanv;
	  fder[2] = 0.0;
	  fder[3] = 0.0;
	  ay_nb_FirstDerSurf3DM(n-1, m-1, p, q, U, V, P, u, v, fder);
	  memcpy(&(Ct[j]), fder, 3*sizeof(double));

	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(Ct[j+3]), temp, 3*sizeof(double));

	  j += 6;
	  v += vd;
	} /* for */

      u += ud;
    } /* for */

  *C = Ct;
  Ct = NULL;

cleanup:

  if(Ct)
    free(Ct);

  if(fder)
    free(fder);

  if(spanus)
    free(spanus);

 return ay_status;
} /* ay_stess_SurfacePoints3D */


/* ay_stess_SurfacePoints4D:
 *   calculate all points of an untrimmed NURBS surface with weights
 */
int
ay_stess_SurfacePoints4D(int n, int m, int p, int q, double *U, double *V,
			 double *Pw, int qf, int *Cn, int *Cm, double **C)
{
 int ay_status = AY_OK;
 int spanu = 0, spanv = 0, j = 0;
 int a, b;
 double u, v, ud, vd;
 double *Ct = NULL, temp[3] = {0}, *fder = NULL, *fd1, *fd2;
 int *spanus = NULL, *spanvs = NULL;

  *Cn = (4 + n) * qf;
  ud = (U[n] - U[p]) / ((*Cn) - 1);

  *Cm = (4 + m) * qf;
  vd = (V[m] - V[q]) / ((*Cm) - 1);

  if(!(spanus = calloc((*Cn)+(*Cm), sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  spanvs = spanus + (*Cn);

  if(!(fder = malloc(ay_nb_FirstDerSurf4DMSize(p, q)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(Ct = calloc((*Cn)*(*Cm)*6, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  fd1 = &(fder[3]);
  fd2 = &(fder[6]);

  /* employ linear variants of FindSpan() as they are much faster
     than a binary search; especially, since we calculate
     spans for all parameters in order */

  u = U[p];
  spanu = p;
  for(a = 0; a < (*Cn)-1; a++)
    {
      /*
	if(u < U[p+1])
	{
	spanus[a] = p;
	}
	else
	{
	while(u > U[spanu])
	{
	spanu++;
	}
	spanus[a] = spanu-1;
	}
      */

      if(u > U[p+1])
	{
	  while(u > U[spanu+1])
	    {
	      spanu++;
	    }
	}
      spanus[a] = spanu;
      u += ud;
    }
  spanus[a] = spanus[a-1];


  v = V[q];
  spanv = q;
  for(a = 0; a < (*Cm)-1; a++)
    {
      /*
	if(v < V[q+1])
	{
	spanvs[a] = q;
	}
	else
	{
	while(v > V[spanv])
	{
	spanv++;
	}
	spanvs[a] = spanv-1;
	}
      */

      if(v > V[q+1])
	{
	  while(v > V[spanv+1])
	    {
	      spanv++;
	    }
	}
      spanvs[a] = spanv;
      v += vd;
    }
  spanvs[a] = spanvs[a-1];

  u = U[p];
  for(a = 0; a < (*Cn); a++)
    {
      spanu = spanus[a];
      v = V[q];

      for(b = 0; b < (*Cm); b++)
	{
	  spanv = spanvs[b];

	  /* calculate point and normal */
	  fder[0] = (double)spanu;
	  fder[1] = (double)spanv;
	  fder[2] = 0.0;
	  fder[3] = 0.0;
	  ay_nb_FirstDerSurf4DM(n-1, m-1, p, q, U, V, Pw, u, v, fder);
	  memcpy(&(Ct[j]), fder, 3*sizeof(double));

	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(Ct[j+3]), temp, 3*sizeof(double));

	  j += 6;
	  v += vd;
	} /* for */

      u += ud;
    } /* for */

  /* return result */
  *C = Ct;
  Ct = NULL;

cleanup:

  if(Ct)
    free(Ct);

  if(fder)
    free(fder);

  if(spanus)
    free(spanus);

 return ay_status;
} /* ay_stess_SurfacePoints4D */


/* ay_stess_IntersectLines2D:
 *  Code taken from the c.g.algorithms FAQ, which in turn points to:
 *  Graphics Gems III pp. 199-202 "Faster Line Segment Intersection"
 *
 *  Input:   p1[2],p2[2]; p3[2],p4[2] - 2 2D line segments
 *  Returns: 0 - no intersection in the given segments
 *	     1 - segments intersect in point ip
 */
int
ay_stess_IntersectLines2D(double *p1, double *p2, double *p3, double *p4,
			  double *ip)
{
 double r, s, den;

  if(((fabs(p1[0]-p3[0])<=2*AY_EPSILON)&&(fabs(p1[1]-p3[1])<=2*AY_EPSILON)) ||
     ((fabs(p1[0]-p4[0])<=2*AY_EPSILON)&&(fabs(p1[1]-p4[1])<=2*AY_EPSILON)))
    {
      ip[0] = p1[0];
      ip[1] = p1[1];
      return 1;
    }

  if(((fabs(p2[0]-p3[0])<=2*AY_EPSILON)&&(fabs(p2[1]-p3[1])<=2*AY_EPSILON)) ||
     ((fabs(p2[0]-p4[0])<=2*AY_EPSILON)&&(fabs(p2[1]-p4[1])<=2*AY_EPSILON)))
    {
      ip[0] = p2[0];
      ip[1] = p2[1];
      return 1;
    }

  if(fabs(p1[0]-p2[0]) <= 2*AY_EPSILON)
    {
      if(fabs(p1[0]-p3[0]) <= 2*AY_EPSILON)
	{
	  ip[0] = p3[0];
	  ip[1] = p3[1];
	  return 1;
	}
      if(fabs(p1[0]-p4[0]) <= 2*AY_EPSILON)
	{
	  ip[0] = p4[0];
	  ip[1] = p4[1];
	  return 1;
	}
    }

  if(fabs(p1[1]-p2[1]) <= 2*AY_EPSILON)
    {
      if(fabs(p1[1]-p3[1]) <= 2*AY_EPSILON)
	{
	  ip[0] = p3[0];
	  ip[1] = p3[1];
	  return 1;
	}
      if(fabs(p1[1]-p4[1]) <= 2*AY_EPSILON)
	{
	  ip[0] = p4[0];
	  ip[1] = p4[1];
	  return 1;
	}
    }

  den = ((p2[0]-p1[0])*(p4[1]-p3[1])-(p2[1]-p1[1])*(p4[0]-p3[0]));

  r = ((p1[1]-p3[1])*(p4[0]-p3[0])-(p1[0]-p3[0])*(p4[1]-p3[1]))/den;

  if((r < AY_EPSILON) || (r > (1.0-AY_EPSILON)))
    {
      return 0; /* XXXX early exit! */
    }

  s = ((p1[1]-p3[1])*(p2[0]-p1[0])-(p1[0]-p3[0])*(p2[1]-p1[1]))/den;

  if((s < AY_EPSILON) || (s > (1.0-AY_EPSILON)))
    {
      return 0; /* XXXX early exit! */
    }

  ip[0] = p1[0]+r*(p2[0]-p1[0]);
  ip[1] = p1[1]+r*(p2[1]-p1[1]);

 return 1;
} /* ay_stess_IntersectLines2D */


/* ay_stess_TessTrimCurve:
 * Helper for ay_stess_TessTrimCurves() below.
 * Tesselate a single trim curve/loop.
 * qf < 0 => ReTess-mode, re-tesselate linear loops for drawing
 */
void
ay_stess_TessTrimCurve(ay_object *o, double **tts, int *tls, int *tds,
		       int *i, double qf)
{
 ay_nurbcurve_object *nc = NULL;
 double angle, m[16], *dtmp, p1[4], p2[4];
 int apply_trafo = AY_FALSE, stride, j;

  nc = (ay_nurbcurve_object*)o->refine;

  if(nc->order == 2 && qf > 0)
    {
      ay_stess_TessLinearTrimCurve(o, tts, tls, tds, i);
      return;
    }

  if(qf < 0)
    qf = fabs(qf);

  if(nc->is_rat)
    stride = 3;
  else
    stride = 2;

  if(AY_ISTRAFO(o))
    {
      apply_trafo = AY_TRUE;
      ay_trafo_creatematrix(o, m);
    }

  if(apply_trafo)
    {
      if(!(dtmp = malloc(nc->length*stride*sizeof(double))))
	{ return; }

      /* apply transformations */
      for(j = 0; j < nc->length; j++)
	{
	  memcpy(p1, &(nc->controlv[j*4]), 4*sizeof(double));
	  AY_APTRAN3(p2, p1, m)
	    memcpy(&(dtmp[j*stride]), p2, 2*sizeof(double));
	  if(nc->is_rat)
	    dtmp[j*stride+2] = nc->controlv[j*4+3];
	}

      ay_stess_CurvePoints2D(nc->length, nc->order-1, nc->knotv, dtmp,
			     stride, nc->is_rat, qf, &(tls[*i]), &(tts[*i]));
      free(dtmp);
      dtmp = NULL;
    }
  else
    {
      ay_stess_CurvePoints2D(nc->length, nc->order-1,
			     nc->knotv, nc->controlv, 4,
			     nc->is_rat, qf, &(tls[*i]), &(tts[*i]));
    }

  /* get orientation of trimloop */
  if(tds)
    {
      angle = 0.0;
      ay_nct_getorientation(nc, 4, 1, 0, &angle);
      if(angle >= 0.0)
	{
	  tds[*i] = 1;
	}
    }

  /* advance trim loop counter */
  *i = *i+1;

 return;
} /* ay_stess_TessTrimCurve */


/* ay_stess_TessLinearTrimCurve:
 * Helper for ay_stess_TessTrimCurves() below.
 * Tesselate a single linear (i.e. order == 2) trim curve/loop.
 */
void
ay_stess_TessLinearTrimCurve(ay_object *o, double **tts, int *tls, int *tds,
			     int *i)
{
 ay_nurbcurve_object *nc = NULL;
 double angle, m[16], *dtmp, p1[4], p2[4];
 int apply_trafo = AY_FALSE, j;

  nc = (ay_nurbcurve_object*)o->refine;

  if(AY_ISTRAFO(o))
    {
      apply_trafo = AY_TRUE;
      ay_trafo_creatematrix(o, m);
    }

  if(!(dtmp = malloc(nc->length*2*sizeof(double))))
    { return; }

  if(apply_trafo)
    {
      /* apply transformations */
      for(j = 0; j < nc->length; j++)
	{
	  memcpy(p1, &(nc->controlv[j*4]), 4*sizeof(double));
	  AY_APTRAN3(p2, p1, m)
	    memcpy(&(dtmp[j*2]), p2, 2*sizeof(double));
	  if(nc->is_rat)
	    {
	      dtmp[j*2]	  /= nc->controlv[j*4+3];
	      dtmp[j*2+1] /= nc->controlv[j*4+3];
	    }
	}
    }
  else
    {
      for(j = 0; j < nc->length; j++)
	{
	  memcpy(&(dtmp[j*2]), &(nc->controlv[j*4]), 2*sizeof(double));
	  if(nc->is_rat)
	    {
	      dtmp[j*2]	  /= nc->controlv[j*4+3];
	      dtmp[j*2+1] /= nc->controlv[j*4+3];
	    }
	}
    } /* if */

  tls[*i] = nc->length;
  tts[*i] = dtmp;

  /* get orientation of trimloop */
  if(tds)
    {
      angle = 0.0;
      ay_nct_getorientation(nc, 4, 1, 0, &angle);
      if(angle >= 0.0)
	{
	  tds[*i] = 1;
	}
    }

  /* advance trim loop counter */
  *i = *i+1;

 return;
} /* ay_stess_TessLinearTrimCurve */


/* ay_stess_TessTrimCurves:
 *  tesselate all trim curves of object <o>
 */
int
ay_stess_TessTrimCurves(ay_object *o, int qf, int *nt, double ***tt,
			int **tl, int **td)
{
 int ay_status = AY_OK;
 double **tts = NULL;
 int i, numtrims = 0, *tls = NULL, *tds = NULL;
 ay_object *trim = NULL, *loop = NULL, *p, *nc = NULL, *cnc = NULL;

  /* count trimloops */
  trim = o->down;
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  numtrims++;
	  break;
	case AY_IDLEVEL:
	  loop = trim->down;
	  while(loop->next)
	    {
	      if(loop->type == AY_IDNCURVE)
		{
		  numtrims++;
		  break;
		}
	      else
		{
		  if(ay_provide_object(loop, AY_IDNCURVE, NULL) == AY_OK)
		    {
		      numtrims++;
		      break;
		    }
		}
	      loop = loop->next;
	    } /* while */
	  break;
	default:
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      numtrims++;
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */
      trim = trim->next;
    } /* while */

  if(numtrims == 0)
    return AY_ERROR;

  /* tesselate trimloops and get their orientation */
  if(!(tts = calloc(numtrims, sizeof(double *))))
    return AY_EOMEM;
  if(!(tls = calloc(numtrims, sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(td)
    if(!(tds = calloc(numtrims, sizeof(int))))
      { ay_status = AY_EOMEM; goto cleanup; }

  i = 0;
  trim = o->down;
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  ay_stess_TessTrimCurve(trim, tts, tls, tds, &i, qf);
	  break;
	case AY_IDLEVEL:
	  loop = trim->down;
	  if(loop->next)
	    {
	      cnc = NULL;
	      ay_nct_concatobjs(loop, &cnc);
	      if(cnc)
		{
		  ay_stess_TessTrimCurve(cnc, tts, tls, tds, &i, qf);
		  ay_object_delete(cnc);
		}
	    }
	  break;
	default:
	  /* trim is curve providing object */
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      ay_stess_TessTrimCurve(nc, tts, tls, tds, &i, qf);
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */

      trim = trim->next;
    } /* while */

  /* return results */
  *nt = numtrims;
  *tt = tts;
  *tl = tls;
  if(td)
    *td = tds;

  /* prevent cleanup code from doing something harmful */
  tts = NULL;
  tls = NULL;
  tds = NULL;

cleanup:

  if(tts)
    free(tts);
  if(tls)
    free(tls);
  if(tds)
    free(tds);

 return ay_status;
} /* ay_stess_TessTrimCurves */


/* ay_stess_ReTessTrimCurves:
 *  re tesselate all linear trims of object <o>
 */
int
ay_stess_ReTessTrimCurves(ay_object *o, int qf, int numtrims, double **tt,
			  int *tl, double **tp)
{
 int ay_status = AY_OK;
 double *tps = NULL, p4[4];
 int i, j, a, npnts = 0;
 ay_object *trim = NULL, *loop = NULL, *p, *nc = NULL, *cnc = NULL;
 ay_nurbcurve_object *c = NULL;
 ay_nurbpatch_object *np = NULL;

  np = (ay_nurbpatch_object*)o->refine;

  /* XXXX todo: adapt qf to NPatch dimensions to accomodate for the
     worst case: a single linear trim segment running along the complete
     patch surface; even in this case, we should provide an adequate
     amount of tesselated points and avoid leaving the surface too
     much/often */
  qf *= 2;

  i = 0;
  trim = o->down;
  while(trim->next)
    {
      c = NULL;
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  c = (ay_nurbcurve_object*)trim->refine;
	  if(c->order == 2)
	    ay_stess_TessTrimCurve(trim, tt, tl, NULL, &i, -qf);
	  else
	    i++;
	  break;
	case AY_IDLEVEL:
	  loop = trim->down;
	  if(loop->next)
	    {
	      cnc = NULL;
	      ay_nct_concatobjs(loop, &cnc);
	      if(cnc)
		{
		  c = (ay_nurbcurve_object*)cnc->refine;
		  if(c->order == 2)
		    ay_stess_TessTrimCurve(cnc, tt, tl, NULL, &i, -qf);
		  else
		    i++;
		  ay_object_delete(cnc);
		}
	    }
	  break;
	default:
	  /* trim is curve providing object */
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      c = (ay_nurbcurve_object*)nc->refine;
	      if(c->order == 2)
		ay_stess_TessTrimCurve(nc, tt, tl, NULL, &i, -qf);
	      else
		i++;
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */

      trim = trim->next;
    } /* while */

  if(tp)
    {
      /* calculate 3D surface points */
      for(i = 0; i < numtrims; i++)
	{
	  npnts += tl[i];
	}

      if(!(tps = calloc(npnts*3, sizeof(double))))
	{ return AY_EOMEM; }

      a = 0;
      for(i = 0; i < numtrims; i++)
	{
	  for(j = 0; j < tl[i]; j++)
	    {
	      if(np->is_rat)
		{
		  ay_nb_SurfacePoint4D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, tt[i][j*2], tt[i][j*2+1],
				       p4);
		  memcpy(&(tps[a]), p4, 3*sizeof(double));
		}
	      else
		{
		  ay_nb_SurfacePoint3D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, tt[i][j*2], tt[i][j*2+1],
				       &(tps[a]));
		}
	      a += 3;
	    } /* for */
	} /* for */

      /* return result */
      *tp = tps;
    } /* if tp */

 return ay_status;
} /* ay_stess_ReTessTrimCurves */


/* ay_stess_SortIntersections:
 *  sort a list of intersections for faster merging
 */
void
ay_stess_SortIntersections(ay_stess_uvp *list, int u)
{
 int done = AY_FALSE;
 ay_stess_uvp *p1, *p2;
 double t;

  if(!list || !list->next)
    return;

  while(!done)
    {
      done = AY_TRUE;

      p1 = list;
      p2 = p1->next;
      while(p1 && p2)
	{
	  if(u)
	    {
	      if(p1->u > p2->u)
		{
		  t = p1->u;
		  p1->u = p2->u;
		  p2->u = t;
		  done = AY_FALSE;
		}
	    }
	  else
	    {
	      if(p1->v > p2->v)
		{
		  t = p1->v;
		  p1->v = p2->v;
		  p2->v = t;
		  done = AY_FALSE;
		}
	    }

	  p1 = p1->next;
	  p2 = p2->next;
	} /* while p1 && p2 */
    } /* while !done */

 return;
} /* ay_stess_SortIntersections */


/* ay_stess_TessTrimmedNPU:
 *  tesselate NURBS patch <o> into lines in parametric direction u
 */
int
ay_stess_TessTrimmedNPU(ay_object *o, int qf, int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			int *reslen, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **nextuvp, *trimuvp;
 ay_stess_uvp *nexttrimuvp, *olduvp;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double *fd1, *fd2, temp[3] = {0}, *ders = NULL;
 double umin, umax, vmin, vmax, ud, vd;
 int i, k, l, ind;
 int out = 0;
 int Cm, Cn;

  p = (ay_nurbpatch_object *)o->refine;

  Cn = (p->width + 4) * qf;
  *reslen = Cn;
  Cm = (p->height + 4) * qf;
  if(!(uvps = calloc(Cn, sizeof(ay_stess_uvp *))))
    {
      return AY_EOMEM;
    }

  U = p->uknotv;
  umin = U[p->uorder-1];
  umax = U[p->width];
  ud = (umax-umin)/((Cn)-1);

  V = p->vknotv;
  vmin = V[p->vorder-1];
  vmax = V[p->height];
  vd = (vmax-vmin)/((Cm)-1);

  u = umin;
  p3[1] = vmin - AY_EPSILON;
  p4[1] = vmax + AY_EPSILON;

  for(i = 0; i < Cn; i++)
    {
      nextuvp = &trimuvp;
      olduvp = NULL;
      trimuvp = NULL;

      if(i == Cn-1)
	u = umax;
      p3[0] = u;
      p4[0] = u;

      /* calc all intersections of all trimloops with current u */
      for(k = 0; k < numtrims; k++)
	{
	  tt = tcs[k];

	  for(l = 0; l < (tcslens[k]-1); l++)
	    {
	      ind = l*2;

	      /* is section crossing or touching u? */
	      if(((tt[ind] <= (u + AY_EPSILON))
		  && (tt[ind+2] >= (u - AY_EPSILON))) ||
		 ((tt[ind] >= (u - AY_EPSILON)) &&
		  (tt[ind+2] <= (u + AY_EPSILON))))
		{
		  /* weed out all sections that run (more or less)
		     exactly along the current u-line, nothing good
		     comes of them */
		  if((fabs(tt[ind] - u) < AY_EPSILON) &&
		     (fabs(tt[ind+2] - u) < AY_EPSILON))
		    {
#ifdef AY_STESSDBG
		      printf("Discarding parallel section.\n");
#endif
		      continue;
		    }
		  ipoint[0] = 0.0;
		  ipoint[1] = 0.0;

		  if((ay_stess_IntersectLines2D(&(tt[ind]),
						&(tt[ind+2]),
						p3, p4, ipoint)))
		    {
		      /* u-line intersects with trimcurve */
		      /* => add new point (but avoid consecutive
			 equal points; those appear if a loop touches
			 start or end of the current u-line) */
		      if(!olduvp ||
			 fabs(olduvp->v - ipoint[1]) > AY_EPSILON)
			{
			  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			    {
			      ay_status = AY_EOMEM;
			      goto cleanup;
			    }
			  newuvp->type = 1;
			  newuvp->dir = tcsdirs[k];
			  newuvp->u = ipoint[0];
			  newuvp->v = ipoint[1];
			  olduvp = newuvp;
			  *nextuvp = newuvp;
			  nextuvp = &(newuvp->next);
			}
		    } /* if have intersection */
		} /* if is not parallel */
	    } /* for */
	} /* for */

      nextuvp = &(uvps[i]);

      if(trimuvp && trimuvp->next)
	{
	  /* we had trimloop points */
	  ay_stess_SortIntersections(trimuvp, AY_FALSE);

	  *nextuvp = trimuvp;
	  nextuvp = &(trimuvp->next);
	  nexttrimuvp = trimuvp->next;

	  v = vmin;
	  while(v < trimuvp->v)
	    v += vd;

	  out = 0;

	  while(nexttrimuvp)
	    {
	      while(v < (nexttrimuvp->v-4*AY_EPSILON))
		{
		  if(!out)
		    {
		      if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			{
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      newuvp->u = u;
		      newuvp->v = v;
		      *nextuvp = newuvp;
		      nextuvp = &(newuvp->next);
		    }
		  v += vd;
		} /* while */

	      out = !out;

	      *nextuvp = nexttrimuvp;
	      nextuvp = &(nexttrimuvp->next);

	      nexttrimuvp = nexttrimuvp->next;
	    } /* while */
	}
      else
	{
	  if(trimuvp)
	    free(trimuvp);
	} /* if have multiple intersections */

      u += ud;
    } /* for */

  /* finally, calculate surfacepoints */
  if(p->is_rat)
    {
      if(!(ders = malloc(
	 ay_nb_FirstDerSurf4DMSize(p->uorder-1, p->vorder-1)*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
    }
  else
    {
      if(!(ders = malloc(
	 ay_nb_FirstDerSurf3DMSize(p->uorder-1, p->vorder-1)*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
    }

  fd1 = &(ders[3]);
  fd2 = &(ders[6]);

  for(i = 0; i < Cn; i++)
    {
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  memset(ders, 0, 4*sizeof(double));

	  if(p->is_rat)
	    ay_nb_FirstDerSurf4DM(p->width-1, p->height-1,
			       p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
				  p->controlv, uvpptr->u, uvpptr->v, ders);
	  else
	    ay_nb_FirstDerSurf3DM(p->width-1, p->height-1,
			       p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
				  p->controlv, uvpptr->u, uvpptr->v, ders);

	  memcpy(uvpptr->C, ders, 3*sizeof(double));

	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(uvpptr->C[3]), temp, 3*sizeof(double));
	  if(uvpptr->next)
	    {
	      uvpptr->next->prev = uvpptr;
	    }
	  uvpptr = uvpptr->next;
	} /* while */
    } /* for */

  free(ders);

  /* return result */
  *result = uvps;

  /* prevent cleanup code from doing something harmful */
  uvps = NULL;

cleanup:

  if(uvps)
    {
      for(i = 0; i < Cn; i++)
	{
	  while(uvps[i])
	    {
	      uvpptr = uvps[i]->next;
	      free(uvps[i]);
	      uvps[i] = uvpptr;
	    }
	}
      free(uvps);
    }

 return ay_status;
} /* ay_stess_TessTrimmedNPU */


/* ay_stess_TessTrimmedNPV:
 *  tesselate NURBS patch <o> into lines in parametric direction v
 */
int
ay_stess_TessTrimmedNPV(ay_object *o, int qf, int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			int *reslen, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **nextuvp, *trimuvp;
 ay_stess_uvp *olduvp, *nexttrimuvp;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double *fd1, *fd2, temp[3] = {0}, *ders = NULL;
 double umin, umax, vmin, vmax, ud, vd;
 int i, k, l, ind;
 int out = 0;
 int Cm, Cn;

  p = (ay_nurbpatch_object *)o->refine;

  Cn = (p->width + 4) * qf;
  Cm = (p->height + 4) * qf;
  *reslen = Cm;
  if(!(uvps = calloc(Cm, sizeof(ay_stess_uvp *))))
    {
      return AY_EOMEM;
    }

  U = p->uknotv;
  umin = U[p->uorder-1];
  umax = U[p->width];
  ud = (umax-umin)/((Cn)-1);

  V = p->vknotv;
  vmin = V[p->vorder-1];
  vmax = V[p->height];
  vd = (vmax-vmin)/((Cm)-1);

  v = vmin;
  p3[0] = umin - AY_EPSILON;
  p4[0] = umax + AY_EPSILON;

  for(i = 0; i < Cm; i++)
    {
      nextuvp = &trimuvp;
      olduvp = NULL;
      trimuvp = NULL;

      if(i == Cm-1)
	v = vmax;
      p3[1] = v;
      p4[1] = v;

      /* calc all intersections of all trimloops with current v */
      for(k = 0; k < numtrims; k++)
	{
	  tt = tcs[k];

	  for(l = 0; l < (tcslens[k]-1); l++)
	    {
	      ind = l*2;

	      /* is section crossing or touching v? */
	      if(((tt[ind+1] <= (v + AY_EPSILON)) &&
		  (tt[ind+2+1] >= (v - AY_EPSILON))) ||
		 ((tt[ind+1] >= (v - AY_EPSILON)) &&
		  (tt[ind+2+1] <= (v + AY_EPSILON))))
		{
		  /* weed out all sections that run (more or less)
		     exactly along the current v-line, nothing good
		     comes of them */
		  if((fabs(tt[ind+1] - v) < AY_EPSILON) &&
		     (fabs(tt[ind+2+1] - v) < AY_EPSILON))
		    continue;

		  ipoint[0] = 0.0;
		  ipoint[1] = 0.0;

		  if((ay_stess_IntersectLines2D(&(tt[ind]),
						&(tt[ind+2]),
						p3, p4, ipoint)))
		    {
		      /* v-line intersects with trimcurve */
		      /* => add new point (but avoid consecutive
			 equal points; those appear if a loop touches
			 start or end of the current v-line) */
		      if(!olduvp ||
			 fabs(olduvp->u - ipoint[0]) > AY_EPSILON)
			{
			  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			    {
			      ay_status = AY_EOMEM;
			      goto cleanup;
			    }
			  newuvp->type = 1;
			  newuvp->dir = tcsdirs[k];
			  newuvp->u = ipoint[0];
			  newuvp->v = ipoint[1];

			  olduvp = newuvp;
			  *nextuvp = newuvp;
			  nextuvp = &(newuvp->next);
			}
		    } /* if have intersection */
		} /* if is not parallel */
	    } /* for */
	} /* for */

      u = umin;
      nextuvp = &(uvps[i]);

      if(trimuvp && trimuvp->next)
	{
	  /* we had trimloop points */
	  ay_stess_SortIntersections(trimuvp, AY_TRUE);

	  *nextuvp = trimuvp;
	  nextuvp = &(trimuvp->next);
	  nexttrimuvp = trimuvp->next;

	  u = umin;
	  while(u < trimuvp->u)
	    u += ud;

	  out = 0;

	  while(nexttrimuvp)
	    {
	      while(u < (nexttrimuvp->u-4*AY_EPSILON))
		{
		  if(!out)
		    {
		      if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			{
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      newuvp->u = u;
		      newuvp->v = v;
		      *nextuvp = newuvp;
		      nextuvp = &(newuvp->next);
		    }
		  u += ud;
		} /* while */

	      out = !out;

	      *nextuvp = nexttrimuvp;
	      nextuvp = &(nexttrimuvp->next);

	      nexttrimuvp = nexttrimuvp->next;
	    } /* while */
	}
      else
	{
	  if(trimuvp)
	    free(trimuvp);
	} /* if have multiple intersections */

      v += vd;
    } /* for */

  /* finally, calculate surfacepoints */
  if(p->is_rat)
    {
      if(!(ders = malloc(
	 ay_nb_FirstDerSurf4DMSize(p->uorder-1, p->vorder-1)*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
    }
  else
    {
      if(!(ders = malloc(
	 ay_nb_FirstDerSurf3DMSize(p->uorder-1, p->vorder-1)*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
    }

  fd1 = &(ders[3]);
  fd2 = &(ders[6]);

  for(i = 0; i < Cm; i++)
    {
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  memset(ders, 0, 4*sizeof(double));

	  if(p->is_rat)
	    ay_nb_FirstDerSurf4DM(p->width-1, p->height-1,
			       p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
				  p->controlv, uvpptr->u, uvpptr->v, ders);
	  else
	    ay_nb_FirstDerSurf3DM(p->width-1, p->height-1,
			       p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
				  p->controlv, uvpptr->u, uvpptr->v, ders);

	  memcpy(uvpptr->C, ders, 3*sizeof(double));

	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(uvpptr->C[3]), temp, 3*sizeof(double));
	  if(uvpptr->next)
	    {
	      uvpptr->next->prev = uvpptr;
	    }
	  uvpptr = uvpptr->next;
	} /* while */
    } /* for */

  free(ders);

  /* return result */
  *result = uvps;

  /* prevent cleanup code from doing something harmful */
  uvps = NULL;

cleanup:

  if(uvps)
    {
      for(i = 0; i < Cm; i++)
	{
	  while(uvps[i])
	    {
	      uvpptr = uvps[i]->next;
	      free(uvps[i]);
	      uvps[i] = uvpptr;
	    }
	}
      free(uvps);
    }

 return ay_status;
} /* ay_stess_TessTrimmedNPV */


/* ay_stess_DrawTrimmedSurface:
 *
 */
void
ay_stess_DrawTrimmedSurface(ay_stess_patch *stess)
{
 int i, j, a, out;
 ay_stess_uvp *uvpptr;

  if(!stess)
    return;

  /* draw iso-u lines */
  for(i = 0; i < stess->upslen; i++)
    {
      uvpptr = stess->ups[i];
      if(uvpptr && uvpptr->next)
	{
	  out = 1;

	  while(uvpptr)
	    {
	      if(uvpptr->type > 0)
		{
		  if(out)
		    {
		      glBegin(GL_LINE_STRIP);
		      glVertex3dv((GLdouble*)(uvpptr->C));
		      out = 0;
		    }
		  else
		    {
		      glVertex3dv((GLdouble*)(uvpptr->C));
		      glEnd();
		      out = 1;
		    } /* if out */
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		} /* if trim point */

	      uvpptr = uvpptr->next;
	    } /* while */

	  if(!out)
	    {
	      glEnd();
	    }
	} /* if have two points */
    } /* for all u lines */

  /* draw iso-v lines */
  for(i = 0; i < stess->vpslen; i++)
    {
      uvpptr = stess->vps[i];

      if(uvpptr && uvpptr->next)
	{
	  out = 1;

	  while(uvpptr)
	    {
	      if(uvpptr->type > 0)
		{
		  if(out)
		    {
		      glBegin(GL_LINE_STRIP);
		      glVertex3dv((GLdouble*)(uvpptr->C));
		      out = 0;
		    }
		  else
		    {
		      glVertex3dv((GLdouble*)(uvpptr->C));
		      glEnd();
		      out = 1;
		    } /* if out */
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		} /* if trim point */

	      uvpptr = uvpptr->next;
	    }  /* while */

	  if(!out)
	    {
	      glEnd();
	    }
	} /* if have two points */
    } /* for all v lines */

  /* draw trimcurves (outlines) */
  a = 0;
  for(i = 0; i < stess->tcslen; i++)
    {
      glBegin(GL_LINE_STRIP);
       for(j = 0; j < stess->tcslens[i]; j++)
	 {
	   glVertex3dv(&(stess->tcspnts[a]));
	   a += 3;
	 } /* for */
      glEnd();
    } /* for */

 return;
} /* ay_stess_DrawTrimmedSurface */


/* ay_stess_ShadeTrimmedSurface:
 *
 */
void
ay_stess_ShadeTrimmedSurface(ay_stess_patch *stess)
{
 int i, instrip = AY_FALSE;
 unsigned int j;
 ay_stess_uvp *u1, *u2, *v1, *v2;
 ay_pomesh_object *po;
 double *p;

  if(!stess)
    return;

  if(stess->pomesh)
    {
      /* assume the pomesh is a triangle soup (from the tesselation) */
      po = stess->pomesh;
      glBegin(GL_TRIANGLES);
       p = stess->normal;
       glNormal3dv((GLdouble*)p);
       p = po->controlv;
       for(j = 0; j < po->npolys; j++)
	 {
	   glVertex3dv((GLdouble*)p);
	   p += 3;
	   glVertex3dv((GLdouble*)p);
	   p += 3;
	   glVertex3dv((GLdouble*)p);
	   p += 3;
	 }
      glEnd();
      return;
    }

  /* search for complete cells (all types 0) in u-direction */
  for(i = 0; i < (stess->upslen-1); i++)
    {
      u1 = stess->ups[i];
      u2 = stess->ups[i+1];

      if(stess->ft_cw)
	{
	  /* forward to next complete grid-cell */
	  while(u1)
	    {
	      if(u1->type == 0 || u1->type == 2)
		break;
	      u1 = u1->next;
	    }
	  while(u2)
	    {
	      if(u2->type == 0 || u2->type == 2)
		break;
	      u2 = u2->next;
	    }
	}
      else
	{
	  /* forward to first trim */
	  while(u1 && u1->type == 0)
	    u1 = u1->next;

	  while(u2 && u2->type == 0)
	    u2 = u2->next;
	}

      if(!u1 || !u2 || !u1->next || !u2->next)
	{
	  continue;
	}

      while(u1 && u1->type != 0)
	u1 = u1->next;

      while(u2 && u2->type != 0)
	u2 = u2->next;

      while(u1 && u1->next && u2 && u2->next)
	{
	  if(u1->type == 0 && u1->next->type == 0 &&
	     u2->type == 0 && u2->next->type == 0 &&
	     u1->v == u2->v)
	    {
	      if(!instrip)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		  instrip = AY_TRUE;
		}
	      glNormal3dv((GLdouble*)&((u1->C)[3]));
	      glVertex3dv((GLdouble*)(u1->C));
	      glNormal3dv((GLdouble*)&((u2->C)[3]));
	      glVertex3dv((GLdouble*)(u2->C));
	      u1 = u1->next;
	      u2 = u2->next;
	      glNormal3dv((GLdouble*)&((u1->C)[3]));
	      glVertex3dv((GLdouble*)(u1->C));
	      glNormal3dv((GLdouble*)&((u2->C)[3]));
	      glVertex3dv((GLdouble*)(u2->C));

	      /* check next cell */
	      if(!u1->next || !u2->next ||
		 u1->next->type != 0 || u2->next->type != 0)
		{
		  if(instrip)
		    {
		      glEnd();
		      instrip = AY_FALSE;
		    }
		}
	    } /* have complete cell */

	  /* forward to next candidate cell */
	  while((u1 && u1->type != 0) ||
		(u1 && u1->next && u1->next->type != 0))
	    u1 = u1->next;

	  while((u2 && u2->type != 0) ||
		(u2 && u2->next && u2->next->type != 0))
	    u2 = u2->next;

	  if(!u1 || !u2)
	    break;

	  if(u1->v != u2->v)
	    {
	      if(u1->v < u2->v)
		{
		  while(u1 && u1->v < u2->v)
		    u1 = u1->next;
		}
	      else
		{
		  while(u2 && u1->v > u2->v)
		    u2 = u2->next;
		}
	    }
	} /* while */

      if(instrip)
	{
	  glEnd();
	  instrip = AY_FALSE;
	}

      /*
       * search for incomplete cells
       */
      u1 = stess->ups[i];
      u2 = stess->ups[i+1];

      if(stess->ft_cw)
	{
	  while(u1)
	    {
	      if(u1->type == 0 || u1->type == 2)
		break;
	      u1 = u1->next;
	    }
	  while(u2)
	    {
	      if(u2->type == 0 || u2->type == 2)
		break;
	      u2 = u2->next;
	    }
	}
      else
	{
	  /* forward to first trim */
	  while(u1 && u1->type == 0)
	    u1 = u1->next;

	  while(u2 && u2->type == 0)
	    u2 = u2->next;
	}

      if(!u1 || !u2 || !u1->next || !u2->next)
	continue;

      while(u1 && u1->type != 0)
	u1 = u1->next;

      while(u2 && u2->type != 0)
	u2 = u2->next;

      if(!u1 || !u2 || !u1->next || !u2->next)
	continue;

      while(u1 && u1->next && u2 && u2->next)
	{
	  if(u1->v != u2->v)
	    {
	      if(u1->v < u2->v)
		{
		  while(u1 && u1->v < u2->v)
		    u1 = u1->next;
		}
	      else
		{
		  while(u2 && u1->v > u2->v)
		    u2 = u2->next;
		}
	    }

	  if(!u1 || !u2 || !u1->next || !u2->next)
	    break;

	  if(u1->type == 0 && u2->type == 0 && u1->v == u2->v)
	    {
	      /* check previous cell */
	      if(u1->prev && u2->prev &&
		 (u1->prev->type || u2->prev->type))
		{
		  glBegin(GL_TRIANGLE_STRIP);
		   glNormal3dv((GLdouble*)&((u1->prev->C)[3]));
		   glVertex3dv((GLdouble*)(u1->prev->C));
		   glNormal3dv((GLdouble*)&((u2->prev->C)[3]));
		   glVertex3dv((GLdouble*)(u2->prev->C));
		   glNormal3dv((GLdouble*)&((u1->C)[3]));
		   glVertex3dv((GLdouble*)(u1->C));
		   glNormal3dv((GLdouble*)&((u2->C)[3]));
		   glVertex3dv((GLdouble*)(u2->C));
		  glEnd();
		}

	      /* check next cell */
	      if(u1->next->type || u2->next->type)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		   glNormal3dv((GLdouble*)&((u1->C)[3]));
		   glVertex3dv((GLdouble*)(u1->C));
		   glNormal3dv((GLdouble*)&((u2->C)[3]));
		   glVertex3dv((GLdouble*)(u2->C));
		   glNormal3dv((GLdouble*)&((u1->next->C)[3]));
		   glVertex3dv((GLdouble*)(u1->next->C));
		   glNormal3dv((GLdouble*)&((u2->next->C)[3]));
		   glVertex3dv((GLdouble*)(u2->next->C));
		  glEnd();
		}
	    } /* if */

	  /* forward to next candidate cell */
	  do
	    u1 = u1->next;
	  while(u1 && u1->type != 0);

	  do
	    u2 = u2->next;
	  while(u2 && u2->type != 0);

	} /* while */
    } /* for */

  /****************************************************/

  /* search for complete cells (all types 0) in v-direction, but
     do not render them, just process the possibly incomplete
     cells right before/behind them */
  for(i = 0; i < (stess->vpslen-1); i++)
    {
      v1 = stess->vps[i];
      v2 = stess->vps[i+1];

      if(stess->ft_cw)
	{
	  /* forward to next complete grid-cell */
	  while(v1)
	    {
	      if(v1->type == 0 || v1->type == 2)
		break;
	      v1 = v1->next;
	    }
	  while(v2)
	    {
	      if(v2->type == 0 || v2->type == 2)
		break;
	      v2 = v2->next;
	    }
	}
      else
	{
	  /* forward to first trim */
	  while(v1 && v1->type == 0)
	    v1 = v1->next;

	  while(v2 && v2->type == 0)
	    v2 = v2->next;
	}

      if(!v1 || !v2 || !v1->next || !v2->next)
	continue;

      while(v1 && v1->type != 0)
	v1 = v1->next;

      while(v2 && v2->type != 0)
	v2 = v2->next;

      while(v1 && v1->next && v2 && v2->next)
	{
	  if(v1->type == 0 && v1->next->type == 0 &&
	     v2->type == 0 && v2->next->type == 0 &&
	     v1->u == v2->u)
	    {
	      if(!instrip)
		{
		  /* here we would start the strip of complete cells,
		     but in v direction, we only check, whether there
		     is an incomplete cell before, and shade it */
		  if(v1->prev && v2->prev &&
		     (v1->prev->type || v2->prev->type))
		    {
		      glBegin(GL_TRIANGLE_STRIP);
		       glNormal3dv((GLdouble*)&((v1->prev->C)[3]));
		       glVertex3dv((GLdouble*)(v1->prev->C));
		       glNormal3dv((GLdouble*)&((v1->C)[3]));
		       glVertex3dv((GLdouble*)(v1->C));
		       glNormal3dv((GLdouble*)&((v2->prev->C)[3]));
		       glVertex3dv((GLdouble*)(v2->prev->C));
		       glNormal3dv((GLdouble*)&((v2->C)[3]));
		       glVertex3dv((GLdouble*)(v2->C));
		      glEnd();
		    }
		  instrip = AY_TRUE;
		} /* if */

	      v1 = v1->next;
	      v2 = v2->next;

	      /* check next cell */
	      if(!v1->next || !v2->next ||
		 v1->next->type != 0 || v2->next->type != 0)
		{
		  if(instrip)
		    {

		      instrip = AY_FALSE;

		      /* here we would end the strip of complete cells,
			 but in v direction, we only check, whether there
			 is an incomplete cell following, and shade it */
		      if(v1->next && v2->next &&
			 (v1->next->type || v2->next->type))
			{
			  glBegin(GL_TRIANGLE_STRIP);
			   glNormal3dv((GLdouble*)&((v1->C)[3]));
			   glVertex3dv((GLdouble*)(v1->C));
			   glNormal3dv((GLdouble*)&((v1->next->C)[3]));
			   glVertex3dv((GLdouble*)(v1->next->C));
			   glNormal3dv((GLdouble*)&((v2->C)[3]));
			   glVertex3dv((GLdouble*)(v2->C));
			   glNormal3dv((GLdouble*)&((v2->next->C)[3]));
			   glVertex3dv((GLdouble*)(v2->next->C));
			  glEnd();
			}
		    } /* if instrip */
		} /* if */
	    } /* if have complete cell */

	  /* forward to next candidate cell */
	  while((v1 && v1->type != 0) ||
		(v1 && v1->next && v1->next->type != 0))
	    v1 = v1->next;

	  while((v2 && v2->type != 0) ||
		(v2 && v2->next && v2->next->type != 0))
	    v2 = v2->next;

	  if(!v1 || !v2)
	    break;

	  if(v1->u != v2->u)
	    {
	      if(v1->u < v2->u)
		{
		  while(v1 && v1->u < v2->u)
		    v1 = v1->next;
		}
	      else
		{
		  while(v2 && v1->u > v2->u)
		    v2 = v2->next;
		}
	    }
	} /* while */

      if(instrip)
	{
	  instrip = AY_FALSE;
	}
    } /* for */

 return;
} /* ay_stess_ShadeTrimmedSurface */


/** ay_stess_AddBoundaryTrim:
 * Add a extra boundary trim curve if there is no enclosing trim
 * and the trim curve direction of the first given trim curve leads
 * to a hole in the surface.
 * This way TessTrimmedNPU()/TessTrimmedNPV() can rely on the fact
 * that the first trim always designates the start of the surface.
 *
 * \param[in,out] o NURBS patch to process
 *
 * \return AY_TRUE if the boundary trim was added
 */
int
ay_stess_AddBoundaryTrim(ay_object *o)
{
 ay_nurbpatch_object *np = NULL;
 ay_nurbcurve_object *nc = NULL;
 double umin, umax, vmin, vmax, *cv, a;
 int is_bound;

  np = (ay_nurbpatch_object *)o->refine;

  umin = np->uknotv[np->uorder-1];
  umax = np->uknotv[np->width];
  vmin = np->vknotv[np->vorder-1];
  vmax = np->vknotv[np->height];

  ay_npt_isboundcurve(o->down, umin, umax, vmin, vmax, &is_bound);

  if(!is_bound)
    {
      a = 0.0;
      nc = (ay_nurbcurve_object *)o->down->refine;
      ay_nct_getorientation(nc, 4, 0, 0, &a);
      if(a < 0.0)
	{
	  ay_stess_boundarytrim->next = o->down;
	  o->down = ay_stess_boundarytrim;
	  nc = (ay_nurbcurve_object *)o->down->refine;
	  cv = nc->controlv;
	  cv[0] = umin;
	  cv[1] = vmin;

	  cv[4] = umax;
	  cv[5] = vmin;

	  cv[8] = umax;
	  cv[9] = vmax;

	  cv[12] = umin;
	  cv[13] = vmax;

	  cv[16] = umin;
	  cv[17] = vmin;
	  return AY_TRUE;
	}
    }

 return AY_FALSE;
} /* ay_stess_AddBoundaryTrim */


/* ay_stess_TessTrimmedNP:
 *
 */
int
ay_stess_TessTrimmedNP(ay_object *o, int qf, ay_stess_patch *stess)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np = NULL;
 double **tcs = NULL; /**< tesselated trim curves [tcslen][tcslens[i]] */
 int b, i, *tcsdirs = NULL; /**< directions of trim curves [tcslen] */

  np = (ay_nurbpatch_object *)o->refine;

  ay_stess_destroy(stess);

  b = ay_stess_AddBoundaryTrim(o);

  ay_status = ay_stess_TessTrimCurves(o, qf,
				      &(stess->tcslen), &tcs,
				      &(stess->tcslens), &tcsdirs);

  if(ay_status)
    goto cleanup;

  ay_status = ay_stess_TessTrimmedNPU(o, qf, stess->tcslen, tcs,
				      stess->tcslens, tcsdirs,
				      &(stess->upslen), &(stess->ups));

  if(ay_status)
    goto cleanup;

  ay_status = ay_stess_TessTrimmedNPV(o, qf, stess->tcslen, tcs,
				      stess->tcslens, tcsdirs,
				      &(stess->vpslen), &(stess->vps));

  if(ay_status)
    goto cleanup;

  stess->ft_cw = !tcsdirs[0];

  ay_status = ay_stess_ReTessTrimCurves(o, qf,
					stess->tcslen, tcs,
					stess->tcslens,
					&(stess->tcspnts));

  if(ay_status)
    goto cleanup;

  /* prevent cleanup code from doing something harmful */
  np = NULL;

  /* clean up the mess */
cleanup:

  if(tcs)
    {
      for(i = 0; i < stess->tcslen; i++)
	{
	  if(tcs[i])
	    free(tcs[i]);
	}
      free(tcs);
    }

  if(tcsdirs)
    free(tcsdirs);

  if(np)
    {
      ay_stess_destroy(stess);
    }

  if(b)
    {
      o->down = o->down->next;
    }

  if(ay_status)
    ay_status = AY_ERROR;

 return ay_status;
} /* ay_stess_TessTrimmedNP */


/* ay_stess_TessTrimmedPlanarNP:
 *
 */
int
ay_stess_TessTrimmedPlanarNP(ay_object *o, int qf, ay_stess_patch *stess)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np = NULL;
 ay_tag *tag;
 double **tcs = NULL; /**< tesselated trim curves [tcslen][tcslens[i]] */
 int tcslen, *tcslens = NULL, *tcsdirs = NULL;
 unsigned int i, j, a, totalverts = 0;
 ay_pomesh_object *po = NULL, *tpo = NULL;
 double u, v, *p;

  np = (ay_nurbpatch_object *)o->refine;

  ay_stess_destroy(stess);

  ay_status = ay_stess_TessTrimCurves(o, qf,
				      &tcslen, &tcs,
				      &tcslens, &tcsdirs);

  if(ay_status)
    goto cleanup;

  if(tcs)
    {
      stess->tcslen = tcslen;
      stess->tcslens = tcslens;

      if(!(po = calloc(1, sizeof(ay_pomesh_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      po->npolys = 1;
      if(!(po->nloops = calloc(1, sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      po->nloops[0] = tcslen;
      if(!(po->nverts = calloc(tcslen, sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      for(i = 0; i < (unsigned int)tcslen; i++)
	{
	  po->nverts[i] = tcslens[i];
	  totalverts += tcslens[i];
	}

      if(!(po->verts = calloc(totalverts, sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      for(i = 0; i < totalverts; i++)
	{
	  po->verts[i] = i;
	}

      /* fill controlv */
      if(!(po->controlv = calloc(totalverts*3, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      stess->tcspnts = po->controlv;

      p = po->controlv;
      for(i = 0; i < (unsigned int)tcslen; i++)
	{
	  a = 0;
	  for(j = 0; j < (unsigned int)tcslens[i]; j++)
	    {
	      u = tcs[i][a];
	      v = tcs[i][a+1];

	      if(np->is_rat)
		{
		  ay_nb_SurfacePoint4D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, u, v, p);
		}
	      else
		{
		  ay_nb_SurfacePoint3D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, u, v, p);
		}

	      a += 2;
	      p += 3;
	    }
	}

      /* set normal */
      stess->normal[0] = 0.0;
      stess->normal[1] = 0.0;
      stess->normal[2] = 1.0;
      p = NULL;
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_nt_tagtype)
	    {
	      memcpy(stess->normal, ((ay_btval*)tag->val)->payload,
		     3*sizeof(double));
	      p = stess->normal;
	      break;
	    }
	  tag = tag->next;
	}

      /* tesselate the polygon */
      ay_status = ay_tess_pomesh(po, /*optimize=*/AY_FALSE, p, &tpo);

      stess->pomesh = tpo;
    } /* if tcs */

  /* prevent cleanup code from doing something harmful */
  np = NULL;

  /* clean up the mess */
cleanup:

  if(tcs)
    {
      for(i = 0; i < (unsigned int)tcslen; i++)
	{
	  if(tcs[i])
	    free(tcs[i]);
	}
      free(tcs);
    }

  if(tcsdirs)
    free(tcsdirs);

  if(np)
    {
      ay_stess_destroy(stess);
    }

  /* free temporary polymesh */
  if(po)
    {
      if(po->verts)
	free(po->verts);
      if(po->nloops)
	free(po->nloops);
      if(po->nverts)
	free(po->nverts);
      /* deliberately not freeing po->controlv as that is re-used from the
	 polygonal trim loops */
      free(po);
    }

  if(ay_status)
    ay_status = AY_ERROR;

 return ay_status;
} /* ay_stess_TessTrimmedPlanarNP */


/* ay_stess_TessNP:
 *  tesselate NURBS patch object <o> with quality factor <qf>;
 *  stores results in <stess>
 */
int
ay_stess_TessNP(ay_object *o, int qf, ay_stess_patch *stess)
{
 int ay_status = AY_ERROR;
 char fname[] = "stess_TessNP";
 ay_nurbpatch_object *npatch;
 ay_object *p;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(!ay_stess_boundarytrim)
    {
      p = calloc(1, sizeof(ay_object));
      ay_status = ay_nct_create(2, 5, AY_KTNURB, NULL, NULL,
				(ay_nurbcurve_object **)&(p->refine));
      if(ay_status)
	{ free(p); return ay_status; }
      ay_object_defaults(p);
      p->type = AY_IDNCURVE;
      ay_stess_boundarytrim = p;
      ay_status = AY_ERROR;
    }

  if(ay_npt_istrimmed(o, 0))
    {
      /* this is a nontrivially trimmed NURBS patch */
      if(npatch->is_planar)
	ay_status = ay_stess_TessTrimmedPlanarNP(o, qf, stess);
      else
	ay_status = ay_stess_TessTrimmedNP(o, qf, stess);
    }
  else
    {
      if(o->down && o->down->next && npatch->is_planar &&
	 ay_npt_istrimmed(o, 1))
	{
	  /* this is a trivially trimmed planar NURBS patch */
	  ay_status = ay_stess_TessTrimmedPlanarNP(o, qf, stess);
	}
    }

  if(ay_status == AY_ERROR)
    {
      /* trimmed tesselation failed or
	 this is an untrimmed or trivially trimmed NURBS patch,
	 where we can safely ignore potentially present trim curves... */
      ay_stess_destroy(stess);

      if(npatch->is_rat)
	{
	  ay_status = ay_stess_SurfacePoints4D(npatch->width, npatch->height,
					   npatch->uorder-1, npatch->vorder-1,
					       npatch->uknotv, npatch->vknotv,
					       npatch->controlv, qf,
					       &stess->tessw, &stess->tessh,
					       &stess->tessv);
	}
      else
	{
	  ay_status = ay_stess_SurfacePoints3D(npatch->width, npatch->height,
					   npatch->uorder-1, npatch->vorder-1,
					       npatch->uknotv, npatch->vknotv,
					       npatch->controlv, qf,
					       &stess->tessw, &stess->tessh,
					       &stess->tessv);
	} /* if israt */
    } /* if */

  stess->qf = qf;

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
    }

 return ay_status;
} /* ay_stess_TessNP */
