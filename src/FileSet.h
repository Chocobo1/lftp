/*
 * lftp and utils
 *
 * Copyright (c) 1996-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $Id$ */

#ifndef FILESET_H
#define FILESET_H

#include <sys/types.h>
#include "xmalloc.h"

CDECL_BEGIN
#include <regex.h>
CDECL_END

#undef TYPE

class TimeInterval;

class FileInfo
{
public:
   char	    *name;
   mode_t   mode;
   time_t   date;
   off_t    size;
   void	    *data;
   const char *user, *group;
   int      nlinks;

   enum	 type
   {
      DIRECTORY,
      SYMLINK,
      NORMAL
   };

   type	 filetype;
   char	 *symlink;

   int	 defined;
   enum defined_bits
   {
      NAME=001,MODE=002,DATE=004,TYPE=010,SYMLINK_DEF=020,
      DATE_UNPREC=040,SIZE=0100,USER=0200,GROUP=0400,NLINKS=01000,

      IGNORE_SIZE_IF_OLDER=02000, // for ignore mask
      IGNORE_DATE_IF_OLDER=04000, // for ignore mask

      ALL_INFO=NAME|MODE|DATE|TYPE|SYMLINK_DEF|DATE_UNPREC|SIZE|USER|GROUP|NLINKS
   };

   ~FileInfo();
   void Init();
   FileInfo()
      {
	 Init();
      }
   FileInfo(const FileInfo &fi);

   void SetName(const char *n);
   void SetUser(const char *n);
   void SetGroup(const char *n);
   FileInfo(const char *n) { Init(); SetName(n); }
   void LocalFile(const char *name, bool follow_symlinks);

   void SetMode(mode_t m) { mode=m; defined|=MODE; }
   void SetDate(time_t t) { date=t; defined|=DATE; defined&=~DATE_UNPREC; }
   void SetDateUnprec(time_t t)
      {
	 if(defined&DATE) return;
	 if(t==(time_t)-1) return;
	 date=t; defined|=DATE_UNPREC;
      }
   void SetType(type t) { filetype=t; defined|=TYPE; }
   void SetSymlink(const char *s) { xfree(symlink); symlink=xstrdup(s);
      filetype=SYMLINK; defined|=TYPE|SYMLINK_DEF; }
   void	SetSize(off_t s) { size=s; defined|=SIZE; }
   void	SetNlink(int n) { nlinks=n; defined|=NLINKS; }

   void	 Merge(const FileInfo&);

   bool	 SameAs(const FileInfo *,
	    const TimeInterval *prec,const TimeInterval *loose_prec,int ignore);
   bool	 OlderThan(time_t t);

   void	 SetAssociatedData(void *d,int len)
      {
	 xfree(data);
	 data=xmemdup(d,len);
      }
   void  *GetAssociatedData() { return data; }

   operator const char *() { return name; }
};

class FileSet
{
public:
   enum sort_e { BYNAME, BYSIZE, DIRSFIRST };

private:
   FileInfo **files;

   /* Alternate pointers when sort != NAME: */
   FileInfo **files_sort;

   int	 fnum;

   int	 ind;

   void	 Sub(int);
   bool sorted;

public:
   FileSet()
   {
      files=files_sort=0;
      sorted = false;
      fnum=0;
      ind=0;
   }
   FileSet(const FileSet *s);
   ~FileSet();
   void Empty();

   int	 get_fnum() const { return fnum; }

   void	 Add(FileInfo *);
   void	 Merge(const FileSet *);
   void	 Merge(char **);   // file list
   void	 SubtractSame(const FileSet *,
	    const TimeInterval *prec,const TimeInterval *loose_prec,int ignore);
   void	 SubtractAny(const FileSet *);
   void  SubtractOlderThan(time_t t);
   void  SubtractNotIn(const FileSet *);
   void  Sort(sort_e newsort, bool casefold=false);
   void  Unsort();

   void	 Exclude(const char *prefix,regex_t *exclude,regex_t *include);
   void  Exclude(const char *prefix,const char *exclude,const char *include);
   void	 ExcludeDots();

   void	 rewind() { ind=0; }
   FileInfo *curr();
   FileInfo *next();

   void	 LocalRemove(const char *dir);
   void	 LocalUtime(const char *dir,bool only_dirs=false);
   void	 LocalChmod(const char *dir,mode_t mask=0);

   void Count(int *d,int *f,int *s,int *o);

   int FindGEIndByName(const char *name) const;
   FileInfo *FindByName(const char *name) const;

   void  SetSize(const char *name,off_t size)
   {
      FileInfo *f=FindByName(name);
      if(f)
	 f->SetSize(size);
   }
   void  SetDate(const char *name,time_t date)
   {
      FileInfo *f=FindByName(name);
      if(f)
	 f->SetDate(date);
   }

   /* add a path to all files */
   void PrependPath(const char *path);

   /* get all defined_bits used by this fileset */
   int Have() const;

   FileInfo * operator[](int i) const;
};

#endif // FILESET_H
