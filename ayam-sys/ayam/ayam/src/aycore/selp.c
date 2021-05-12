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

/* selp.c - selected points related functions */

/* ay_selp_clear:
 *  clear list of selected points from object o
 */
void
ay_selp_clear(ay_object *o)
{
 ay_point *next = NULL, *p = NULL;

  if(!o)
    return;

  p = o->selp;

  while(p)
    {
      next = p->next;
      free(p);
      p = next;
    }

  o->selp = NULL;

 return;
} /* ay_selp_clear */


/* ay_selp_copy:
 *  copy list of selected points
 */
void
ay_selp_copy(ay_point *pnt, ay_point **res)
{
 ay_point *newpnt = NULL;

  if(!pnt || !res)
    return;

  while(pnt)
    {
      if(!(newpnt = malloc(sizeof(ay_point))))
	return;

      memcpy(newpnt, pnt, sizeof(ay_point));

      newpnt->next = *res;
      *res = newpnt;

      pnt = pnt->next;
    }

 return;
} /* ay_selp_copy */


/* ay_selp_selall:
 *  select (tag) all points
 */
int
ay_selp_selall(ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char fname[] = "selp_selall";
 double p[3] = {DBL_MIN, DBL_MIN, DBL_MIN};
 unsigned int i = 0;
 ay_point *newp = NULL;
 ay_pointedit pe = {0};

  if(!o)
    return AY_ENULL;

  ay_selp_clear(o);
  ay_pact_getpoint(0, o, p, &pe);

  for(i = 0; i < pe.num; i++)
    {
      if(!(newp = malloc(sizeof(ay_point))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}

      newp->next = o->selp;
      o->selp = newp;
      newp->point = pe.coords[i];
      if(pe.indices)
	{
	  newp->index = pe.indices[i];
	}
      else
	{
	  newp->index = 0;
	}
      newp->type = pe.type;
      newp->readonly = pe.readonly;
    } /* for */

  ay_pact_clearpointedit(&pe);

 return AY_OK;
} /* ay_selp_selall */


/** ay_selp_applytrafotcmd:
 *  Apply transformations to editable points of selected objects.
 *
 *  Implements the \a applyTrafo scripting interface command.
 *  See also the corresponding section in the \ayd{scapplytrafo}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_selp_applytrafotcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_point *bak = NULL, *p = NULL;
 int all = AY_TRUE, notify_parent = AY_FALSE;
 double m[16];

  if(argc > 2)
    {
      ay_error(AY_EARGS, argv[0], "[-sel]");
      return TCL_OK;
    }

  if((argc > 1) && !strcmp(argv[1], "-sel"))
    {
      all = AY_FALSE;
    }

  while(sel)
    {
      o = sel->object;
      /* so that we may use continue */
      sel = sel->next;

      if(all)
	{
	  bak = o->selp;
	  o->selp = NULL;
	  ay_status = ay_selp_selall(o);
	  if(ay_status)
	    {
	      /* object has no points? */
	      ay_selp_clear(o);
	      o->selp = bak;
	      continue;
	    }
	}

      if(o->selp && !o->selp->readonly)
	{
	  /* create trafo */
	  ay_trafo_creatematrix(o, m);

	  /* transform points */
	  p = o->selp;
	  while(p)
	    {
	      ay_trafo_apply3(p->point, m);

	      p = p->next;
	    }

	  /* clear trafo */
	  ay_trafo_defaults(o);
	  (void)ay_notify_object(o);
	  notify_parent = AY_TRUE;
	}

      /* restore old selected points */
      if(all)
	{
	  ay_selp_clear(o);
	  o->selp = bak;
	}
    } /* while */

  if(notify_parent)
    {
      (void)ay_notify_parent();
    }

 return TCL_OK;
} /* ay_selp_applytrafotcmd */


/* ay_selp_invert:
 *  invert list of selected points for object o
 */
int
ay_selp_invert(ay_object *o)
{
 int ay_status = AY_OK;
 ay_point *t = NULL, *p = NULL, **last2 = NULL, *p2 = NULL;

  if(!o)
    return AY_ENULL;

  if(!o->selp)
    {
      ay_status = ay_selp_selall(o);
      return ay_status;
    }

  /* save old selection of points to p */
  p = o->selp;

  /* select all points */
  o->selp = NULL;
  ay_status = ay_selp_selall(o);
  if(ay_status)
    {
      ay_selp_clear(o);
      o->selp = p;
      return ay_status;
    }

  /* remove all points, that are in p (old selection), from p2 (new
     selection); additionally, clear p */
  while(p)
    {
      last2 = &(o->selp);
      p2 = o->selp;
      while(p2)
	{
	  if(p->point == p2->point)
	    {
	      *last2 = p2->next;
	      free(p2);
	      break;
	    }
	  else
	    {
	      last2 = &(p2->next);
	      p2 = p2->next;
	    } /* if */
	} /* while */
      t = p;
      p = p->next;
      free(t);
    } /* while */

 return AY_OK;
} /* ay_selp_invert */


/** ay_selp_inverttcmd:
 *  Invert point selection of selected objects.
 *
 *  Implements the \a invPnts scripting interface command.
 *  See also the corresponding section in the \ayd{scinvpnts}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_selp_inverttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  while(sel)
    {
      o = sel->object;

      ay_status = ay_selp_invert(o);

      if(ay_status)
	{
	  ay_error(AY_ERROR, argv[0], NULL);
	  return TCL_OK;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_selp_inverttcmd */


/* ay_selp_center:
 *  center all selected points (readonly points will be
 *  omitted silently)
 *  p - (selected) points to center
 *  dim - control in which dimensions centering shall occur
 *   0: all dimensions
 *   1: only x-y
 *   2: only y-z
 *   3: only x-z
 *  center - center
 */
int
ay_selp_center(ay_point *p, int dim, double *center)
{
 int ay_status = AY_OK;
 ay_point *pnt = NULL;

  if(!p || !center)
    return AY_ENULL;

  /* center points, by translating them */
  pnt = p;
  switch(dim)
    {
    case 0:
      while(pnt)
	{
	  if(!pnt->readonly)
	    {
	      pnt->point[0] -= center[0];
	      pnt->point[1] -= center[1];
	      pnt->point[2] -= center[2];
	    }
	  pnt = pnt->next;
	}
      break;
    case 1:
      while(pnt)
	{
	  if(!pnt->readonly)
	    {
	      pnt->point[0] -= center[0];
	      pnt->point[1] -= center[1];
	    }
	  pnt = pnt->next;
	}
      break;
    case 2:
      while(pnt)
	{
	  if(!pnt->readonly)
	    {
	      pnt->point[1] -= center[1];
	      pnt->point[2] -= center[2];
	    }
	  pnt = pnt->next;
	}
      break;
    case 3:
      while(pnt)
	{
	  if(!pnt->readonly)
	    {
	      pnt->point[0] -= center[0];
	      pnt->point[2] -= center[2];
	    }
	  pnt = pnt->next;
	}
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ay_selp_center */


/* ay_selp_getcenter:
 *  calculate center of selected points
 *  p - (selected) points to calculate the center from
 *  mode - which center
 *   0: minmax bbox
 *   1: COG
 *  center - center
 */
int
ay_selp_getcenter(ay_point *p, int mode, double *center)
{
 int ay_status = AY_OK, is_rat = AY_FALSE;
 double xmin = DBL_MAX, xmax = -DBL_MAX;
 double ymin = DBL_MAX, ymax = -DBL_MAX;
 double zmin = DBL_MAX, zmax = -DBL_MAX;
 unsigned int i = 0, j = 0, nump = 0, numpu = 0;
 ay_point *pnt = NULL;
 double **pnts = NULL, *e, *hpnts = NULL, h[3];

  if(!p || !center)
    return AY_ENULL;

  pnt = p;

  switch(mode)
    {
    case 0:
      /* compute min-max-bbox center */
      while(pnt)
	{
	  if((pnt->type == AY_PTRAT) && ay_prefs.rationalpoints)
	    {
	      e = h;
	      h[0] = pnt->point[0]*pnt->point[3];
	      h[1] = pnt->point[1]*pnt->point[3];
	      h[2] = pnt->point[2]*pnt->point[3];
	    }
	  else
	    {
	      e = pnt->point;
	    }

	  if(e[0] < xmin)
	    xmin = e[0];
	  if(e[0] > xmax)
	    xmax = e[0];

	  if(e[1] < ymin)
	    ymin = e[1];
	  if(e[1] > ymax)
	    ymax = e[1];

	  if(e[2] < zmin)
	    zmin = e[2];
	  if(e[2] > zmax)
	    zmax = e[2];

	  pnt = pnt->next;
	} /* while pnt */
      center[0] = xmin+(xmax-xmin)*0.5;
      center[1] = ymin+(ymax-ymin)*0.5;
      center[2] = zmin+(zmax-zmin)*0.5;
      break;
    case 1:
      /* compute COG */

      /* first count selected points */
      while(pnt)
	{
	  nump++;
	  if(pnt->type == AY_PTRAT)
	    is_rat = AY_TRUE;
	  pnt = pnt->next;
	}
      if(!(pnts = malloc(nump * sizeof(double*))))
	{
	  return AY_EOMEM;
	}
      pnt = p;

      if(is_rat && ay_prefs.rationalpoints)
	{
	  /* create temporary homogeneous coordinates */
	  if(!(hpnts = malloc(nump * 3 * sizeof(double))))
	    {
	      free(pnts);
	      return AY_EOMEM;
	    }
	  for(i = 0; i < nump; i++)
	    {
	      pnts[i] = &(hpnts[j]);
	      if(pnt->type == AY_PTRAT)
		{
		  hpnts[j]   = pnt->point[0]*pnt->point[3];
		  hpnts[j+1] = pnt->point[1]*pnt->point[3];
		  hpnts[j+2] = pnt->point[2]*pnt->point[3];
		}
	      else
		{
		  memcpy(&(hpnts[j]), pnt->point, 3*sizeof(double));
		}
	      j += 3;
	      pnt = pnt->next;
	    }
	}
      else
	{
	  for(i = 0; i < nump; i++)
	    {
	      pnts[i] = pnt->point;
	      pnt = pnt->next;
	    }
	}

      if(nump > 1)
	{
	  qsort(pnts, nump, sizeof(double*), ay_nct_cmppntp);
	  /* calculate the number of unique points */
	  numpu = nump;
	  for(i = 0; i < nump-1; i++)
	    {
	      if(!ay_nct_cmppntp(&(pnts[i]), &(pnts[i+1])))
		{
		  numpu--;
		}
	    }
	  /* for the special case of two equal points, make
	     sure we have the first of the points as cog */
	  center[0] = (pnts[0])[0]/(double)numpu;
	  center[1] = (pnts[0])[1]/(double)numpu;
	  center[2] = (pnts[0])[2]/(double)numpu;

	  /* calculate the cog */
	  for(i = 1; i < nump; i++)
	    {
	      if(ay_nct_cmppntp(&(pnts[i-1]), &(pnts[i])))
		{
		  center[0] += (pnts[i])[0]/(double)numpu;
		  center[1] += (pnts[i])[1]/(double)numpu;
		  center[2] += (pnts[i])[2]/(double)numpu;
		}
	    }
	}
      else
	{
	  /* just one point => this is the cog */
	  center[0] = (pnts[0])[0];
	  center[1] = (pnts[0])[1];
	  center[2] = (pnts[0])[2];
	}

      if(hpnts)
	free(hpnts);
      free(pnts);
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ay_selp_getcenter */


/** ay_selp_centertcmd:
 *  Moves all points of the selected objects so that their
 *  center is identical to the object coordinate systems center.
 *
 *  Implements the \a centerPnts scripting interface command.
 *  See also the corresponding section in the \ayd{sccenterpnts}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_selp_centertcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_point *oldpointsel = NULL;
 ay_object *o = NULL;
 double center[3] = {0};
 int dim = 0, mode = 0, notify_parent = AY_FALSE;

  if(!strcmp(argv[0], "centerNC"))
    {
      ay_error(AY_EWARN, argv[0], "Command is deprecated, please use centerPnts instead.");
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc > 1)
    {
      tcl_status = Tcl_GetInt(interp, argv[1], &dim);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }

  if(argc > 2)
    {
      tcl_status = Tcl_GetInt(interp, argv[2], &mode);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }

  while(sel)
    {
      o = sel->object;

      /* save old point selection */
      oldpointsel = o->selp;
      o->selp = NULL;

      /* center all points */
      ay_status = ay_selp_selall(o);
      if(!ay_status)
	{
	  ay_status = ay_selp_getcenter(o->selp, mode, center);
	  if(!ay_status)
	    {
	      ay_status = ay_selp_center(o->selp, dim, center);
	    }
	}

      if(ay_status)
	{
	  ay_error(ay_status, argv[0], "Could not center.");
	}
      else
	{
	  ay_notify_object(o);
	  o->modified = AY_TRUE;
	  notify_parent = AY_TRUE;
	}

      /* recover point selection */
      ay_selp_clear(o);
      o->selp = oldpointsel;

      sel = sel->next;
    } /* while */

  if(notify_parent)
    {
      (void)ay_notify_parent();
    }

 return TCL_OK;
} /* ay_selp_centertcmd */


/* ay_selp_sel:
 *  select points from indices
 */
int
ay_selp_sel(ay_object *o, unsigned int indiceslen, unsigned int *indices)
{
 int ay_status = AY_OK;
 ay_point *po = NULL, *p = NULL;
 unsigned int i = 0;
 char have_it = AY_FALSE;

  if(!o || !indices)
    return AY_ENULL;

  for(i = 0; i < indiceslen; i++)
    {
      if(!po)
	{
	  if(!(po = calloc(1, sizeof(ay_point))))
	    {
	      return AY_EOMEM;
	    }
	}
      have_it = AY_FALSE;
      p = o->selp;
      while(p)
	{
	  if(indices[i] == p->index)
	    {
	      have_it = AY_TRUE;
	      break;
	    }
	  p = p->next;
	}
      if(!have_it)
	{
	  po->index = indices[i];
	  po->next = o->selp;
	  o->selp = po;
	  po = NULL;
	}
    } /* for */

  /* update pointers */
  ay_status = ay_pact_getpoint(3, o, NULL, NULL);

  /* cleanup */
  if(po)
    free(po);

 return ay_status;
} /* ay_selp_sel */


/** ay_selp_seltcmd:
 *  Select points of selected objects.
 *
 *  Implements the \a selPnts scripting interface command.
 *  See also the corresponding section in the \ayd{scselpnts}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_selp_seltcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 unsigned int i = 1, indiceslen = 0, *indices = NULL;
 int count = AY_FALSE, select_all = AY_FALSE, select_none = AY_FALSE;
 int return_result = AY_FALSE;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_point *p;
 Tcl_Obj *to = NULL, *tor = NULL;

  /* parse args */
  if(argc == 1)
    {
      select_none = AY_TRUE;
    }
  else
    {
      if(argv[1][0] == '-')
	{
	  /* -all */
	  if(argv[1][1] == 'a')
	    {
	      select_all = AY_TRUE;
	    }
	  /* -count */
	  if(argv[1][1] == 'c')
	    {
	      if(argc != 3)
		{
		  return_result = AY_TRUE;
		  tor = Tcl_NewListObj(0, NULL);
		}
	      else
		{
		  Tcl_SetVar(interp, argv[2], "", TCL_LEAVE_ERR_MSG);
		}
	      count = AY_TRUE;
	    } /* if -count */
	  /* -none */
	  if(argv[1][1] == 'n')
	    {
	      select_none = AY_TRUE;
	    }
	  /* -get */
	  if(argv[1][1] == 'g')
	    {
	      if(argc != 3)
		{
		  return_result = AY_TRUE;
		  tor = Tcl_NewListObj(0, NULL);
		}
	      else
		{
		  Tcl_SetVar(interp, argv[2], "", TCL_LEAVE_ERR_MSG);
		}

	      if(sel)
		{
		  o = sel->object;
		  p = o->selp;
		  while(p)
		    {
		      if(p->index < (unsigned int)INT_MAX)
			to = Tcl_NewIntObj(p->index);
		      else
			to = Tcl_NewWideIntObj((Tcl_WideInt)p->index);

		      if(return_result)
			Tcl_ListObjAppendElement(interp, tor, to);
		      else
			Tcl_SetVar2Ex(interp, argv[2], NULL, to,
				      TCL_APPEND_VALUE | TCL_LIST_ELEMENT |
				      TCL_LEAVE_ERR_MSG);
		      p = p->next;
		    }
		} /* if sel */

	      if(return_result)
		{
		  Tcl_SetObjResult(interp, tor);
		}

	      return TCL_OK;
	    } /* if -get */
	  /* -has */
	  if(argv[1][1] == 'h' && argv[1][2] == 'a')
	    {
	      to = Tcl_NewIntObj(0);

	      if(sel)
		{
		  o = sel->object;
		  while(o)
		    {
		      if(o->selp)
			{
			  to = Tcl_NewIntObj(1);
			  break;
			}
		      o = o->next;
		    }
		} /* if sel */

	      Tcl_SetObjResult(interp, to);

	      return TCL_OK;
	    } /* if -has */
	  /* -help */
	  if(argv[1][1] == 'h' && argv[1][2] == 'e')
	    {
	      ay_error(AY_EARGS, argv[0],
   "[-all | -none | -get vname | -have | -count vname | ind1 ind2 ... indn]");
	      goto cleanup;
	    }
	} /* if arg is option */
    } /* if have args */

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      goto cleanup;
    }

  /* parse list of indices */
  if(!count && !select_none && !select_all)
    {
      if(!(indices = calloc(argc, sizeof(unsigned int))))
	{
	  ay_error(AY_EOMEM, argv[0], NULL);
	  goto cleanup;
	}
      while(i < (unsigned int)argc)
	{
	  ay_status = ay_tcmd_getuint(argv[i], &(indices[indiceslen]));
	  if(ay_status)
	    {
	      goto cleanup;
	    }
	  indiceslen++;
	  i++;
	} /* while */
    } /* if */

  while(sel)
    {
      o = sel->object;
      if(count)
	{
	  i = 0;
	  p = o->selp;
	  while(p)
	    {
	      i++;
	      p = p->next;
	    }

	  if(i < (unsigned int)INT_MAX)
	    to = Tcl_NewIntObj(i);
	  else
	    to = Tcl_NewWideIntObj((Tcl_WideInt)i);

	  if(return_result)
	    Tcl_ListObjAppendElement(interp, tor, to);
	  else
	    Tcl_SetVar2Ex(interp, argv[2], NULL, to, TCL_APPEND_VALUE |
			   TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);

	  sel = sel->next;
	  continue;
	}

      if(select_none)
	{
	  ay_selp_clear(o);
	}
      else
	{
	  if(select_all)
	    {
	      ay_status = ay_selp_selall(o);

	      /* it is no error if we could not select any points, when
		 the user requested no specific points, but "all" */
	      if(ay_status == AY_ERROR)
		{
		  ay_status = AY_OK;
		}
	    }
	  else
	    {
	      ay_status = ay_selp_sel(o, indiceslen, indices);
	    }
	} /* if */

      if(ay_status)
	{
	  ay_error(AY_ERROR, argv[0], NULL);
	  /*
	  goto cleanup;
	  */
	}

      o->modified = 2;
      (void)ay_notify_object(o);

      sel = sel->next;
    } /* while */

cleanup:

  if(indices)
    free(indices);

  if(tor)
    {
      Tcl_SetObjResult(interp, tor);
    }

 return TCL_OK;
} /* ay_selp_seltcmd */


/** ay_selp_rem:
 * helper for interactive curve point removal action,
 * updates the list of selected points
 *
 * \param[in,out] o curve object to process
 * \param[in] index designates the removed point
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_selp_rem(ay_object *o, unsigned int index)
{
 int ay_status = AY_OK;
 ay_point **pp = NULL, *p = NULL;

  if(!o)
    return AY_ENULL;

  /* remove point with matching index */
  p = o->selp;
  pp = &(o->selp);
  while(p)
    {
      if(index == p->index)
	{
	  *pp = p->next;
	  free(p);
	  break;
	}
      pp = &(p->next);
      p = p->next;
    }

  /* readjust indices of remaining points */
  p = o->selp;
  while(p)
    {
      if(p->index > index)
	{
	  p->index--;
	}
      p = p->next;
    }

  /* update pointers */
  ay_status = ay_pact_getpoint(3, o, NULL, NULL);

 return ay_status;
} /* ay_selp_rem */


/** ay_selp_ins:
 * helper for interactive curve point insertion action,
 * updates the list of selected points
 *
 * \param[in,out] o curve object to process
 * \param[in] index designates the point after which the new point is inserted
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_selp_ins(ay_object *o, unsigned int index)
{
 int ay_status = AY_OK;
 ay_point *p = NULL;

  if(!o)
    return AY_ENULL;

  /* readjust indices of old points */
  p = o->selp;

  while(p)
    {
      if(p->index > index )
	{
	  p->index++;
	}
      p = p->next;
    }

  /* update pointers */
  ay_status = ay_pact_getpoint(3, o, NULL, NULL);

 return ay_status;
} /* ay_selp_ins */


/* ay_selp_getpnts:
 *  getpnts callback helper, many objects call into this function from
 *  their getpnts callback to realize picking of points
 */
int
ay_selp_getpnts(int mode, ay_object *o, double *p, ay_pointedit *pe,
		int readonly, int arrlen, int stride, int ishom, double *arr)
{
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double **pecoords = NULL, **pecoordstmp, *pecoord = NULL, *c;
 double h[3];
 int i = 0, j = 0, a = 0;
 unsigned int *peindices = NULL, *peindicestmp, peindex = 0;

  if(!o || !arr)
    return AY_ENULL;

  if((mode != 3) && (!p || !pe))
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    {
      if(stride == 4 && ishom)
	pe->type = AY_PTRAT;

      pe->readonly = readonly;
    }

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = malloc(arrlen * sizeof(double*))))
	return AY_EOMEM;

      if(!(pe->indices = malloc(arrlen * sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < arrlen; i++)
	{
	  pe->coords[i] = &(arr[a]);
	  pe->indices[i] = i;
	  a += stride;
	}

      pe->num = arrlen;
      break;
    case 1:
      /* selection based on a single point */
      for(i = 0; i < arrlen; i++)
	{
	  if(stride == 4 && ishom)
	    {
	      dist = AY_VLEN((p[0] - arr[j]*arr[j+3]),
			     (p[1] - arr[j+1]*arr[j+3]),
			     (p[2] - arr[j+2]*arr[j+3]));
	    }
	  else
	    {
	      dist = AY_VLEN((p[0] - arr[j]),
			     (p[1] - arr[j+1]),
			     (p[2] - arr[j+2]));
	    }
	  if(dist < min_dist)
	    {
	      pecoord = &(arr[j]);
	      peindex = i;
	      min_dist = dist;
	    }

	  j += stride;
	} /* for */

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!(pe->coords = malloc(sizeof(double *))))
	return AY_EOMEM;

      if(!(pe->indices = malloc(sizeof(unsigned int))))
	return AY_EOMEM;

      pe->coords[0] = pecoord;
      pe->indices[0] = peindex;
      pe->num = 1;
      break;
    case 2:
      /* selection based on planes */
      j = 0;
      a = 0;
      for(i = 0; i < arrlen; i++)
	{
	  if(stride == 4 && ishom)
	    {
	      c = h;
	      h[0] = arr[j]*arr[j+3];
	      h[1] = arr[j+1]*arr[j+3];
	      h[2] = arr[j+2]*arr[j+3];
	    }
	  else
	    {
	      c = &(arr[j]);
	    }

	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {
	      if(!(pecoordstmp = realloc(pecoords, (a+1)*sizeof(double *))))
		{
		  if(pecoords)
		    free(pecoords);
		  if(peindices)
		    free(peindices);
		  return AY_EOMEM;
		}
	      pecoords = pecoordstmp;
	      if(!(peindicestmp = realloc(peindices,
				       (a+1)*sizeof(unsigned int))))
		{
		  if(pecoords)
		    free(pecoords);
		  if(peindices)
		    free(peindices);
		  return AY_EOMEM;
		}
	      peindices = peindicestmp;

	      pecoords[a] = &(arr[j]);
	      peindices[a] = i;
	      a++;
	    } /* if */

	  j += stride;
	} /* for */

      if(!pecoords)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      pe->coords = pecoords;
      pe->indices = peindices;
      pe->num = a;
      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)arrlen)
	    {
	      pnt->point = &(arr[pnt->index*stride]);
	      if(stride == 4 && ishom)
		pnt->type = AY_PTRAT;
	      pnt->readonly = readonly;
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
      return AY_ERROR;
      break;
    } /* switch */

 return AY_OK;
} /* ay_selp_getpnts */


/** ay_selp_selectmpnc:
 * properly select all points of multiple points of a NURBS curve
 *
 * \param[in,out] o NURBS curve object to process
 * \param[in] select_all if AY_TRUE, all points of all multiple points
 *   will be selected;
 *  if AY_FALSE, only multiple points where there is already
 *   a single point of it selected will be considered
 */
void
ay_selp_selectmpnc(ay_object *o, int select_all)
{
 ay_point *p = NULL, **nextp = NULL, *newp = NULL;
 ay_mpoint *mp = NULL;
 ay_nurbcurve_object *nc;
 int i, found = AY_FALSE;

  if(!o || (o->type != AY_IDNCURVE))
    return;

  nc = (ay_nurbcurve_object *)o->refine;

  if(nc->createmp)
    {
      mp = nc->mpoints;
      while(mp)
	{
	  nextp = &(o->selp);

	  if(!select_all)
	    {
	      found = AY_FALSE;

	      for(i = 0; i < mp->multiplicity; i++)
		{
		  p = o->selp;

		  while(p)
		    {
		      if(p->index == mp->indices[i])
			{
			  found = AY_TRUE;
			  break;
			}
		      p = p->next;
		    } /* while */
		  if(found)
		    {
		      break;
		    } /* if */
		} /* for */
	    } /* if */

	  if(select_all || found)
	    {
	      for(i = 0; i < mp->multiplicity; i++)
		{
		  p = o->selp;
		  found = AY_FALSE;
		  while(p)
		    {
		      if(p->index == mp->indices[i])
			{
			  /* advancing "nextp" ensures that we append
			     new selected points right after the
			     other already selected points of this
			     multiple points */
			  nextp = &(p->next);
			  found = AY_TRUE;
			  break;
			}
		      p = p->next;
		    } /* while */
		  if(!found)
		    {
		      if(!(newp = calloc(1, sizeof(ay_point))))
			{
			  return;
			}
		      newp->next = *nextp;
		      *nextp = newp;
		      newp->point = mp->points[i];
		      newp->index = mp->indices[i];
		    } /* if */
		} /* for */
	    } /* if */
	  mp = mp->next;
	} /* while */
    } /* if */

 return;
} /* ay_selp_selectmpnc */


/** ay_selp_normalize:
 *  Normalize selected points.
 *
 * \param[in,out] o object to process
 * \param[in] digits number of significant figures to keep
 */
void
ay_selp_normalize(ay_object *o, int digits)
{
 ay_point *pnt;
 double *p;

  if(!o || !o->selp || digits <= 0)
    return;

  pnt = o->selp;

  while(pnt)
    {
      p = pnt->point;

      *p = ay_trafo_round(*p, digits);
      p++;
      *p = ay_trafo_round(*p, digits);
      p++;
      *p = ay_trafo_round(*p, digits);

      if(pnt->type == AY_PTRAT)
	{
	  p++;
	  *p = ay_trafo_round(*p, digits);
	}

      pnt = pnt->next;
    }

 return;
} /* ay_selp_normalize */


/** ay_selp_collapsetcmd:
 *  Collapse selected points of selected NURBS curves/patches to
 *  multiple points.
 *  Implements the \a collMP scripting interface command.
 *  See also the corresponding section in the \ayd{sccollmp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_selp_collapsetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 int notify_parent = AY_FALSE;
 char fmsg[] = "Collapse operation failed.";
 ay_list_object *sel = ay_selection;
 ay_object *o;
 ay_point *pnt;
 double center[3] = {0};

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->selp)
	{
	  pnt = o->selp;
	  if(!pnt->readonly)
	    {
	      ay_selp_getcenter(pnt, 0, center);
	      while(pnt)
		{
		  memcpy(pnt->point, center, 3*sizeof(double));
		  pnt = pnt->next;
		}

	      switch(o->type)
		{
		case AY_IDNCURVE:
		  {
		    ay_status = ay_nct_collapseselp(o);
		    if(ay_status)
		      {
			ay_error(ay_status, argv[0], fmsg);
		      }
		  }
		  break;
		case AY_IDNPATCH:
		  {
		    ay_status = ay_npt_collapseselp(o);
		    if(ay_status)
		      {
			ay_error(ay_status, argv[0], fmsg);
		      }
		  }
		  break;
		default:
		  break;
		} /* switch */

	      (void)ay_notify_object(o);
	      sel->object->modified = AY_TRUE;
	      notify_parent = AY_TRUE;
	    } /* if have editable points */
	} /* if have selected points */
      sel = sel->next;
    } /* while */

  if(notify_parent)
    {
      (void)ay_notify_parent();
    }

 return TCL_OK;
} /* ay_selp_collapsetcmd */


/** ay_selp_explodetcmd:
 *  Explode selected multiple points of selected NURBS curves/patches to
 *  single points.
 *  Implements the \a explMP scripting interface command.
 *  See also the corresponding section in the \ayd{scexplmp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_selp_explodetcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 int notify_parent = AY_FALSE;
 char fmsg[] = "Explode operation failed.";
 ay_object *o;
 ay_list_object *sel = ay_selection;
 ay_point *opnt, *fpnt, *pnt, before = {0}, after = {0};
 double v[3] = {0}, vlen;
 unsigned int i, j, n;
 ay_voidfp *arr = NULL;
 ay_getpntcb *cb = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  arr = ay_getpntcbt.arr;

  while(sel)
    {
      o = sel->object;
      if(o->selp)
	{
	  pnt = o->selp;
	  switch(o->type)
	    {
	    case AY_IDNCURVE:
	      ay_status = ay_nct_explodemp(o);
	      if(ay_status >= AY_ERROR)
		{
		  ay_error(ay_status, argv[0], fmsg);
		}
	      break;
	    case AY_IDNPATCH:
	      ay_status = ay_npt_explodemp(o);
	      if(ay_status >= AY_ERROR)
		{
		  ay_error(ay_status, argv[0], fmsg);
		}
	      break;
	    default:
	      break;
	    } /* switch */

	    cb = (ay_getpntcb *)(arr[o->type]);
	    if(pnt && !pnt->readonly && cb)
	      {
		fpnt = pnt;
		n = 0;
		before.index = pnt->index;
		after.index = pnt->index;
		while(pnt && AY_V3COMP(fpnt->point,pnt->point))
		  {
		    if(before.index > pnt->index)
		     before.index = pnt->index;
		    if(after.index < pnt->index)
		      after.index = pnt->index;
		    n++;
		    pnt = pnt->next;
		  }
		if(n > 1)
		  {
		    opnt = o->selp;
		    o->selp = NULL;
		    after.index++;
		    if(before.index > 0)
		      {
			before.index--;
			before.next = &(after);
			ay_selp_copy(&before, &(o->selp));
		      }
		    else
		      {
			ay_selp_copy(&after, &(o->selp));
		      }
		    ay_status = cb(3, o, NULL, NULL);
		    pnt = opnt;
		    if(o->selp)
		      {
			if(o->selp->next)
			  {
			    /* have two sections */
			    AY_V3SUB(v, o->selp->point, pnt->point);
			    j = 1;
			    for(i = 0; i < n/2; i++)
			      {
				pnt->point[0] += (v[0]/(double)n)*j;
				pnt->point[1] += (v[1]/(double)n)*j;
				pnt->point[2] += (v[2]/(double)n)*j;
				j++;
				pnt = pnt->next;
			      }

			    if(n%2)
			      pnt = pnt->next;

			    AY_V3SUB(v, o->selp->next->point, pnt->point);
			    j = 1;
			    for(i = n/2+((n%2)?1:0); i < n; i++)
			      {
				pnt->point[0] += (v[0]/(double)n)*j;
				pnt->point[1] += (v[1]/(double)n)*j;
				pnt->point[2] += (v[2]/(double)n)*j;
				j++;
				pnt = pnt->next;
			      }
			  }
			else
			  {
			    /* have just one section */
			    AY_V3SUB(v, pnt->point, o->selp->point);
			    j = 1;
			    for(i = 0; i < n; i++)
			      {
				pnt->point[0] += (v[0]/(double)n)*j;
				pnt->point[1] += (v[1]/(double)n)*j;
				pnt->point[2] += (v[2]/(double)n)*j;
				j++;
				pnt = pnt->next;
			      }
			  }
			ay_selp_clear(o);
		      }
		    else
		      {
			/* have no section */
			vlen = AY_V3LEN(pnt->point);
			if(vlen < AY_EPSILON || vlen != vlen)
			  {
			    v[0] = 0.1;
			    for(i = 0; i < n; i++)
			      {
				pnt->point[0] = (v[0]/(double)n)*i;
				/*
				  pnt->point[1] = (v[1]/(double)n)*i;
				  pnt->point[2] = (v[2]/(double)n)*i;
				*/
				pnt = pnt->next;
			      }
			  }
			else
			  {
			    v[0] = vlen*0.1;
			    for(i = 0; i < n; i++)
			      {
				pnt->point[0] += (v[0]/(double)n)*i;
				/*
				  pnt->point[1] += (v[1]/(double)n)*i;
				  pnt->point[2] += (v[2]/(double)n)*i;
				*/
				pnt = pnt->next;
			      }
			  }
		      }

		    o->selp = opnt;
		    (void)ay_notify_object(o);
		    o->modified = AY_TRUE;
		    notify_parent = AY_TRUE;
		  }
	      }
	    else
	      ay_error(AY_EWARN, argv[0], "No editable points found!");
	} /* if pnt */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    {
      (void)ay_notify_parent();
    }

 return TCL_OK;
} /* ay_selp_explodetcmd */


/** ay_selp_managelist:
 * Compile array of points from a list of objects.
 *
 * \param[in] o list of objects
 * \param[in,out] lenptr where to store the compiled number of points
 * \param[in,out] pntsptr where to store the compiled points
 */
void
ay_selp_managelist(ay_object *o, unsigned int *lenptr, double **pntsptr)
{
 int ay_status = AY_OK;
 unsigned int pntslen = 0, j, a = 0;
 double *pnts = NULL, *p1, *p2, dummy[3], m[16];
 ay_pointedit pe = {0};

  if(!o || !lenptr || !pntsptr)
    return;

  while(o)
    {
      ay_status = ay_pact_getpoint(0, o, dummy, &pe);

      if(!ay_status && pe.num)
	{
	  pntslen += pe.num;
	  p1 = realloc(pnts, pntslen*4*sizeof(double));

	  if(p1)
	    {
	      pnts = p1;
	      if(AY_ISTRAFO(o))
		{
		  ay_trafo_creatematrix(o, m);

		  for(j = 0; j < pe.num; j++)
		    {
		      p1 = &(pnts[a]);
		      p2 = pe.coords[j];
		      AY_APTRAN3(p1, p2, m);
		      if(pe.type == AY_PTRAT)
			{
			  p1[3] = pe.coords[j][3];
			}
		      else
			{
			  p1[3] = 1.0;
			}
		      a += 4;
		    } /* for */
		}
	      else
		{
		  for(j = 0; j < pe.num; j++)
		    {
		      p1 = &(pnts[a]);
		      p2 = pe.coords[j];
		      memcpy(p1, p2, 3*sizeof(double));
		      if(pe.type == AY_PTRAT)
			{
			  p1[3] = pe.coords[j][3];
			}
		      else
			{
			  p1[3] = 1.0;
			}
		      a += 4;
		    } /* for */
		} /* if is trafo */
	    }
	  else
	    {
	      /* realloc() failed! */
	      ay_pact_clearpointedit(&pe);
	      if(pnts)
		free(pnts);
	      return;
	    } /* if */
	} /* if */

      ay_pact_clearpointedit(&pe);

      o = o->next;
    } /* while */

  /* return results */
  *lenptr = pntslen;
  *pntsptr = pnts;

 return;
} /* ay_selp_managelist */


/** ay_selp_reducetominmax:
 * Change the point selection so that only the two points with the minimum
 * and maximum index remain selected.
 *
 * \param[in,out] o object to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_selp_reducetominmax(ay_object *o)
{
 unsigned int min = UINT_MAX, max = 0;
 ay_point *selp, *pmin = NULL, *pmax = NULL;
 ay_point *next = NULL, *p = NULL;

  if(!o)
    return AY_ENULL;

  selp = o->selp;

  if(!selp || !selp->next)
    return AY_OK;

  while(selp)
    {
      if(min > selp->index)
	{
	  min = selp->index;
	  pmin = selp;
	}
      if(max < selp->index)
	{
	  max = selp->index;
	  pmax = selp;
	}
      selp = selp->next;
    }

  if(min == max)
    return AY_OK;

  p = o->selp;
  while(p)
    {
      next = p->next;
      if(p != pmin && p != pmax)
	free(p);
      p = next;
    }

  if(pmin && pmax)
    {
      o->selp = pmin;
      pmin->next = pmax;
      pmax->next = NULL;
    }

 return AY_OK;
} /* ay_selp_reducetominmax */


/** ay_selp_updatempselection:
 * Update selection state of multiple points; if all points that
 * make up a multiple point are selected, the multiple point will
 * also be marked selected, else it will be marked unselected.
 *
 * \param[in] n total number of points in curve/surface
 * \param[in] selp selected points
 * \param[in,out] mp multiple points
 */
void
ay_selp_updatempselection(unsigned int n, ay_point *selp, ay_mpoint *mp)
{
 char *sel = NULL;
 int i, j;

  if(!(sel = calloc(n, sizeof(char))))
    return;

  while(selp)
    {
      if(selp->index < n)
	sel[selp->index] = 1;
      selp = selp->next;
    }

  while(mp)
    {
      j = 0;
      for(i = 0; i < mp->multiplicity; i++)
	{
	  if(mp->indices[i] < n)
	    if(sel[mp->indices[i]])
	      j++;
	}
      if(j == mp->multiplicity)
	{
	  mp->selected = AY_TRUE;
	}
      else
	{
	  mp->selected = AY_FALSE;
	}
      mp = mp->next;
    }

  free(sel);

 return;
} /* ay_selp_updatempselection */


/** ay_selp_find:
 * Check whether a given point is among a list of selected points.
 *
 * \param selp the list of selected points to search
 * \param point the address of the coordinates of the point
 *
 * \returns AY_TRUE if the point is among the list of selected points,
 *          AY_FALSE else
 */
int
ay_selp_find(ay_point *selp, double *point)
{
 ay_point *p = selp;

  while(p)
    {
      if(p->point == point)
	return AY_TRUE;
      p = p->next;
    }

 return AY_FALSE;
} /* ay_selp_find */
