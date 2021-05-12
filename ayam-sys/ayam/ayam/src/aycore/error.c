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

/* error.c - functions for writing out error messages */

char *ay_error_igntype = "Ignoring object of unsupported type.";

static char aye_gentxterr[] = "Error! ", aye_gentxtwarn[] = "Warning! ";
static char aye_errcmd[] = "after idle {puts stderr {";
static char aye_warncmd[] = "after idle {puts stdout {";
static char aye_lmsg1[] = "Last message repeated ", aye_lmsg2[] = " times.";


/* prototypes of functions local to this module: */
void ay_error_wlog(const char *message);

/* functions: */

/* ay_error_wlog:
 *
 */
void
ay_error_wlog(const char *message)
{
 FILE *log = NULL;
 static int warned = AY_FALSE;
 int i;
 const char *m = NULL;

  if(!message)
    return;

  if(ay_prefs.writelog)
    {
      if(ay_prefs.logfile)
	{
	  if(!(log = fopen(ay_prefs.logfile, "ab")))
	    {
	      if(!warned)
		{
		  fprintf(stderr,
			"Ayam: Unable to open logfile: \"%s\" for writing!\n",
			  ay_prefs.logfile);
		  warned = AY_TRUE;
		} /* if */
	      return;
	    } /* if */
	} /* if */

      m = message;
      for(i = 0; i < 2; i++)
	{
	  m = strchr(m, '{');
	  if(!m)
	    break;
	  m++;
	}

      if(m && *m)
	{
	  fprintf(log, "%s\n", m);
	}
      else
	{
	  fprintf(log, "%s\n", message);
	} /* if */

      fclose(log);
    } /* if */

 return;
} /* ay_error_wlog */


/* ay_error:
 *  write error message to console according to:
 *  code (see error codes in ayam.h)
 *  where (function or command name)
 *  what (more exact specification of error, or additional info)
 */
void
ay_error(int code, const char *where, const char *what)
{
 Tcl_Interp *interp = ay_interp;
 Tcl_DString ds, dsl;
 char countstr[TCL_INTEGER_SPACE];
 static char *last_message = NULL;
 static int count = 0;
 int len;

  ay_errno = code;

  if(code == AY_OK)
    {
      goto set_ay_error;
    }

  Tcl_DStringInit(&ds);

  if((code == AY_EWARN) || (code == AY_EOUTPUT))
    {
      Tcl_DStringAppend(&ds, aye_warncmd, -1);
    }
  else
    {
      Tcl_DStringAppend(&ds, aye_errcmd, -1);
    }

  if(code == AY_EFLUSH)
    {
      if(count >= 1)
	{
	  Tcl_DStringInit(&dsl);
	  Tcl_DStringAppend(&dsl, aye_warncmd, -1);
	  Tcl_DStringAppend(&dsl, aye_lmsg1, -1);
	  len = sprintf(countstr, "%d", count);
	  Tcl_DStringAppend(&dsl, countstr, len);
	  Tcl_DStringAppend(&dsl, aye_lmsg2, -1);
	  Tcl_DStringAppend(&dsl, "} }", -1);
	  Tcl_Eval(interp, Tcl_DStringValue(&dsl));
	  Tcl_ResetResult(interp);

	  if(ay_prefs.writelog)
	    {
	      len = Tcl_DStringLength(&dsl);
	      if(len > 3)
		Tcl_DStringTrunc(&dsl, len-3);
	      ay_error_wlog(Tcl_DStringValue(&dsl));
	    }

	  Tcl_DStringFree(&dsl);
	}
      count = 0;
      if(last_message)
	{
	  free(last_message);
	  last_message = NULL;
	}

      Tcl_DStringFree(&ds);
      return;
    } /* if flush */

  if(where)
    {
      Tcl_DStringAppend(&ds, where, -1);
      Tcl_DStringAppend(&ds, ": ", -1);
    }

  switch(code)
    {
    case AY_EWARN:
      Tcl_DStringAppend(&ds, aye_gentxtwarn, -1);
      break;
    case AY_ERROR:
      Tcl_DStringAppend(&ds, aye_gentxterr, -1);
      break;
    case AY_EOMEM:
      Tcl_DStringAppend(&ds, "Out of memory!", -1);
      break;
    case AY_EOUTPUT:
      break;
    case AY_EOPENFILE:
      Tcl_DStringAppend(&ds, "Unable to open file: ", -1);
      break;
    case AY_ECLOSEFILE:
      Tcl_DStringAppend(&ds, "Unable to close file: ", -1);
      break;
    case AY_EFORMAT:
      Tcl_DStringAppend(&ds, "Wrong file format!", -1);
      break;
    case AY_ERANGE:
      Tcl_DStringAppend(&ds, "Parameter out of range: ", -1);
      break;
    case AY_ENOSEL:
      Tcl_DStringAppend(&ds, "No object(s) selected!", -1);
      break;
    case AY_EARGS:
      Tcl_DStringAppend(&ds, "Insufficient arguments, need: ", -1);
      break;
    case AY_EOPT:
      Tcl_DStringAppend(&ds, "Missing or malformed value for option: ", -1);
      break;
    case AY_EUOPT:
      Tcl_DStringAppend(&ds, "Encountered unknown option: ", -1);
      break;
    case AY_EWTYPE:
      Tcl_DStringAppend(&ds, "Object is of wrong type, need: ", -1);
      break;
    case AY_ETYPE:
      Tcl_DStringAppend(&ds, "Type exists already!", -1);
      break;
    case AY_ENTYPE:
      Tcl_DStringAppend(&ds, "No such object type: ", -1);
      break;
    case AY_EREF:
      Tcl_DStringAppend(&ds, "Reference counter is not zero!", -1);
      break;
    case AY_ENULL:
      Tcl_DStringAppend(&ds,
		      "Internal error: Illegal NULL pointer encountered!", -1);
      break;
    case AY_EUEOF:
      Tcl_DStringAppend(&ds, "Unexpected EOF encountered: ", -1);
      break;
    default:
      Tcl_DStringAppend(&ds, aye_gentxterr, -1);
      break;
    } /* switch */

  if(what)
    {
      Tcl_DStringAppend(&ds, what, -1);
    }

  if((code == AY_EARGS) || (code == AY_EWTYPE))
    {
      Tcl_DStringAppend(&ds, ".", -1);
    }

  Tcl_DStringAppend(&ds, "} }", -1);

  if(last_message && !strcmp(last_message, Tcl_DStringValue(&ds)))
    {
      /* last messages were identical, just count and exit */
      count++;
      Tcl_DStringFree(&ds);
      goto set_ay_error;
    }
  else
    {
      /* last messages were not identical,
	 save the current message,
	 output the saved message */
      if(count >= 1)
	{
	  Tcl_DStringInit(&dsl);
	  Tcl_DStringAppend(&dsl, aye_warncmd, -1);
	  Tcl_DStringAppend(&dsl, aye_lmsg1, -1);
	  len = sprintf(countstr, "%d", count);
	  Tcl_DStringAppend(&dsl, countstr, len);
	  Tcl_DStringAppend(&dsl, aye_lmsg2, -1);
	  Tcl_DStringAppend(&dsl, "} }", -1);
	  if(((code == AY_EOUTPUT) && (ay_prefs.errorlevel > 2)) ||
	     ((code == AY_EWARN) && (ay_prefs.errorlevel > 1)) ||
	     ay_prefs.errorlevel > 0)
	    {
	      Tcl_Eval(interp, Tcl_DStringValue(&dsl));
	      Tcl_ResetResult(interp);
	    }
	  if(ay_prefs.writelog)
	    {
	      len = Tcl_DStringLength(&dsl);
	      if(len > 3)
		Tcl_DStringTrunc(&dsl, len-3);
	      ay_error_wlog(Tcl_DStringValue(&dsl));
	    }
	  Tcl_DStringFree(&dsl);
	} /* if */
	count = 0;
    } /* if */

  /* output the current message */
  if(((code == AY_EOUTPUT) && (ay_prefs.errorlevel > 2)) ||
     ((code == AY_EWARN) && (ay_prefs.errorlevel > 1)) ||
     ay_prefs.errorlevel > 0)
    {
      Tcl_Eval(interp, Tcl_DStringValue(&ds));
      Tcl_ResetResult(interp);
    }
  if(ay_prefs.writelog)
    {
      len = Tcl_DStringLength(&ds);
      if(len > 3)
	Tcl_DStringTrunc(&ds, len-3);
      ay_error_wlog(Tcl_DStringValue(&ds));
    }

  if(last_message)
    {
      free(last_message);
      last_message = NULL;
    }

  if(Tcl_DStringValue(&ds))
    {
      if(!(last_message = malloc((Tcl_DStringLength(&ds)+1) * sizeof(char))))
	{
	  fprintf(stderr,"Ayam: Cannot handle error message; out of memory!\n");
	  return;
	}
      strcpy(last_message, Tcl_DStringValue(&ds));
    }
  Tcl_DStringFree(&ds);

set_ay_error:
  if(code != AY_EOUTPUT)
    {
      Tcl_SetVar2Ex(interp, "ay_error", NULL, Tcl_NewIntObj(code),
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

 return;
} /* ay_error */


/* ay_error_tcmd:
 *  Report an error to the user.
 *  Implements the \a ayError scripting interface command.
 *  See also the corresponding section in the \ayd{scayerror}.
 *  \returns TCL_OK in any case.
 */
int
ay_error_tcmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
{
 char *place = NULL, *detail = NULL;
 int tcl_status = TCL_OK, ecode = AY_OK;

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "ecode [fname detail]");
      return TCL_OK;
    }

  if(argc == 4)
    {
      place = argv[2];
      detail = argv[3];
    }

  tcl_status = Tcl_GetInt(interp, argv[1], &ecode);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  ay_error(ecode, place, detail);

 return TCL_OK;
} /* ay_error_tcmd */


/* ay_error_glucb:
 *   GLU error callback
 */
void
ay_error_glucb(GLenum err)
{
 char fname[] = "GLU";

  ay_error(AY_ERROR, fname, (char *)gluErrorString(err));

 return;
} /* ay_error_glucb */


/** ay_error_reportdrange:
 * format and output double parameter range error
 *
 * \param fname location of error
 * \param pname parameter name
 * \param lb lower bound of range
 * \param ub uper bound of range
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_error_reportdrange(char *fname, char *pname, double lb, double ub)
{
 char *msg = NULL;

  if(pname)
    {
      if((msg = malloc((strlen(pname)+TCL_DOUBLE_SPACE*2+64)*sizeof(char))))
	sprintf(msg, "Parameter %s out of range, should be [%lg, %lg].",
		pname, lb, ub);
    }
  else
    {
      if((msg = malloc((TCL_DOUBLE_SPACE*2+64)*sizeof(char))))
	sprintf(msg, "Parameter out of range, should be [%lg, %lg].",
		lb, ub);
    }

  if(msg)
    {
      ay_error(AY_ERROR, fname, msg);
      free(msg);
    }
  else
    ay_error(AY_ERANGE, fname, pname);

 return AY_ERANGE;
} /* ay_error_reportdrange */


/** ay_error_reportirange:
 * format and output integer parameter range error
 *
 * \param fname location of error
 * \param pname parameter name
 * \param lb lower bound of range
 * \param ub uper bound of range
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_error_reportirange(char *fname, char *pname, int lb, int ub)
{
 char *msg = NULL;

  if(pname)
    {
      if((msg = malloc((strlen(pname)+TCL_INTEGER_SPACE*2+64)*sizeof(char))))
	sprintf(msg, "Parameter %s out of range, should be [%d, %d].",
		pname, lb, ub);
    }
  else
    {
      if((msg = malloc((TCL_INTEGER_SPACE*2+64)*sizeof(char))))
	sprintf(msg, "Parameter out of range, should be [%d, %d].",
		lb, ub);
    }

  if(msg)
    {
      ay_error(AY_ERROR, fname, msg);
      free(msg);
    }
  else
    {
      ay_error(AY_ERANGE, fname, pname);
    }

 return AY_ERANGE;
} /* ay_error_reportirange */


/** ay_error_reportnan:
 * format and output nan parameter error
 *
 * \param fname location of error
 * \param pname parameter name
 */
void
ay_error_reportnan(char *fname, char *pname)
{
 char *msg = NULL;

  if(pname)
    {
      if((msg = malloc(32+(strlen(pname))*sizeof(char))))
	{
	  sprintf(msg, "Parameter %s is NaN.", pname);
	}
    }

  if(pname && msg)
    {
      ay_error(AY_ERROR, fname, msg);
      free(msg);
    }
  else
    {
      ay_error(AY_ERROR, fname, "Parameter is NaN.");
    }

 return;
} /* ay_error_reportnan */


/** ay_error_reportobject
 * format and output object path name
 *
 * \param ecode error code (AY_E*)
 * \param fname location of error
 * \param o object (must be linked to the scene)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_error_reportobject(int ecode, char *fname, ay_object *o)
{
 char *opath = NULL;
 int found = AY_FALSE;
 size_t len = 0;

  if(!o)
    return AY_ENULL;

  (void)ay_object_getpathname(o, ay_root, &len, &found, &opath);
  if(opath)
    {
      ay_error(ecode, fname, opath);
      free(opath);
    }
  else
    return AY_ERROR;

 return AY_OK;
} /* ay_error_reportobject */


/* ay_error_printglerrortcmd:
 *  get current GL error state and string
 *  and call ay_error with it for output
 *  Implements the \a getGLErr scripting interface command.
 *  See also the corresponding section in the \ayd{scgetglerr}.
 *  \returns TCL_OK in any case.
 */
int
ay_error_printglerrortcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[])
{

  ay_error(AY_EOUTPUT, argv[0], (char *)gluErrorString(glGetError()));

 return TCL_OK;
} /* ay_error_printglerrortcmd */
