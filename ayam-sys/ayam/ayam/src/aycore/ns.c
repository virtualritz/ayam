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
#ifndef AYWITHAQUA
#include <X11/Xutil.h>
#endif /* !AYWITHAQUA */

/* ns.c - Notify Script tag helpers */


/* prototypes of functions local to this module */

Tk_RestrictAction ay_ns_restrictall(ClientData clientData, XEvent *eventPtr);


/* functions: */

/* ay_ns_restrictall:
 *  This Tk callback is used to process all GUI events
 *  while script tags (and script object scripts) are
 *  evaluated. It mostly discards the events, except
 *  for the keypress to break scripts (<Ctrl+Shift+C>),
 *  which leads to the global Tcl variable "cancelled"
 *  being set to 1, which in turn (hopefully) breaks
 *  out of the script (while/for commands check for this
 *  variable).
 */
Tk_RestrictAction
ay_ns_restrictall(ClientData clientData,
		  XEvent *eventPtr)
{
 char cancelled[] = "cancelled";

#ifndef AYWITHAQUA
#ifndef WIN32
 XKeyEvent *key_event = (XKeyEvent *) eventPtr;
 KeySym ks;
 XComposeStatus status;
 char tmpstr[128];
#endif /* !WIN32 */

#ifdef WIN32
  if((GetKeyState(VK_SHIFT) < 0) &&
     (GetKeyState(VK_CONTROL) < 0) &&
     (GetKeyState('C') < 0))
    {
      Tcl_SetVar2Ex(ay_interp, cancelled, NULL, Tcl_NewIntObj(1),
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#ifndef AYNOSAFEINTERP
      Tcl_SetVar2Ex(ay_safeinterp, cancelled, NULL, Tcl_NewIntObj(1),
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif
      return TK_DISCARD_EVENT;
    }
#else
  if(eventPtr->type == KeyPress)
    {
      if(key_event->state & (ControlMask|ShiftMask))
	{
	  XLookupString(key_event, tmpstr, 128, &ks, &status);
	  if(ks == 0x43)
	    {
	      Tcl_SetVar2Ex(ay_interp, cancelled, NULL, Tcl_NewIntObj(1),
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#ifndef AYNOSAFEINTERP
	      Tcl_SetVar2Ex(ay_safeinterp, cancelled, NULL, Tcl_NewIntObj(1),
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif
	      return TK_DISCARD_EVENT;
	    }
	  else
	    {
	      return TK_DISCARD_EVENT;
	    } /* if */
	}
      else
	{
	  return TK_DISCARD_EVENT;
	} /* if */
    } /* if */
#endif /* WIN32 */
#endif /* !AYWITHAQUA */

 return TK_DEFER_EVENT;
} /* ay_ns_restrictall */


/** ay_ns_execute:
 * Execute a ANS/BNS tag.
 *
 * Even though \a o is listed here as in parameter, whether or not
 * \a o (or even other objects in the scene) will be changed is entirely
 * up to the script in the tag.
 *
 * \param[in] o object with ANS/BNS tag
 * \param[in] tag tag to execute
 *
 */
void
ay_ns_execute(ay_object *o, ay_tag *tag)
{
 static int lock = 0;
 int ay_status = AY_OK, result = TCL_OK;
 char fname[] = "ns_execute";
 ay_list_object *l = NULL, *old_selection = NULL;
 ay_list_object *old_currentlevel;
 ClientData old_restrictcd;
 Tcl_Interp *interp = NULL;
 char *script = NULL;

  if(!o || !tag)
    return;

  /* this lock protects ourselves from running in an endless
     recursive loop should the script modify our child objects */
  if(lock)
    {
      return;
    }
  else
    {
      lock = 1;
    } /* if */

  script = tag->val;

#ifdef AYNOSAFEINTERP
  interp = ay_interp;
#else
  interp = ay_safeinterp;
#endif

  old_currentlevel = ay_currentlevel;
  old_selection = ay_selection;
  ay_selection = NULL;

  if((ay_status = ay_sel_add(o, AY_FALSE)))
    goto cleanup;

  ay_currentlevel = NULL;
  if((ay_status = ay_clevel_add(NULL)))
    goto cleanup;
  if((ay_status = ay_clevel_add(ay_root)))
    goto cleanup;
  if((ay_status = ay_clevel_add(o)))
    goto cleanup;
  if((ay_status = ay_clevel_add(o->down)))
    goto cleanup;

  Tk_RestrictEvents(ay_ns_restrictall, NULL, &old_restrictcd);
  result = Tcl_GlobalEval(interp, script);
  Tk_RestrictEvents(NULL, NULL, &old_restrictcd);

  if(result == TCL_ERROR)
    {
      ay_error(AY_ERROR, fname, Tcl_GetStringResult(interp));
      ay_status = AY_OK;
      if(ay_prefs.disablefailedscripts)
	{
	  ay_status = ay_ns_disable(tag);
	}
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Script failed!");
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Script failed and disabled!");
	}
    }

cleanup:
  if(ay_status)
    ay_error(AY_ERROR, fname, "Error setting up the script environment!");

  /* restore current level */
  (void)ay_clevel_delall();
  if(ay_currentlevel)
    free(ay_currentlevel);
  ay_currentlevel = old_currentlevel;

  /* restore selection */
  while(ay_selection)
    {
      l = ay_selection->next;
      /*
      if(ay_selection->object)
	ay_selection->object->selected = AY_FALSE;
      */
      free(ay_selection);
      ay_selection = l;
    } /* while */

  ay_selection = old_selection;

  lock = 0;

 return;
} /* ay_ns_execute */


/** ay_ns_disable:
 * Disables a BNS/ANS tag by setting its type/name to DBNS/DANS respectively.
 *
 * \param[in,out] tag tag to disable
 *
 * \return AY_OK on success, error code otherwise
 */
int
ay_ns_disable(ay_tag *tag)
{
 char *tc;

  if(!tag)
    return AY_ENULL;

  if(tag->type == ay_ans_tagtype)
    {
      tc = realloc(tag->name, (strlen(ay_dans_tagname)+1)*sizeof(char));
      if(tc)
	{
	  tag->type = ay_dans_tagtype;
	  tag->name = tc;
	  strcpy(tag->name, ay_dans_tagname);
	}
      else
	{
	  return AY_EOMEM;
	}
    }

  if(tag->type == ay_bns_tagtype)
    {
      tc = realloc(tag->name, (strlen(ay_dbns_tagname)+1)*sizeof(char));
      if(tc)
	{
	  tag->type = ay_dbns_tagtype;
	  tag->name = tc;
	  strcpy(tag->name, ay_dbns_tagname);
	}
      else
	{
	  return AY_EOMEM;
	}
    }

 return AY_OK;
} /* ay_ns_disable */


/** ay_ns_ensable:
 * Enables a DBNS/DANS tag by setting its type/name to BNS/ANS respectively.
 *
 * \param[in,out] tag tag to enable
 *
 */
void
ay_ns_enable(ay_tag *tag)
{

  if(!tag)
    return;

  if(tag->type == ay_dans_tagtype)
    {
      tag->type = ay_ans_tagtype;
      memcpy(tag->name, ay_ans_tagname, 4*sizeof(char));
    }

  if(tag->type == ay_dbns_tagtype)
    {
      tag->type = ay_bns_tagtype;
      memcpy(tag->name, ay_bns_tagname, 4*sizeof(char));
    }

 return;
} /* ay_ns_enable */


/* ay_ns_init:
 *  initialize ns module by registering the ANS/BNS/DANS/DBNS tag types
 */
void
ay_ns_init(Tcl_Interp *interp)
{

  /* register NS tag types */
  (void)ay_tags_register(ay_bns_tagname, &ay_bns_tagtype);
  (void)ay_tags_register(ay_ans_tagname, &ay_ans_tagtype);

  (void)ay_tags_register(ay_dbns_tagname, &ay_dbns_tagtype);
  (void)ay_tags_register(ay_dans_tagname, &ay_dans_tagtype);

 return;
} /* ay_ns_init */
