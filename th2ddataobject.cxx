/**
 * @file th2ddataobject.cxx
 */
  
/* Copyright (C) 2000 Stacho Mudrak
 * 
 * $Date: $
 * $RCSfile: $
 * $Revision: $
 *
 * -------------------------------------------------------------------- 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * --------------------------------------------------------------------
 */
 
#include "th2ddataobject.h"
#include "thexception.h"
#include "thchenc.h"

th2ddataobject::th2ddataobject()
{
  this->pscrapoptr = NULL;
  this->nscrapoptr = NULL;
  this->fscrapptr = NULL;
  this->scale = TT_2DOBJ_SCALE_M;
  this->tags = TT_2DOBJ_TAG_CLIP_AUTO | TT_2DOBJ_TAG_VISIBILITY_ON;
  this->place = TT_2DOBJ_PLACE_NONE;
}


th2ddataobject::~th2ddataobject()
{
}


int th2ddataobject::get_class_id() 
{
  return TT_2DDATAOBJECT_CMD;
}


bool th2ddataobject::is(int class_id)
{
  if (class_id == TT_2DDATAOBJECT_CMD)
    return true;
  else
    return thdataobject::is(class_id);
}



thcmd_option_desc th2ddataobject::get_cmd_option_desc(char * opts)
{
  int id = thmatch_token(opts, thtt_2ddataobject_opt);
  if (id == TT_2DOBJ_UNKNOWN)
    return thdataobject::get_cmd_option_desc(opts);
  else
    return thcmd_option_desc(id);
}


void th2ddataobject::set(thcmd_option_desc cod, char ** args, int argenc, unsigned long indataline)
{
  int i;
  switch (cod.id) {
  
    case TT_2DOBJ_SCALE:
      this->scale = thmatch_token(*args, thtt_2dobj_scales);
      if (this->scale == TT_2DOBJ_SCALE_UNKNOWN)
        ththrow(("invalid scale -- %s",*args))
      break;    

    case TT_2DOBJ_CLIP:
      i = thmatch_token(*args, thtt_onoffauto);
      this->tags &= ~(TT_2DOBJ_TAG_CLIP_ON | TT_2DOBJ_TAG_CLIP_AUTO);
      switch (i) {
        case TT_TRUE:
          this->tags |= TT_2DOBJ_TAG_CLIP_ON;
          break;
        case TT_FALSE:
          break;
        case TT_AUTO:
          this->tags |= TT_2DOBJ_TAG_CLIP_AUTO;
          break;
        default:
          ththrow(("invalid clip -- %s",*args))
      }
      break;    

    case TT_2DOBJ_VISIBILITY:
      i = thmatch_token(*args, thtt_bool);
      this->tags &= ~TT_2DOBJ_TAG_VISIBILITY_ON;
      switch (i) {
        case TT_TRUE:
          this->tags |= TT_2DOBJ_TAG_VISIBILITY_ON;
          break;
        case TT_FALSE:
          break;
        default:
          ththrow(("invalid visibility switch -- %s",*args))
      }
      break;    

    case TT_2DOBJ_PLACE:
      this->place = thmatch_token(*args, thtt_2ddataobject_place);
      if (this->place == TT_2DOBJ_PLACE_UNKNOWN)
        ththrow(("invalid place value -- %s",*args))
      break;    

    default:
      thdataobject::set(cod, args, argenc, indataline);
      
  }
}


int th2ddataobject::get_context()
{
  return THCTX_SCRAP;
}


void th2ddataobject::self_print_properties(FILE * outf)
{
  thdataobject::self_print_properties(outf);
  fprintf(outf,"th2ddataobject:\n");
  fprintf(outf,"\tscale: %d\n",this->scale);
  fprintf(outf,"\ttags: %d\n",this->tags);
  // insert intended print of object properties here
}

