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

/* object.c - general object management */

/* prototypes of functions local to this module: */

int ay_object_isclosed(ay_object *o, int argc, char *argv[]);

int ay_object_isplanar(ay_object *o);

int ay_object_isdegen(ay_object *o);

int ay_object_istrimmed(ay_object *o);

int ay_object_israt(ay_object *o);


/** ay_object_create:
 * Allocate a new #ay_object structure, fill it with vital default values,
 * and call the type specific create callback without parameters.
 *
 * \param[in] index designates the type of object to create (AY_ID*)
 * \param[in,out] o where to store the address of the new object
 *
 * \returns AY_OK on success, error code else.
 */
int
ay_object_create(unsigned int index, ay_object **o)
{
 int ay_status = AY_OK;
 char fname[] = "object_create";
 ay_voidfp *arr = NULL;
 ay_createcb *cb = NULL;
 ay_object *new = NULL;

  arr = ay_createcbt.arr;

  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_object_defaults(new);

  new->type = index;

  cb = (ay_createcb *)(arr[index]);

  ay_status = cb(0, NULL, new);

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      free(new);
      return AY_ERROR;
    }

  *o = new;

 return ay_status;
} /* ay_object_create */


/** ay_object_createargs:
 * Allocate a new #ay_object structure, fill it with vital default values,
 * and call the type specific create callback with user provided parameters.
 *
 * \param[in] index designates the type of object to create (AY_ID*)
 * \param[in] argc number of parameters (>= 0)
 * \param[in] argv array of parameters [argc] (may be NULL if \a argc is 0)
 * \param[in,out] o where to store the address of the new object
 *
 * \returns AY_OK on success, error code else.
 */
int
ay_object_createargs(unsigned int index, int argc, char **argv, ay_object **o)
{
 int ay_status = AY_OK;
 char fname[] = "object_createargs";
 ay_voidfp *arr = NULL;
 ay_createcb *cb = NULL;
 ay_object *new = NULL;

  arr = ay_createcbt.arr;

  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_object_defaults(new);

  new->type = index;

  cb = (ay_createcb *)(arr[index]);

  ay_status = cb(argc, argv, new);

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      free(new);
      return AY_ERROR;
    }

  *o = new;

 return ay_status;
} /* ay_object_createargs */


/** ay_object_createtcmd:
 *  Create a new object and link it to the scene hierarchy (as last
 *  object in the current level).
 *  Implements the \a crtOb scripting interface command.
 *  See also the corresponding section in the \ayd{sccrtob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_createtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 unsigned int index;
 Tcl_HashEntry *entry = NULL;
 Tcl_Interp *ointerp = ay_interp;
 ay_createcb *cb = NULL;
 ay_object *o = NULL;
 char *a = "ay", *n = "sc", *v = "1";

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "typename [args]");
      return TCL_OK;
    }

  entry = Tcl_FindHashEntry(&ay_otypesht, argv[1]);
  if(!entry)
    {
      ay_error(AY_ENTYPE, argv[0], argv[1]);
      return TCL_OK;
    }

  index = *((unsigned int *) Tcl_GetHashValue(entry));

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  o->type = index;
  ay_object_defaults(o);

  arr = ay_createcbt.arr;
  cb = (ay_createcb *)(arr[index]);
  if(cb)
    {
      /* this kludge allows create callbacks to use variables/data from
	 the safe interpreter when the crtOb command is e.g. called in
	 a Script object script */
      ay_interp = interp;
      ay_status = cb(argc, argv, o);
      ay_interp = ointerp;
    }

  if(ay_status)
    {
      ay_error(ay_status, argv[0], "Create callback failed!");
      free(o);
      return TCL_OK;
    }

  if(!o->parent && (interp != ay_safeinterp))
    {
      ay_object_placemark(o);
    }

  /* for potential parent objects, create endlevel terminator */
  if(o->parent && (!o->down))
    {
      o->down = ay_endlevel;
    }

  /* link object to scene structure */
  ay_object_link(o);

  /* set scene changed flag */
  Tcl_SetVar2(interp, a, n, v, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return TCL_OK;
} /* ay_object_createtcmd */


/** ay_object_delete:
 *  _recursively_ delete the children of an object and then the object;
 *  fails if there are objects/children with refcount > 0;
 *  does not unlink the object!
 *
 * \param[in,out] o the object to be deleted
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_object_delete(ay_object *o)
{
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_deletecb *cb = NULL;
 ay_object *d = NULL;
 ay_mat_object *mat = NULL;
 unsigned int *refcountptr;
 ay_tag *tag = NULL;

  if(!o)
    return AY_ENULL;

  /* never delete objects with a reference count > 0 */
  if(o->refcount > 0)
    return AY_EREF;

  /* never delete the one and only end level object */
  if(o == ay_endlevel)
    return AY_OK;

  /* delete children first */
  while(o->down && (o->down != ay_endlevel))
    {
      d = o->down;
      /* unlink the first child and try to delete it */
      o->down = d->next;
      ay_status = ay_object_delete(d);
      if(ay_status)
	{
	  /* delete failed, re-link the child */
	  o->down = d;
	  return ay_status;
	} /* if */
    } /* while have children */

  if(o->refine)
    {
      arr = ay_deletecbt.arr;
      cb = (ay_deletecb *)(arr[o->type]);
      if(cb)
	{
	  ay_status = cb(o->refine);
	  if(ay_status)
	    {
	      return ay_status;
	    }
	}
    }

  /* delete selected points */
  if(o->selp)
    {
      ay_selp_clear(o);
    }

  /* see if other objects point to us via NO tags; remove those tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_nm_tagtype && tag->is_binary)
	{
	  d = (ay_object*)(((ay_btval*)tag->val)->payload);
	  tag = tag->next;
	  ay_tags_remnonm(o, d);
	}
      else
	{
	  tag = tag->next;
	}
    } /* while tag */

  /* delete tags */
  if(o->tags)
    {
      ay_tags_delall(o);
    }

  /* remove reference to material */
  if(o->mat)
    {
      mat = o->mat;
      refcountptr = mat->refcountptr;
      (*refcountptr)--;
    }

  /* free name */
  if(o->name)
    {
      free(o->name);
      o->name = NULL;
    }

  /* finally, delete the object */
  free(o);

 return AY_OK;
} /* ay_object_delete */


/** ay_object_deletemulti:
 *  delete multiple objects connected via their ->next fields
 *
 *  If the \a force argument is AY_FALSE (0) and the hierarchy contains
 *  undeletable objects (e.g. due to references), the removal process
 *  will be stopped and the object hierarchy will be in unknown state.
 *  This variant should only be used, when there are no referenced
 *  objects in \a o, otherwise memory may leak.
 *
 *  If the \a force argument is AY_TRUE (1), the removal of master objects
 *  will be enforced by setting the reference counts to 0 before deletion.
 *  Furthermore, all internal references will be removed beforehand via
 *  instt_removeinstances() and matt_removeallrefs().
 *  This variant should only be used after a successful call to
 *  object_candelete(),
 *  otherwise access to freed memory/crashes can occur later (via the
 *  references)!
 *
 *  If the \a force argument is 2, the removal of master objects will be
 *  enforced by setting the reference counts to 0 before deletion.
 *  There will be _no_ calls to instt_removeinstances() or
 *  matt_removeallrefs()!
 *  The object hierarchy should either have no references and no objects
 *  with materials (i.e. already have been cleaned by instt_removeinstances()
 *  and matt_removeallrefs()) or the objects must be sorted in a way that
 *  all references are deleted before the respective master/material objects.
 *  Otherwise crashes can occur while removing (use after free()).
 *  This variant should only be used after a successful call to
 *  object_candelete(),
 *  otherwise access to freed memory/crashes can occur later (via the
 *  references)!
 *
 *  It is safe to call this function with hierarchies that have no
 *  endlevel-terminator(s).
 *
 * \param[in,out] o the object(s) to be deleted
 * \param[in] force how to handle undeletable objects, see above (0, 1, 2)
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_object_deletemulti(ay_object *o, int force)
{
 int ay_status = AY_OK;
 ay_object *next = NULL, *d = NULL;

  if(!o)
    return AY_ENULL;

  d = o;

  if(force == AY_TRUE)
    {
      ay_instt_removeinstances(&(d), NULL);
      ay_matt_removeallrefs(d);
      force = 2;
    }

  while(d)
    {
      if(force)
	{
	  if(d->down && d->down != ay_endlevel)
	    {
	      (void)ay_object_deletemulti(d->down, force);
	      d->down = NULL;
	    }
	  d->refcount = 0;
	}

      next = d->next;

      ay_status = ay_object_delete(d);
      if(!force && ay_status)
	{
	  return ay_status;
	}
      d = next;
    } /* while */

 return AY_OK;
} /* ay_object_deletemulti */


/** ay_object_deletetcmd:
 *  Delete selected objects.
 *  Implements the \a delOb scripting interface command.
 *  Also implements the \a candelOb scripting interface command.
 *  See also the corresponding section in the \ayd{scdelob}.
 *  See also the corresponding section in the \ayd{sccandelob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_deletetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL, *oldnext;
 ay_list_object *sel, *try_again = NULL, **next_try_again, *t;
 ay_mat_object *m = NULL;

  if(!ay_selection)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argv[0][0] == 'c')
    {
      /* is candelOb */
      if((ay_selection->object != ay_root) &&
	 (ay_object_candeletelist(ay_selection, NULL) == AY_OK))
	{
	  Tcl_SetResult(interp, "1", TCL_VOLATILE);
	}
      else
	{
	  Tcl_SetResult(interp, "0", TCL_VOLATILE);
	}
      return TCL_OK;
    }

  /* remove all references of the objects to be removed
     in order to avoid illegal memory accesses; should e.g.
     a material be removed first and later an object of that
     material decreases the reference counter of the removed
     material object while being deleted, an illegal memory
     access ensues */
  sel = ay_selection;
  while(sel)
    {
      o = sel->object;
      if(o->down && o->down->next)
	{
	  ay_instt_removeinstances(&(o->down), NULL);
	  ay_matt_removeallrefs(o->down);
	}
      if(o->mat)
	{
	  m = o->mat;
	  o->mat = NULL;
	  if(m->refcountptr)
	    (*(m->refcountptr))--;
	}
      sel = sel->next;
    }

  /* check for referenced objects that can not be deleted */
  sel = ay_selection;
  if(ay_object_candeletelist(sel, NULL) != AY_OK)
    {
      while(sel)
	{
	  /* temporarily isolate the object so that clearclipboard()
	     does not browse through too much of the scene */
	  oldnext = sel->object->next;
	  sel->object->next = NULL;
	  ay_instt_clearclipboard(sel->object);
	  sel->object->next = oldnext;
	  sel = sel->next;
	}
      sel = ay_selection;
      if(ay_object_candeletelist(sel, NULL) != AY_OK)
	{
	  while(sel)
	    {
	      ay_instt_removeinstances(&(sel->object->down), NULL);
	      sel = sel->next;
	    }
	}
    }

  next_try_again = &(try_again);
  sel = ay_selection;
  while(sel)
    {
      o = sel->object;
      if(o == ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "Root object may not be deleted!");
	}
      else
	{
	  ay_object_unlink(o);

	  ay_undo_clearobj(o);

	  ay_status = ay_object_delete(o);
	  if(ay_status)
	    {
	      /* could not delete the object, probably due to
		 its reference counter being not zero; add the object
		 to the try_again list and continue deleting
		 objects in the hope that the user selected _all_
		 instances and we can delete this object anyway
		 in a second attempt */
	      if(!(*next_try_again = calloc(1, sizeof(ay_list_object))))
		{
		  ay_sel_free(AY_FALSE);
		  while(try_again)
		    {
		      t = try_again->next;
		      free(try_again);
		      try_again = t;
		    } /* while */
		  ay_error(AY_EOMEM, argv[0], NULL);
		  return TCL_OK;
		}
	      (*next_try_again)->object = o;
	      next_try_again = &((*next_try_again)->next);
	    } /* if delete failed */
	} /* if is ay_root */
      sel = sel->next;
    } /* while */

  /* free selection, we _must_ do it now, because otherwise
   * the ay_error() below can lead to the processing of Tcl
   * events and a redraw that includes iterating over the
   * list of selected objects that we just free()d => bummer! */
  ay_sel_free(AY_FALSE);

  while(try_again)
    {
      ay_status = ay_object_delete(try_again->object);
      if(ay_status)
	{
	  ay_error(ay_status, argv[0], NULL);
	  ay_object_link(try_again->object);
	}
      t = try_again->next;
      free(try_again);
      try_again = t;
    } /* while */

 return TCL_OK;
} /* ay_object_deletetcmd */


/** ay_object_link:
 *  link single object \a o to scene structure;
 *  uses \a ay_next;
 *  fixes o->down for parent objects with NULL as down;
 *  properly maintains \a ay_next and \a ay_currentlevel
 *
 * \param[in,out] o object to link
 */
void
ay_object_link(ay_object *o)
{

  if(ay_next)
    {
      o->next = *ay_next;

      *ay_next = o;

      ay_next = &(o->next);
   }

  if(o->parent && !o->down)
    {
      o->down = ay_endlevel;
    }

  /* just in case that we are linking the very first object to
     an empty sub-level (ay_currentlevel points to the end-level object)
     we need to correct ay_currentlevel to point to the new object instead */
  if(ay_currentlevel && ay_currentlevel->object != ay_root)
    {
      if(ay_currentlevel->next && ay_currentlevel->next->object)
	{
	  ay_clevel_set(ay_currentlevel->next->object->down);
	}
    }

 return;
} /* ay_object_link */


/** ay_object_unlink:
 *  unlink object \a o from scene, without deleting it!
 *  \a o must be in the current level;
 *  properly maintains \a ay_next and \a ay_currentlevel
 *
 * \param[in] o object to unlink
 */
void
ay_object_unlink(ay_object *o)
{
 int done;
 ay_list_object *clevel = ay_currentlevel;
 ay_object *clevelobj = NULL, *p1, *p2;

  clevelobj = clevel->object;

  /* unlink first object of current level? */
  if(clevelobj == o)
    { /* yes */
      p1 = clevel->next->object;
      p1->down = o->next;
      if(ay_next == &(o->next))
	{
	  /* repair ay_next */
	  ay_next = &(p1->down);
	}
      ay_clevel_set(o->next);
    }
  else
    { /* no */
      p1 = clevelobj;
      p2 = p1;
      if(p1)
	{
	  p1 = p1->next;
	}
      done = AY_FALSE;
      while(p1 && !done)
	{
	  if(p1 == o)
	    {
	      p2->next = o->next;
	      done = AY_TRUE;
	      if(ay_next == &(o->next))
		{
		  /* repair ay_next */
		  ay_next = &(p2->next);
		}
	    } /* if */
	  p2 = p1;
	  p1 = p1->next;
	} /* while */
    } /* if */

 return;
} /* ay_object_unlink */


/** ay_object_defaults:
 *  set object attributes of a freshly allocated object to safe
 *  default settings
 *
 * \param[in,out] o object to process
 */
void
ay_object_defaults(ay_object *o)
{
  if(!o)
    return;

  o->quat[3] = 1.0;

  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;

  o->inherit_trafos = AY_TRUE;

 return;
} /* ay_object_defaults */


/** ay_object_placemark:
 *  move an object to the mark (global mark or that of the current view)
 *
 * \param[in,out] o object to process
 */
void
ay_object_placemark(ay_object *o)
{
 ay_list_object tsel = {0}, *osel;

  if(!o)
    return;

  /* move object to the mark? */
  if(ay_prefs.createatmark && ay_currentview && ay_currentview->drawmark)
    {
      /* fake single object selection for snaptomarkcb() */
      osel = ay_selection;
      tsel.object = o;
      ay_selection = &tsel;

      ay_pact_snaptomarkcb(ay_currentview->togl, -1, NULL);
      ay_selection = osel;
    }

 return;
} /* ay_object_placemark */


/** ay_object_gettypename:
 *  return type name that has been registered for
 *  a given object type
 *
 * \param[in] index object type to lookup (AY_ID*)
 *
 * \returns object type name or \a NULL
 */
char *
ay_object_gettypename(unsigned int index)
{
 void **arr = ay_typenamest.arr;
 return((char*)arr[index]);
} /* ay_object_gettypename */


/** ay_object_getname:
 *  return object name or (if object is unnamed) name that has
 *  been registered for the type of the object
 *
 * \param[in] o object to inquire
 *
 * \returns object name, object type name or \a NULL
 */
char *
ay_object_getname(ay_object *o)
{
 char *name = NULL;

  if(o->type == AY_IDINSTANCE)
    {
      return ay_object_getname((ay_object *)(o->refine));
    }

  if(o->name)
    {
      name = o->name;
    }
  else
    {
      name = ay_object_gettypename(o->type);
    }

 return name;
} /* ay_object_getname */


/** ay_object_setnametcmd:
 *  Set the name of an object.
 *  Implements the \a nameOb scripting interface command.
 *  See also the corresponding section in the \ayd{scnameob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_setnametcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "name");
      return TCL_OK;
    }

  o = sel->object;

  if((o == ay_root) || (o->type == AY_IDVIEW) || (o->type == AY_IDMATERIAL))
    {
      return TCL_OK;
    }

  if(o)
    {
      if(o->name)
	{
	  free(o->name);
	  o->name = NULL;
	}

      /* clear the current name if argument is "" */
      if(strlen(argv[1]) == 0)
	{
	  return TCL_OK;
	}

      if(!(o->name = malloc(strlen((argv[1])+1) * sizeof(char))))
	{
	  ay_error(AY_EOMEM, argv[0], NULL);
	  return TCL_OK;
	}

      strcpy(o->name, argv[1]);
    } /* if o */

 return TCL_OK;
} /* ay_object_setnametcmd */


/* ay_object_copy:
 *  copy object src to dst
 *  this is a deep copy!
 *  tags, material, attributes, and transformations are copied as well
 */
int
ay_object_copy(ay_object *src, ay_object **dst)
{
 int ay_status = AY_OK;
 char fname[] = "object_copy";
 ay_object *sub = NULL, *new = NULL;
 ay_object **next = NULL;
 ay_voidfp *arr = NULL;
 ay_copycb *cb = NULL;
 ay_mat_object *mat = NULL;
 unsigned int *refcountptr;

  if(!src || !dst)
    return AY_ENULL;

  /* silently avoid to really copy the endlevel terminator object;
     instead, just copy the reference */
  if(src == ay_endlevel)
    {
      *dst = ay_endlevel;
      return AY_OK;
    }

  /* copy generic object */
  if(!(new = malloc(sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  memcpy(new, src, sizeof(ay_object));
  /* danger! links point to original hierarchy */

  new->next = NULL;
  if(src->down != ay_endlevel)
    {
      new->down = NULL;
    }
  new->selp = NULL;
  new->tags = NULL;

  new->refcount = 0;

  /* copy type specific part */
  arr = ay_copycbt.arr;
  cb = (ay_copycb*)(arr[src->type]);
  if(cb)
    {
      ay_status = cb(src->refine, &(new->refine));

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "copy callback failed");
	  free(new);
	  return AY_ERROR;
	}
    }

  /* copy name */
  if(src->name)
    {
      if(!(new->name = malloc((strlen(src->name)+1) * sizeof(char))))
	{
	  free(new);
	  return AY_EOMEM;
	}

      strcpy(new->name, src->name);
    }

  /* copy tags */
  ay_status = ay_tags_copyall(src, new);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "copy tags failed");
      if(new->name)
	{
	  free(new->name);
	}
      free(new);
      return AY_ERROR;
    }

  /* increase material objects refcount */
  if(new->mat)
    {
      mat = new->mat;
      refcountptr = mat->refcountptr;
      (*refcountptr)++;
    }

  /* copy children */
  if(src->down && (src->down != ay_endlevel))
    {
      sub = src->down;
      next = &(new->down);
      while(sub)
	{
	  if((ay_status = ay_object_copy(sub, next)))
	    {
	      ay_object_delete(new);
	      return ay_status;
	    }
	  next = &((*next)->next);

	  sub = sub->next;
	} /* while */
    } /* if have children */

  new->modified = AY_TRUE;
  new->selected = AY_FALSE;

  *dst = new;

 return AY_OK;
} /* ay_object_copy */


/* ay_object_copymulti:
 *  copy multiple objects (linked via ->next) from src to dst
 */
int
ay_object_copymulti(ay_object *src, ay_object **dst)
{
 int ay_status = AY_OK;

  if(!src || !dst)
    {
      return AY_ENULL;
    }

  while(src)
    {
      ay_status = ay_object_copy(src, dst);
      if(ay_status || !(*dst))
	{
	  return ay_status;
	}
      else
	{
	  dst = &((*dst)->next);
	}
      src = src->next;
    } /* while */

 return AY_OK;
} /* ay_object_copymulti */


/** ay_object_isclosed:
 * Helper for ay_object_ishastcmd() below. Check object for closedness.
 *
 * \param[in] o object to check
 * \param[in] argc number of parameters in \a argv
 * \param[in] argv parameters
 *
 * \returns AY_TRUE if object is a closed parametric curve or surface;
 *          AY_FALSE else
 */
int
ay_object_isclosed(ay_object *o, int argc, char *argv[])
{
 ay_object *p;
 ay_nurbcurve_object *nc;
 ay_nurbpatch_object *np;
 int closed = AY_FALSE;

  if(o->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object*)o->refine;
      if(ay_nct_isclosed(nc))
	closed = AY_TRUE;
    }
  else
    {
      if(o->type == AY_IDNPATCH)
	{
	  np = (ay_nurbpatch_object*)o->refine;
	  if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'v')
	    {
	      if(ay_npt_isclosedv(np))
		closed = AY_TRUE;
	    }
	  else
	    {
	      if(ay_npt_isclosedu(np))
		closed = AY_TRUE;
	    }
	}
      else
	{
	  p = NULL;
	  (void)ay_provide_object(o, AY_IDNCURVE, &p);
	  if(p)
	    {
	      nc = (ay_nurbcurve_object*)p->refine;
	      if(ay_nct_isclosed(nc))
		closed = AY_TRUE;
	      ay_object_deletemulti(p, AY_FALSE);
	    }
	  else
	    {
	      (void)ay_provide_object(o, AY_IDNPATCH, &p);
	      if(p)
		{
		  np = (ay_nurbpatch_object*)p->refine;
		  if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'v')
		    {
		      if(ay_npt_isclosedv(np))
			closed = AY_TRUE;
		    }
		  else
		    {
		      if(ay_npt_isclosedu(np))
			closed = AY_TRUE;
		    }
		  ay_object_deletemulti(p, AY_FALSE);
		}
	    }
	} /* if not NPatch */
    } /* if not NCurve */

 return closed;
} /* ay_object_isclosed */


/** ay_object_isplanar:
 * Helper for ay_object_ishastcmd() below. Check object for planarity.
 *
 * \param[in] o object to check
 *
 * \returns AY_TRUE if object is a planar parametric curve or surface;
 *          AY_FALSE else
 */
int
ay_object_isplanar(ay_object *o)
{
 ay_object *p;
 ay_nurbpatch_object *np;
 int planar = AY_FALSE;

  if((o->type == AY_IDNCURVE) ||
     (o->type == AY_IDICURVE) ||
     (o->type == AY_IDACURVE))
    {
      ay_nct_isplanar(o, AY_FALSE, NULL, &planar);
    }
  else
    if(o->type == AY_IDNPATCH)
      {
	np = (ay_nurbpatch_object*)o->refine;
	if(ay_npt_isplanar(np, NULL))
	  planar = AY_TRUE;
      }
    else
      {
	p = NULL;
	(void)ay_provide_object(o, AY_IDNCURVE, &p);
	if(p)
	  {
	    ay_nct_isplanar(p, AY_FALSE, NULL, &planar);
	    ay_object_deletemulti(p, AY_FALSE);
	  }
	else
	  {
	    (void)ay_provide_object(o, AY_IDNPATCH, &p);
	    if(p)
	      {
		np = (ay_nurbpatch_object*)p->refine;
		if(ay_npt_isplanar(np, NULL))
		  planar = AY_TRUE;
		ay_object_deletemulti(p, AY_FALSE);
	      }
	  }
      } /* if not NCurve or NPatch */

 return planar;
} /* ay_object_isplanar */


/** ay_object_isdegen:
 * Helper for ay_object_ishastcmd() below. Check object for degeneracy.
 *
 * \param[in] o object to check
 *
 * \returns AY_TRUE if object is a degenerate parametric curve or surface;
 *          AY_FALSE else
 */
int
ay_object_isdegen(ay_object *o)
{
 ay_object *p;
 ay_nurbcurve_object *nc;
 ay_nurbpatch_object *np;
 int deg = AY_FALSE;

  if(o->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object*)o->refine;
      if(ay_nct_isdegen(nc))
	deg = AY_TRUE;
    }
  else
    if(o->type == AY_IDNPATCH)
      {
	np = (ay_nurbpatch_object*)o->refine;
	if(ay_npt_isdegen(np))
	  deg = AY_TRUE;
      }
    else
      {
	p = NULL;
	(void)ay_provide_object(o, AY_IDNCURVE, &p);
	if(p)
	  {
	    nc = (ay_nurbcurve_object*)p->refine;
	    if(ay_nct_isdegen(nc))
	      deg = AY_TRUE;
	    ay_object_deletemulti(p, AY_FALSE);
	  }
	else
	  {
	    (void)ay_provide_object(o, AY_IDNPATCH, &p);
	    if(p)
	      {
		np = (ay_nurbpatch_object*)p->refine;
		if(ay_npt_isdegen(np))
		  deg = AY_TRUE;
		ay_object_deletemulti(p, AY_FALSE);
	      }
	  }
      } /* if not NCurve or NPatch */

 return deg;
} /* ay_object_isdegen */


/** ay_object_israt:
 * Helper for ay_object_ishastcmd() below. Check object for weights.
 *
 * \param[in] o object to check
 *
 * \returns AY_TRUE if object is a parametric curve or surface with
 *          at least one weight value != 1.0;
 *          AY_FALSE else
 */
int
ay_object_israt(ay_object *o)
{
 ay_object *p;
 ay_pointedit pe = {0};
 ay_nurbcurve_object *nc;
 ay_nurbpatch_object *np;
 ay_pamesh_object *pm;
 int rat = AY_FALSE;
 unsigned int i;
 double pnt[3] = {0};

  switch(o->type)
    {
    case AY_IDNCURVE:
      nc = (ay_nurbcurve_object*)o->refine;
      if(ay_nct_israt(nc))
	rat = AY_TRUE;
      break;
    case AY_IDNPATCH:
      np = (ay_nurbpatch_object*)o->refine;
      if(ay_npt_israt(np))
	rat = AY_TRUE;
      break;
    case AY_IDPAMESH:
      pm = (ay_pamesh_object*)o->refine;
      if(ay_pmt_israt(pm))
	rat = AY_TRUE;
      break;
    case AY_IDICURVE:
    case AY_IDACURVE:
    case AY_IDBPATCH:
    case AY_IDIPATCH:
    case AY_IDAPATCH:
    case AY_IDSDMESH:
    case AY_IDPOMESH:
      break;
    default:
      p = NULL;
      (void)ay_provide_object(o, AY_IDNCURVE, &p);
      if(p)
	{
	  nc = (ay_nurbcurve_object*)p->refine;
	  if(ay_nct_israt(nc))
	    rat = AY_TRUE;
	  ay_object_deletemulti(p, AY_FALSE);
	}
      else
	{
	  (void)ay_provide_object(o, AY_IDNPATCH, &p);
	  if(p)
	    {
	      np = (ay_nurbpatch_object*)p->refine;
	      if(ay_npt_israt(np))
		rat = AY_TRUE;
	      ay_object_deletemulti(p, AY_FALSE);
	    }
	  else
	    {
	      ay_pact_getpoint(0, o, pnt, &pe);
	      if(pe.type == AY_PTRAT)
		{
		  for(i = 0; i < pe.num; i++)
		    {
		      if(fabs(1.0-pe.coords[i][3]) > AY_EPSILON)
			{
			  rat = AY_TRUE;
			  break;
			}
		    } /* for */
		  ay_pact_clearpointedit(&pe);
		}
	    } /* if not provided NPatch */
	} /* if not provided NCurve */
    } /* switch */

 return rat;
} /* ay_object_israt */


/** ay_object_istrimmed:
 * Helper for ay_object_ishastcmd() below. Check object for trims.
 *
 * \param[in] o object to check
 *
 * \returns AY_TRUE if object is a non-trivially trimmed parametric surface;
 *          AY_FALSE else
 */
int
ay_object_istrimmed(ay_object *o)
{
 ay_object *p;
 int trimmed = AY_FALSE;

  if(o->type == AY_IDNPATCH)
    {
      if(ay_npt_istrimmed(o, 0))
	trimmed = AY_TRUE;
    }
  else
    {
      (void)ay_provide_object(o, AY_IDNPATCH, &p);
      if(p)
	{
	  if(ay_npt_istrimmed(p, 0))
	    trimmed = AY_TRUE;
	  ay_object_deletemulti(p, AY_FALSE);
	}
    } /* if not NPatch */

 return trimmed;
} /* ay_object_istrimmed */


/** ay_object_ishastcmd:
 *  Check whether an object has certain properties (i.e. has child objects).
 *  Implements the \a hasChild scripting interface command.
 *  Also implements the \a hasMat scripting interface command.
 *  Also implements the \a hasRefs scripting interface command.
 *  Also implements the \a hasTrafo scripting interface command.
 *  Also implements the \a isCurve scripting interface command.
 *  Also implements the \a isRational scripting interface command.
 *  Also implements the \a isSurface scripting interface command.
 *  Also implements the \a isDegen scripting interface command.
 *  Also implements the \a isPlanar scripting interface command.
 *  Also implements the \a isParent scripting interface command.
 *  Also implements the \a isTrimmed scripting interface command.
 *  See also the corresponding section in the \ayd{schaschild}.
 *
 *  \returns 1 if selected object has a regular (other than
 *  endlevel) child object
 */
int
ay_object_ishastcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 char *res = NULL, no[] = "0", yes[] = "1";

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      sel = sel->next;

      if(argv[0][0] == 'h')
	{
	  /* is has... */
	  switch(argv[0][3])
	    {
	    case 'C':
	      /* is hasChild */
	      if(o->down && o->down != ay_endlevel)
		res = yes;
	      else
		res = no;
	      break;
	    case 'M':
	      /* is hasMat */
	      if(o->mat)
		res = yes;
	      else
		res = no;
	      break;
	    case 'R':
	      /* is hasRefs */
	      if(o->refcount)
		res = yes;
	      else
		res = no;
	      break;
	    case 'T':
	      /* is hasTrafo */
	      if(argc > 1)
		{
		  if((argv[1][0] == '-') && (argv[1][1] == 'r'))
		    {
		      if((fabs(o->quat[0]) > AY_EPSILON) ||
			 (fabs(o->quat[1]) > AY_EPSILON) ||
			 (fabs(o->quat[2]) > AY_EPSILON) ||
			 (fabs(1.0 - o->quat[3]) > AY_EPSILON))
			res = yes;
		      else
			res = no;
		    }
		  if((argv[1][0] == '-') && (argv[1][1] == 's'))
		    {
		      if((fabs(1.0 - o->scalx) > AY_EPSILON) ||
			 (fabs(1.0 - o->scaly) > AY_EPSILON) ||
			 (fabs(1.0 - o->scalz) > AY_EPSILON))
			res = yes;
		      else
			res = no;
		    }
		  if((argv[1][0] == '-') && (argv[1][1] == 't'))
		    {
		      if((fabs(o->movx) > AY_EPSILON) ||
			 (fabs(o->movy) > AY_EPSILON) ||
			 (fabs(o->movz) > AY_EPSILON))
			res = yes;
		      else
			res = no;
		    }
		}
	      else
		{
		  if(AY_ISTRAFO(o))
		    res = yes;
		  else
		    res = no;
		}
	      break;
	    default:
	      /* break the loop */
	      sel = NULL;
	      break;
	    } /* switch */
	}
      else
	{
	  /* is is... */
	  switch(argv[0][2])
	    {
	    case 'C':
	      if(argv[0][3] == 'u')
		{
		  /* is isCurve */
		  if(o->type == AY_IDNCURVE)
		    res = yes;
		  else
		    {
		      if(ay_provide_object(o, AY_IDNCURVE, NULL) == AY_OK)
			res = yes;
		      else
			res = no;
		    }
		}
	      else
		{
		  /* is isClosed */
		  if(ay_object_isclosed(o, argc, argv))
		    res = yes;
		  else
		    res = no;
		}
	      break;
	    case 'D':
	      /* is isDegen */
	      if(ay_object_isdegen(o))
		res = yes;
	      else
		res = no;
	      break;
	    case 'P':
	      if(argv[0][3] == 'a')
		{
		  /* is isParent */
		  if(o->parent)
		    res = yes;
		  else
		    res = no;
		  break;
		}
	      else
		{
		  /* is isPlanar */
		  if(ay_object_isplanar(o))
		    res = yes;
		  else
		    res = no;
		} /* if */
	      break;
	    case 'R':
	      /* is isRational */
	      if(ay_object_israt(o))
		res = yes;
	      else
		res = no;
	      break;
	    case 'S':
	      /* is isSurface */
	      if(o->type == AY_IDNPATCH)
		res = yes;
	      else
		{
		  if(ay_provide_object(o, AY_IDNPATCH, NULL) == AY_OK)
		    res = yes;
		  else
		    res = no;
		}
	      break;
	    case 'T':
	      /* is isTrimmed */
	      if(ay_object_istrimmed(o))
		res = yes;
	      else
		res = no;
	      break;
	    default:
	      /* break the loop */
	      sel = NULL;
	      break;
	    } /* switch */
	} /* if has or is */

      if(res)
	{
	  if(ay_selection->next)
	    Tcl_AppendElement(interp, res);
	  else
	    Tcl_SetResult(interp, res, TCL_VOLATILE);
	}
    } /* while sel */

 return TCL_OK;
} /* ay_object_ishastcmd */


/** ay_object_gettypeornametcmd:
 *  Implements the \a getType scripting interface command.
 *  Also implements the \a getName scripting interface command.
 *  See also the corresponding section in the \ayd{scgettype}.
 *  See also the corresponding section in the \ayd{scgetname}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_gettypeornametcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 char *typeorname = NULL;
 int i = 1, return_result = AY_FALSE, silence = AY_FALSE;

  /* check args */
  if(argc > 1)
    {
      if(argv[1][0] == '-' && argv[1][1] == 's')
	{
	  silence = AY_TRUE;
	  i++;
	}
    }

  if(argc <= i)
    {
      return_result = AY_TRUE;
    }

  if(!return_result)
    {
      Tcl_SetVar(interp, argv[i], "", TCL_LEAVE_ERR_MSG);
    }

  if(!sel)
    {
      if(!silence)
	ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      if(!o)
	{
	  ay_error(AY_ENULL, argv[0], NULL);
	  return TCL_OK;
	}

      if(argv[0][3] == 'N')
	{
	  typeorname = o->name;
	}
      else
	{
	  typeorname = ay_object_gettypename(o->type);
	}

      if(typeorname)
	{
	  if(return_result)
	    {
	      if(ay_selection->next)
		Tcl_AppendElement(interp, typeorname);
	      else
		Tcl_SetResult(interp, typeorname, TCL_VOLATILE);
	    }
	  else
	    {
	      Tcl_SetVar(interp, argv[i], typeorname, TCL_APPEND_VALUE |
			 TCL_LIST_ELEMENT);
	    }
	}
      else
	{
	  if(argv[0][3] == 'N')
	    {
	      if(!silence)
		ay_error(AY_EWARN, argv[0], "Object has no name.");
	    }
	  else
	    {
	      ay_error(AY_ENULL, argv[0], NULL);
	      return TCL_OK;
	    }
	}
      sel = sel->next;
    } /* while sel */

 return TCL_OK;
} /* ay_object_gettypeornametcmd */


/** ay_object_crtendlevel:
 * Stores a pointer to the end level object (ay_endlevel) in the designated
 * struct slot, usually o.next or o.down, where o is of type ay_object.
 *
 * This function used to create an entirely new ay_object _and_
 * ay_level_object. It is left here to provide API backwards compatibility
 * only, as storing a pointer should not require a function call.
 *
 * \param[in,out] o where to store the pointer to the end level object
 *
 * \returns AY_OK on success, error code else.
 */
int
ay_object_crtendlevel(ay_object **o)
{
  if(!o)
    return AY_ENULL;
  *o = ay_endlevel;
 return AY_OK;
} /* ay_object_crtendlevel */


/** ay_object_replace:
 * Overwrite object \a dst with the one pointed to by \a src.
 * Type specific contents of \a dst are lost afterwards;
 * object \a src should not be accessed via \a src after successful
 * completion because it is free()d here!
 *
 * The children of \a dst will be deleted, if this can not be done
 * due to references, they will be prepended to the object clipboard.
 * This will be reported to the user!
 *
 * \param[in,out] src source object
 * \param[in,out] dst destination object
 *
 * \returns AY_OK on success, error code else.
 */
int
ay_object_replace(ay_object *src, ay_object *dst)
{
 int ay_status = AY_OK;
 char fname[] = "replace";
 ay_voidfp *arr = NULL;
 ay_deletecb *dcb = NULL;
 ay_mat_object *oldmat = NULL;
 ay_object *oldnext = NULL, *d = NULL;
 ay_tag *tag = NULL;
 int oldrefcount = 0;

  if(!src || !dst)
    return AY_ENULL;

  oldmat = dst->mat;
  oldrefcount = dst->refcount;
  oldnext = dst->next;

  if(dst->down && dst->down->next)
    {
      ay_status = ay_object_candelete(dst->down, dst->down);

      if(ay_status != AY_OK)
	{
	  ay_clipb_prepend(dst->down, fname);
	}
      else
	{
	  d = dst->down;
	  while(d)
	    {
	      ay_undo_clearobj(d);
	      d = d->next;
	    }
	  (void)ay_object_deletemulti(dst->down, /*force=*/AY_TRUE);
	}
      dst->down = ay_endlevel;
    }

  if(dst->tags)
    {
      /* see if other objects point to dst via NO tags; remove those tags */
      tag = dst->tags;
      while(tag)
	{
	  if(tag->type == ay_nm_tagtype && tag->is_binary)
	    {
	      d = (ay_object*)(((ay_btval*)tag->val)->payload);
	      tag = tag->next;
	      ay_tags_remnonm(dst, d);
	    }
	  else
	    {
	      tag = tag->next;
	    }
	} /* while */

      /* delete all tags */
      ay_tags_delall(dst);
    }

  if(dst->name)
    {
      free(dst->name);
      dst->name = NULL;
    }

  if(dst->selp)
    {
      ay_selp_clear(dst);
    }

  arr = ay_deletecbt.arr;
  dcb = (ay_deletecb*)(arr[dst->type]);
  if(dcb)
    {
      ay_status = dcb(dst->refine);

      if(ay_status)
	return ay_status;
    }

  memcpy(dst, src, sizeof(ay_object));

  if(oldmat)
    {
      if(ay_matt_mayhavematerial(src->type) && !src->mat)
	{
	  dst->mat = oldmat;
	}
      else
	{
	  (*(oldmat->refcountptr))--;
	}
    }

  dst->refcount = oldrefcount;
  dst->next = oldnext;

  free(src);

 return AY_OK;
} /* ay_object_replace */


/** ay_object_count:
 * _Recursively_ counts all objects pointed to by \a o (including all
 * siblings and all children of \a o and its siblings).
 * The terminating endlevel objects must be present but are not included.
 *
 * \param[in] o an object in the scene hierarchy
 *
 * \returns the number of of objects besides and beneath \a o
 */
unsigned int
ay_object_count(ay_object *o)
{
 unsigned int lcount = 0;

  if(!o)
    return 0;

  while(o->next)
    {
      if(o->down && o->down->next)
	lcount += ay_object_count(o->down);
      lcount++;
      o = o->next;
    }

 return lcount;
} /* ay_object_count */


/** ay_object_candelete:
 * _Recursively_ check, whether there are referenced/master objects
 * in the hierarchy pointed to by \a o and whether any references/instances
 * of those masters are _not_ in the hierarchy pointed to by \a h.
 * If this is the case, the objects pointed to by \a o must not be deleted.
 *
 * Note: this function checks multiple objects and should therefore
 * rather be called ay_object_candeletemulti()
 *
 * Common usage is ay_object_candelete(o->down, o->down); to see whether
 * the children of o are "self contained" and can consequently be deleted.
 *
 * \param[in] h an object in the scene hierarchy
 * \param[in] o an object in the scene hierarchy
 *
 * \returns AY_OK if the objects pointed to by o can be deleted safely
 */
int
ay_object_candelete(ay_object *h, ay_object *o)
{
 int ay_status = AY_OK;
 unsigned int refs;

  if(!h || !o)
    return AY_ENULL;

  while(o && o->next)
    {
      if(o->down && o->down->next)
	{
	  /* recurse into children */
	  ay_status = ay_object_candelete(h, o->down);

	  /* immediately return a negative result */
	  if(ay_status)
	    return ay_status;
	}

      if(o->refcount)
	{
	  refs = 0;
	  ay_instt_countrefs(h, o, &refs);
	  if(o->refcount > refs)
	    return AY_ERROR;
	}
      o = o->next;
    } /* while */

 return ay_status;
} /* ay_object_candelete */


/** ay_object_candeletelist:
 * _Recursively_ check, whether there are referenced/master objects
 * in the list/hierarchy pointed to by \a l and whether any
 * references/instances of those masters are _not_ in the hierarchy
 * pointed to by \a l.
 * If this is the case, \a l must not be deleted.
 * For the initial invocation \a o must be NULL.
 *
 * \param[in] l a list of objects in the scene hierarchy
 * \param[in] o must be NULL
 *
 * \returns AY_OK if the objects pointed to by l can be deleted safely
 */
int
ay_object_candeletelist(ay_list_object *l, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *m;
 unsigned int refs;

  if(!l)
    return AY_ENULL;

  if(o)
    {
      /* worker invocation, check all objects pointed to by o */
      while(o && o->next)
	{
	  if(o->down && o->down->next)
	    {
	      /* recurse into children */
	      ay_status = ay_object_candeletelist(l, o->down);

	      /* immediately return a negative result */
	      if(ay_status)
		return ay_status;
	    }

	  if(o->refcount)
	    {
	      refs = 0;
	      ay_instt_countrefslist(l, o, &refs);
	      if(o->refcount > refs)
		return AY_ERROR;
	    }

	  o = o->next;
	} /* while */
    }
  else
    {
      /* top level invocation, just iterate the list, check
	 the objects directly pointed to by it, and if those
	 objects have children, fire off a recursive worker */
      while(l)
	{
	  m = l->object;

	  if(m->refcount)
	    {
	      refs = 0;
	      ay_instt_countrefslist(l, m, &refs);
	      if(m->refcount > refs)
		return AY_ERROR;
	    }

	  if(m->down && m->down->next)
	    {
	      ay_status = ay_object_candeletelist(l, m->down);

	      /* immediately return a negative result */
	      if(ay_status)
		return ay_status;
	    }

	  l = l->next;
	} /* while */
    } /* if */

 return ay_status;
} /* ay_object_candeletelist */


/** ay_object_getpathname:
 * _Recursively_ build up the full path name of an object in the scene
 * hierarchy.
 *
 * \param[in] o object to search for
 * \param[in] h hierarchy where to search for \a o (usually ay_root)
 * \param[in,out] totallen helper variable, should be initialized with 0
 * \param[in,out] found indicates wether \a o was found in \a h
 * \param[in,out] result where to store the result
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_object_getpathname(ay_object *o, ay_object *h, size_t *totallen, int *found,
		      char **result)
{
 int ay_status = AY_OK;
 size_t curlen, curtotallen;
 char *curname;

  while(h->next)
    {
      curname = ay_object_getname(h);
      curlen = strlen(curname);
      curtotallen = *totallen;

      /* +1 for the final NULL terminator or
	 the separator for any intermediate level */
      *totallen += curlen+1;

      if(h != o)
	{
	  if(h->down && h->down->next)
	    {
	      /* go down */
	      ay_status = ay_object_getpathname(h->down, o, totallen,
						found, result);
	      if(ay_status)
		break;
	      if(*found)
		{
		  /* prepend current (level) name */
		  memcpy(&((*result)[curtotallen]), curname, curlen);
		  (*result)[curtotallen+curlen] = ':';
		  return AY_OK;
		}
	    }
	}
      else
	{
	  /* found the object */

	  /* allocate memory for result */
	  if(!(*result = malloc(*totallen*sizeof(char))))
	     return AY_EOMEM;

	  /* save current (object) name to end of result */
	  strcpy(&((*result)[*totallen-curlen-1]), curname);
	  *found = AY_TRUE;
	  return AY_OK;
	} /* if */

      *totallen -= curlen+1;

      h = h->next;
    } /* while */

 return ay_status;
} /* ay_object_getpathname */


/** ay_object_find:
 *  _Recursively_ search through all objects beneath and below \a h
 *  for object \a o.
 *
 * \param[in] o object to search for
 * \param[in] h hierarchy where to search for \a o (usually ay_root)
 *
 * \returns AY_TRUE if the object was found, AY_FALSE else
 */
int
ay_object_find(ay_object *o, ay_object *h)
{
 int found = AY_FALSE;

  if(!h || !o)
    return AY_FALSE;

  while(h->next)
    {
      if(h != o)
	{
	  if(h->down && (h->down != ay_endlevel))
	    {
	      found = ay_object_find(o, h->down);
	      if(found)
		return AY_TRUE;
	    }
	}
      else
	{
	  return AY_TRUE;
	} /* if */
      h = h->next;
    } /* while */

 return AY_FALSE;
} /* ay_object_find */

