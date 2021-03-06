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

/* geom.c - miscellaneous small geometric algorithms */


/* ay_geom_intersectlines3D:
 *  calculate intersection of the two 3D lines <p1><t1> and <p2><t2>
 *  (where p is a point on the line and t is the tangent)
 *  outputs the point in <p>
 *  returns 0 when no intersection exists (collinear/parallel lines)
 *  otherwise returns 1
 */
int
ay_geom_intersectlines3D(double *p1, double *t1,
			 double *p2, double *t2,
			 double *p)
{
 double v[3] = {0}, px[3] = {0};
 double pd[3] = {0};
 double vw = 0.0, vw2 = 0.0;

  if(!p1 || !t1 || !p2 || !t2 || !p)
    return 0;

  AY_V3CROSS(px, t1, t2)
  AY_V3CROSS(v, px, t1)
  vw = AY_V3DOT(v, t2);

  if((vw*vw) < 1e-7)
    return 0;

  AY_V3SUB(pd, p1, p2)
  vw2 = AY_V3DOT(v, pd);
  p[0] = p2[0]+(vw2/vw)*t2[0];
  p[1] = p2[1]+(vw2/vw)*t2[1];
  p[2] = p2[2]+(vw2/vw)*t2[2];

 return 1;
} /* ay_geom_intersectlines3D */


/* ay_geom_intersectlines2D:
 *  calculate intersection of the two 2D lines <p1><t1> and <p2><t2>
 *  (where p is a point on the line and t is the tangent)
 *  outputs the point in <p>
 *  returns 0 when no intersection exists (collinear/parallel lines)
 *  otherwise returns 1
 */
int
ay_geom_intersectlines2D(double *p1, double *t1,
			 double *p2, double *t2,
			 double *p)
{
 double den, numa, ua/*, numb, ub */;

  if(!p1 || !t1 || !p2 || !t2 || !p)
    return 0;

  den = t2[1]*t1[0] - t2[0]*t1[1];

  if(fabs(den) < AY_EPSILON)
    return 0;

  numa = (t2[0]*(p1[1]-p2[1]) - t2[1]*(p1[0]-p2[0]));
  /*numb = (t1[0]*(p1[1]-p2[1]) - t1[1]*(p1[0]-p2[0]));*/

  if((fabs(numa) < AY_EPSILON)/* && (fabs(numb) < AY_EPSILON)*/)
    return 0;

  ua = numa/den;
  /*ub = numb/den;*/

  p[0] = p1[0] + ua*t1[0];
  p[1] = p1[1] + ua*t1[1];

 return 1;
} /* ay_geom_intersectlines2D */


/* ay_geom_pointstoplane:
 *  convert 3 points to plane equation coefficients
 */
void
ay_geom_pointstoplane(double x1, double y1, double z1,
		      double x2, double y2, double z2,
		      double x3, double y3, double z3,
		      double *A, double *B, double *C, double *D)
{

  *A = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
  *B = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
  *C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
  *D = -(x1 * (y2 * z3 - y3 * z2) + x2 * (y3 * z1 - y1 * z3) +
	 x3 * (y1 * z2 - y2 * z1));

 return;
} /* ay_geom_pointstoplane */


/** ay_geom_normalfrom3pnts:
 * Calculate normal vector for the plane spanned by three 3D points.
 * Does not check for equal points.
 *
 * \param[in] p1 first point
 * \param[in] p2 second point
 * \param[in] p3 third point
 * \param[in,out] n where to store the result
 */
void
ay_geom_normalfrom3pnts(double *p1, double *p2, double *p3, double *n)
{
 double v1[3], v2[3], len;

  if(!p1 || !p2 || !p3 || !n)
    return;

  AY_V3SUB(v2, p1, p2)
  AY_V3SUB(v1, p3, p2)
  AY_V3CROSS(n, v1, v2)
  len = AY_V3LEN(n);
  if(len > AY_EPSILON)
    AY_V3SCAL(n, 1.0/len)

 return;
} /* ay_geom_normalfrom3pnts */


/** ay_geom_anglefrom3pnts:
 * Calculate angle from three 3D points. If a normal is provided,
 * the angle will be signed with respect to this normal, otherwise
 * it will always be positive.
 * Does not check for equal points.
 *
 * \param[in] p1 first point
 * \param[in] p2 second point (apex of angle)
 * \param[in] p3 third point
 * \param[in] n normal vector (may be NULL)
 *
 * \returns angle in radians or 0.0 if any of p1, p2, p3 are NULL
 */
double
ay_geom_anglefrom3pnts(double *p1, double *p2, double *p3, double *n)
{
 double V1[3], V2[3], VC[3], angle, len, dir;

  if(!p1 || !p2 || !p3)
    return 0.0;

  AY_V3SUB(V1, p1, p2);
  len = AY_V3LEN(V1);
  if(len > AY_EPSILON)
    AY_V3SCAL(V1, 1.0/len)

  AY_V3SUB(V2, p3, p2);
  len = AY_V3LEN(V2);
  if(len > AY_EPSILON)
    AY_V3SCAL(V2, 1.0/len)

  angle = AY_V3DOT(V1, V2);
  if(angle <= -1.0)
    angle = -AY_PI;
  else
    if(angle >= 1.0)
      angle = 0.0;
    else
      angle = acos(angle);

  if(n)
    {
      /* normal provided => calculate direction */
      AY_V3CROSS(VC, V1, V2);

      dir = AY_V3DOT(VC, n);

      if(dir < 0)
	angle *= -1.0;
    }

 return angle;
} /* ay_geom_anglefrom3pnts */


/** ay_geom_extractmiddlepoint:
 * calculate center point from a closed curve
 * (or similar set of coordinates)
 *
 * \param[in] mode if 0 calculate the bounding box center, else calculate
 *  the center of gravity
 * \param[in] cv coordinate array
 * \param[in] cvlen number of points in \a cv
 * \param[in] cvstride size of a point in \a cv (>=3, unchecked)
 * \param[in,out] tcv temporary array of size 3*cvlen (to avoid reallocation
 *  for repeated calls, only used for mode 1), may be NULL if mode is 0
 * \param[in,out] result pointer where to store the center point ([3])
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_geom_extractmiddlepoint(int mode, double *cv, int cvlen, int cvstride,
			   double **tcv, double *result)
{
 int ay_status = AY_OK;
 int stride = 3, a, i, j;
 double *p = NULL, *t = NULL;
 double minmax[6];

  if(!result)
    return AY_ENULL;

  memset(result, 0, 3*sizeof(double));

  if(mode == 0)
    {
      if(!cv)
	return AY_ENULL;

      minmax[0] = DBL_MAX;
      minmax[1] = -DBL_MAX;
      minmax[2] = DBL_MAX;
      minmax[3] = -DBL_MAX;
      minmax[4] = DBL_MAX;
      minmax[5] = -DBL_MAX;
      p = cv;
      for(i = 0; i < cvlen; i++)
	{
	  if(p[0] < minmax[0])
	    minmax[0] = p[0];
	  if(p[0] > minmax[1])
	    minmax[1] = p[0];

	  if(p[1] < minmax[2])
	    minmax[2] = p[1];
	  if(p[1] > minmax[3])
	    minmax[3] = p[1];

	  if(p[2] < minmax[4])
	    minmax[4] = p[2];
	  if(p[2] > minmax[5])
	    minmax[5] = p[2];

	  p += cvstride;
	} /* for */

      result[0] = minmax[0]+((minmax[1]-minmax[0])*0.5);
      result[1] = minmax[2]+((minmax[3]-minmax[2])*0.5);
      result[2] = minmax[4]+((minmax[5]-minmax[4])*0.5);
    }
  else
    {
      /* mode != 0 */
      if(!tcv)
	return AY_ENULL;

      if(!*tcv)
	if(!(*tcv = malloc(cvlen*stride*sizeof(double))))
	  return AY_EOMEM;
      t = *tcv;
      p = cv;
      for(i = 0; i < cvlen; i++)
	{
	  memcpy(&(t[i*stride]), p, stride*sizeof(double));
	  p += cvstride;
	}

      qsort(t, cvlen, stride*sizeof(double), ay_nct_cmppnt);

      a = 0;
      i = 0;
      j = cvlen;
      while(i < cvlen)
	{
	  result[0] += t[a];
	  result[1] += t[a+1];
	  result[2] += t[a+2];

	  /* skip over sequence of equal points */
	  if((i < (cvlen-1)) &&
	     !ay_nct_cmppnt((void*)(&(t[a])),(void*)(&(t[a+stride]))))
	    {
	      do
		{
		  i++;
		  a += stride;
		  j--;
		}
	      while((i < (cvlen-1)) &&
		!ay_nct_cmppnt((void*)(&(t[a])),(void*)(&(t[a+stride]))));
	    }

	  i++;
	  a += stride;
	} /* while */

      result[0] /= j;
      result[1] /= j;
      result[2] /= j;
    } /* if */

 return ay_status;
} /* ay_geom_extractmiddlepoint */


/** ay_geom_extractmeannormal:
 * calculate the mean normal from a closed curve
 *  (or similar set of coordinates)
 *
 * \param[in] cv coordinate array
 * \param[in] cvlen number of points in \a cv
 * \param[in] cvstride size of a point in \a cv (>=3, unchecked)
 * \param[in] m center point in which the normal is calculated, may be NULL
 * \param[in,out] result pointer where the resulting normal is stored ([3])
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_geom_extractmeannormal(double *cv, int cvlen, int cvstride,
			  double *m, double *result)
{
 int ay_status = AY_OK;
 int i, snlen = 0;
 double mm[3] = {0}, *p1, *p2, *psn;
 double len, *sn = NULL;

  if(!cv || !result)
    return AY_ENULL;

  if(!m)
    {
      ay_status = ay_geom_extractmiddlepoint(0, cv, cvlen,
					     cvstride, NULL, mm);

      if(ay_status)
	return ay_status;

      m = mm;
    }

  if(!(sn = calloc(cvlen, 3*sizeof(double))))
    return AY_EOMEM;

  psn = sn;
  p1 = cv;
  p2 = p1+cvstride;

  for(i = 0; i < cvlen-1; i++)
    {
      if((!AY_V3COMP(p1,p2)) && (!AY_V3COMP(m,p1)) && (!AY_V3COMP(m,p2)))
	{
	  ay_geom_normalfrom3pnts(m, p1, p2, psn);
	  len = AY_V3LEN(psn);
	  if(len == len)
	    {
	      if(fabs(len) > AY_EPSILON)
		{
		  AY_V3SCAL(psn,1.0/len);
		}
	      snlen++;
	      psn += 3;
	    }
	}

      p1 = p2;
      p2 += cvstride;
    }

  memset(result, 0, 3*sizeof(double));

  psn = sn;
  for(i = 0; i < snlen; i++)
    {
      result[0] += psn[0]/snlen;
      result[1] += psn[1]/snlen;
      result[2] += psn[2]/snlen;
      psn += 3;
    }

  len = AY_V3LEN(result);
  if(fabs(len) > AY_EPSILON)
    {
      AY_V3SCAL(result, 1.0/len);
    }

  free(sn);

 return AY_OK;
} /* ay_geom_extractmeannormal */


/** ay_geom_scalecog:
 *  scale a number of points around their center of gravity
 *
 * \param[in] scale scale factor
 * \param[in,out] cv points to scale
 * \param[in] len number of points
 * \param[in] stride memory distance between two points (>=3, unchecked)
 */
void
ay_geom_scalecog(double scale, double *cv, int len, int stride)
{
 int i;
 double *p, cog[3] = {0};

  if(!cv)
    return;

  p = cv;
  for(i = 0; i < len; i++)
    {
      cog[0] += (p[0]/len);
      cog[1] += (p[1]/len);
      cog[2] += (p[2]/len);

      p += stride;
    } /* for */

  p = cv;
  for(i = 0; i < len; i++)
    {
      p[0] -= cog[0];
      p[1] -= cog[1];
      p[2] -= cog[2];

      p[0] *= scale;
      p[1] *= scale;
      p[2] *= scale;

      p[0] += cog[0];
      p[1] += cog[1];
      p[2] += cog[2];

      p += stride;
    } /* for */

 return;
} /* ay_geom_scalecog */
