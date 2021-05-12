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

/* otype.c - object type management */

typedef struct cid_s
{
  struct cid_s *next; /**< next id */
  unsigned int id; /**< id */
} cid;

unsigned int *ids;

cid *cids;

/** ay_otype_register:
 *  Register a new (custom) object type, filling pointers
 *  to the callback functions into various global callback tables.
 *
 *  It is also possible to overwrite the callbacks of a core or
 *  custom object type by calling this function with an already
 *  registered type name.
 *
 * \param[in] name type name
 * \param[in] crtcb create callback
 * \param[in] delcb delete callback
 * \param[in] copycb copy callback
 * \param[in] drawcb draw callback
 * \param[in] drawhcb draw handles callback
 * \param[in] shadecb shade callback
 * \param[in] setpropcb set property callback
 * \param[in] getpropcb get property callback
 * \param[in] getpntcb get point callback
 * \param[in] readcb read callback
 * \param[in] writecb write callback
 * \param[in] wribcb RIB export callback
 * \param[in] bbccb bounding box calculation callback
 * \param[in,out] type_index pointer where to store the type index
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_otype_register(char *name,
		  ay_createcb  *crtcb,
		  ay_deletecb  *delcb,
		  ay_copycb    *copycb,
		  ay_drawcb    *drawcb,
		  ay_drawcb    *drawhcb,
		  ay_drawcb    *shadecb,
		  ay_propcb    *setpropcb,
		  ay_propcb    *getpropcb,
		  ay_getpntcb  *getpntcb,
		  ay_readcb    *readcb,
		  ay_writecb   *writecb,
		  ay_wribcb    *wribcb,
		  ay_bbccb     *bbccb,
		  unsigned int *type_index)
{
 int ay_status = AY_OK;
 static unsigned int tc = AY_IDLAST; /* type counter and index into tables */
 int increase_tc = AY_TRUE;
 int i = 0, new_item = 0;
 Tcl_HashEntry *entry = NULL;

 /* check, whether type is already registered */
  if((entry = Tcl_FindHashEntry(&ay_otypesht, name)))
    {
      /* yes, we already registered this type */
      increase_tc = AY_FALSE;
      i = *((unsigned int*) Tcl_GetHashValue(entry));
    }
  else
    {
      /* no, this is a new type */
      entry = Tcl_CreateHashEntry(&ay_otypesht, name, &new_item);
      Tcl_SetHashValue(entry, ay_otype_getpointer(tc));
      i = tc;
    }

  /* register typename */
  if((ay_status = ay_table_additem(&ay_typenamest, (void*)name, i)))
    return ay_status;

  /* register create callback */
  if((ay_status = ay_table_addcallback(&ay_createcbt, (ay_voidfp)crtcb, i)))
    return ay_status;

  /* register delete callback */
  if((ay_status = ay_table_addcallback(&ay_deletecbt, (ay_voidfp)delcb, i)))
    return ay_status;

  /* register copy callback */
  if((ay_status = ay_table_addcallback(&ay_copycbt, (ay_voidfp)copycb, i)))
    return ay_status;

  /* register draw callback */
  if((ay_status = ay_table_addcallback(&ay_drawcbt, (ay_voidfp)drawcb, i)))
    return ay_status;

  /* register draw points callback */
  if((ay_status = ay_table_addcallback(&ay_drawhcbt, (ay_voidfp)drawhcb, i)))
    return ay_status;

  /* register shade callback */
  if((ay_status = ay_table_addcallback(&ay_shadecbt, (ay_voidfp)shadecb, i)))
    return ay_status;

  /* register setprop callback */
  if((ay_status = ay_table_addcallback(&ay_setpropcbt, (ay_voidfp)setpropcb, i)))
    return ay_status;

  /* register getprop callback */
  if((ay_status = ay_table_addcallback(&ay_getpropcbt, (ay_voidfp)getpropcb, i)))
    return ay_status;

  /* register pickpnt callback */
  if((ay_status = ay_table_addcallback(&ay_getpntcbt, (ay_voidfp)getpntcb, i)))
    return ay_status;

  /* register read callback */
  if((ay_status = ay_table_addcallback(&ay_readcbt, (ay_voidfp)readcb, i)))
    return ay_status;

  /* register write callback */
  if((ay_status = ay_table_addcallback(&ay_writecbt, (ay_voidfp)writecb, i)))
    return ay_status;

  /* register wrib callback */
  if((ay_status = ay_table_addcallback(&ay_wribcbt, (ay_voidfp)wribcb, i)))
    return ay_status;

  /* register bbc callback */
  if((ay_status = ay_table_addcallback(&ay_bbccbt, (ay_voidfp)bbccb, i)))
    return ay_status;

  *type_index = i;

  if(increase_tc)
    tc++;

 return ay_status;
} /* ay_otype_register */


/** ay_otype_registercore:
 *  Register a new core object type, filling pointers
 *  to the callback functions into various global callback tables.
 *
 *  It is not possible to overwrite the callbacks of a core object
 *  type by calling this function multiple times with the same name.
 *
 * \param[in] name type name
 * \param[in] crtcb create callback
 * \param[in] delcb delete callback
 * \param[in] copycb copy callback
 * \param[in] drawcb draw callback
 * \param[in] drawhcb draw handles callback
 * \param[in] shadecb shade callback
 * \param[in] setpropcb set property callback
 * \param[in] getpropcb get property callback
 * \param[in] getpntcb get point callback
 * \param[in] readcb read callback
 * \param[in] writecb write callback
 * \param[in] wribcb RIB export callback
 * \param[in] bbccb bounding box calculation callback
 * \param[in] type_index index in callback table
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_otype_registercore(char *name,
		      ay_createcb  *crtcb,
		      ay_deletecb  *delcb,
		      ay_copycb    *copycb,
		      ay_drawcb    *drawcb,
		      ay_drawcb    *drawhcb,
		      ay_drawcb    *shadecb,
		      ay_propcb    *setpropcb,
		      ay_propcb    *getpropcb,
		      ay_getpntcb  *getpntcb,
		      ay_readcb    *readcb,
		      ay_writecb   *writecb,
		      ay_wribcb    *wribcb,
		      ay_bbccb     *bbccb,
		      unsigned int type_index)
{
 int ay_status = AY_OK;
 /* char fname[] = "ay_otype_registercore";*/
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;

  /* check, whether type is already registered */
  /* this should not happen, btw. ... */
  if(Tcl_FindHashEntry(&ay_otypesht, name))
    { /* yes, we already have registered this type */
      return AY_ETYPE;
    }

  /* no, this is a new type */
  entry = Tcl_CreateHashEntry(&ay_otypesht, name, &new_item);
  Tcl_SetHashValue(entry, ay_otype_getpointer(type_index));

  /* register typename */
  if((ay_status = ay_table_additem(&ay_typenamest, (void*)name, type_index)))
    return ay_status;

  /* register create callback */
  if((ay_status = ay_table_addcallback(&ay_createcbt, (ay_voidfp)crtcb,
				       type_index)))
    return ay_status;

  /* register delete callback */
  if((ay_status = ay_table_addcallback(&ay_deletecbt, (ay_voidfp)delcb,
				       type_index)))
    return ay_status;

  /* register copy callback */
  if((ay_status = ay_table_addcallback(&ay_copycbt, (ay_voidfp)copycb,
				       type_index)))
    return ay_status;

  /* register draw callback */
  if((ay_status = ay_table_addcallback(&ay_drawcbt, (ay_voidfp)drawcb,
				       type_index)))
    return ay_status;

  /* register draw points callback */
  if((ay_status = ay_table_addcallback(&ay_drawhcbt, (ay_voidfp)drawhcb,
				       type_index)))
    return ay_status;

  /* register shade callback */
  if((ay_status = ay_table_addcallback(&ay_shadecbt, (ay_voidfp)shadecb,
				       type_index)))
    return ay_status;

  /* register setprop callback */
  if((ay_status = ay_table_addcallback(&ay_setpropcbt, (ay_voidfp)setpropcb,
				       type_index)))
    return ay_status;

  /* register getprop callback */
  if((ay_status = ay_table_addcallback(&ay_getpropcbt, (ay_voidfp)getpropcb,
				       type_index)))
    return ay_status;

  /* register pickpnt callback */
  if((ay_status = ay_table_addcallback(&ay_getpntcbt, (ay_voidfp)getpntcb,
				       type_index)))
    return ay_status;

  /* register read callback */
  if((ay_status = ay_table_addcallback(&ay_readcbt, (ay_voidfp)readcb,
				       type_index)))
    return ay_status;

  /* register write callback */
  if((ay_status = ay_table_addcallback(&ay_writecbt, (ay_voidfp)writecb,
				       type_index)))
    return ay_status;

  /* register wrib callback */
  if((ay_status = ay_table_addcallback(&ay_wribcbt, (ay_voidfp)wribcb,
				       type_index)))
    return ay_status;

  /* register bbc callback */
  if((ay_status = ay_table_addcallback(&ay_bbccbt, (ay_voidfp)bbccb,
				       type_index)))
    return ay_status;

 return ay_status;
} /* ay_otype_registercore */


/** ay_otype_getpointer:
 * Create a pointer for use as ID Tcl hash table value.
 *
 * \param[in] id integer value for ID
 *
 * \returns pointer or NULL
 */
unsigned int *
ay_otype_getpointer(unsigned int id)
{
 unsigned int *pointer = NULL;
 cid *c = cids;

  if(id < AY_IDLAST)
    {
      pointer = &(ids[id]);
    }
  else
    {
      while(c)
	{
	  if(id == c->id)
	    {
	      pointer = &(c->id);
	      break;
	    }
	  if(!c->next /*&& !pointer*/)
	    {
	      if((c->next = calloc(1, sizeof(cid))))
		{
		  c->next->id = id;
		  pointer = &(c->next->id);
		}
	      break;
	    }
	  c = c->next;
	}
    }

 return pointer;
} /* ay_otype_getpointer */


/** ay_otype_init:
 * Initialize object type module.
 * Must be called before registering any object or tag types.
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_otype_init()
{
 unsigned int i;

  if(!(ids = malloc(AY_IDLAST*sizeof(unsigned int))))
    return AY_EOMEM;

  for(i = 0; i < AY_IDLAST; i++)
    ids[i] = i;

  if(!(cids = calloc(1, sizeof(cid))))
    {
      free(ids);
      return AY_EOMEM;
    }

  cids->id = AY_IDLAST;

 return AY_OK;
} /* ay_otype_init */

