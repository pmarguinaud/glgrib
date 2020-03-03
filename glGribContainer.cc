#include "glGribContainer.h"
#include <eccodes.h>
#include <stdlib.h>
#include <algorithm>
#include <stdexcept>
#include <map>


extern "C"
{
#include "lfi/lfi_grok.h"
#include "lfi/lfi_type.h"
#include "lfi/lfi_args.h"
#include "lfi/lfi_fort.h"
extern void lfiouv_mt64_ (LFIOUV_ARGS_DECL);
extern void lfinfo_mt64_ (LFINFO_ARGS_DECL);
extern void lfilec_mt64_ (LFILEC_ARGS_DECL);
extern void lfifer_mt64_ (LFIFER_ARGS_DECL);
extern void lfipos_mt64_ (LFIPOS_ARGS_DECL);
extern void lficas_mt64_ (LFICAS_ARGS_DECL);
extern void lfierf_mt64_ (LFIERF_ARGS_DECL);
}

namespace glGrib
{

class containerPlain : public glGrib::Container
{
public:
  containerPlain (const std::string & file) : glGrib::Container (file) {}
  codes_handle * getHandleByExt (const std::string &) override;
  const std::string & getNextExt (const std::string &) override;
  const std::string & getPrevExt (const std::string &) override;
  bool hasExt (const std::string & ext) const override
  {
    return std::any_of (index.cbegin (), index.cend (), 
                        [&ext](const index_elt_t & e) { return e.ext == ext; });
  }
  iterator begin () override
  {
    _iteratorPlain * it = new _iteratorPlain ();
    it->cont = this;
    this->buildIndex ();
    it->rank = 0;
    return it;
  }
  iterator end () override
  {
    return iterator ();
  }
  void open ()
  {
    fp = fopen (getFile ().c_str (), "r");
    if (fp == nullptr)
      throw std::runtime_error (std::string ("Could not open ") + 
                                getFile () + std::string (" for reading"));
  }
  void close ()
  {
    fclose (fp);
    fp = nullptr;
  }
  void buildIndex () override;
private:
  codes_handle * searchHandleByExt (const std::string &);


  class index_elt_t
  {
  public:
    index_elt_t (const std::string & _ext, long _off) : ext (_ext), off (_off) {}
    std::string ext;
    long off = -1;
  };
  class index_t : public std::vector<index_elt_t>
  {
  public:
    void add (const std::string & ext, long offset)
    {
      push_back (index_elt_t (ext, offset));
    }
  };

  class _iteratorPlain : public _iterator
  {
  public:
    _iteratorPlain () 
    {
    }
    void incr () override
    {
      rank++;
    }
    const std::string & str () override
    {
      return cont->index[rank].ext;
    }
    bool isEqual (const _iterator * _other) const override
    {
      // Both at end
      if ((this == nullptr) && (_other == nullptr))
        return true;

      // At end or not
      if (_other == nullptr)
        return rank == cont->index.size ();

      const _iteratorPlain * other = dynamic_cast<const _iteratorPlain *>(_other);

      // At end ?
      if (this == nullptr)
        return other->rank == other->cont->index.size ();

      // Do we operate on same container
      if (cont != other->cont)
        return false;

      return rank == other->rank;
    }
    glGrib::containerPlain * cont = nullptr;
    int rank = 0;
  };

  friend class _iteratorPlain;
  std::vector<std::string> keys = {"discipline", "parameterCategory", "parameterNumber", 
                                   "level", "indicatorOfParameter", "indicatorOfTypeOfLevel",
                                   "table2Version"};
  FILE * fp = nullptr;
  index_t index;
  long offset = 0;
};

}

static std::string empty  = "";

const std::string & glGrib::containerPlain::getNextExt (const std::string & ext) 
{
  if ((ext == empty) && (index.size () > 0))
    return index.front ().ext;
  for (int i = 0; i < index.size ()-1; i++)
    if (index[i].ext == ext)
      return index[i+1].ext;
  return empty;
}

const std::string & glGrib::containerPlain::getPrevExt (const std::string & ext) 
{
  if ((ext == empty) && (index.size () > 0))
    return index.back ().ext;
  for (int i = 1; i < index.size (); i++)
    if (index[i].ext == ext)
      return index[i-1].ext;
  return empty;
}

namespace glGrib
{

class containerFA : public glGrib::Container
{
public:
  containerFA (const std::string & file) : glGrib::Container (file) {}
  codes_handle * getHandleByExt (const std::string &) override;
  const std::string & getNextExt (const std::string &) override;
  const std::string & getPrevExt (const std::string &) override;
  bool hasExt (const std::string & ext) const override
  {
    return std::any_of (names.cbegin (), names.cend (), 
                        [&ext](const std::string & e) { return e == ext; });
  }
  iterator begin () override
  {
    _iteratorFA * it = new _iteratorFA ();
    it->cont = this;
    this->buildIndex ();
    return iterator (it);
  }
  iterator end () override
  {
    return iterator ();
  }
  void buildIndex () override;
private:
  class _iteratorFA : public _iterator
  {
  public:
    void incr () override
    {
      rank++;
    }
    const std::string & str () override
    {
      return cont->names[rank];
    }
    bool isEqual (const _iterator * _other) const override
    {
      // Both at end
      if ((this == nullptr) && (_other == nullptr))
        return true;

      // At end or not
      if (_other == nullptr)
        return rank == cont->names.size ();

      const _iteratorFA * other = dynamic_cast<const _iteratorFA *>(_other);
     
      // At end ?
      if (this == nullptr)
        return other->rank == other->cont->names.size (); 

      // Do we operate on same container
      if (cont != other->cont)
        return false;

      return rank == other->rank;
    }
    glGrib::containerFA * cont = nullptr;
    int rank = 0;
  };
  friend class _iteratorFA;
  std::vector<std::string> names;
  integer64 INUMER = 77;
  void * LFI;
  lficom_t lficomm;
  void open ();
  void close ();
};

}

const std::string & glGrib::containerFA::getNextExt (const std::string & ext) 
{
  if ((ext == empty) && (names.size () > 0))
    return names.front ();
  for (int i = 0; i < names.size ()-1; i++)
    if (names[i] == ext)
      return names[i+1];
  return empty;
}

const std::string & glGrib::containerFA::getPrevExt (const std::string & ext) 
{
  if ((ext == empty) && (names.size () > 0))
    return names.back ();
  for (int i = 1; i < names.size (); i++)
    if (names[i] == ext)
      return names[i-1];
  return empty;
}

class container_cache_t : public std::map<std::string,glGrib::Container *>
{
public:
   void add (const std::string & file, glGrib::Container * cont)
   {
     insert (std::pair<std::string,glGrib::Container *>(file, cont));
   }
   glGrib::Container * get (const std::string & file)
   {
     auto it = find (file);
     if (it == end ())
       return nullptr;
     return it->second;
   }
};

static container_cache_t contCache;

void glGrib::Container::remove (glGrib::Container * cont)
{
  if (contCache.get (cont->getFile ()) == nullptr)
    delete cont;
}

glGrib::Container * glGrib::Container::create (const std::string & file, bool keep)
{
  glGrib::Container * cont = contCache.get (file);

  if (cont == nullptr)
    {
      lfi_grok_t type = lfi_grok (file.c_str (), file.length ());
      switch (type)
        {
          case LFI_NONE:
            throw std::runtime_error (std::string ("Could not open ") + 
                                      file + std::string (" for reading"));
            break;
          case LFI_UNKN:
            cont = new glGrib::containerPlain (file);
            break;
          case LFI_NETW:
          case LFI_PURE:
          case LFI_ALTM:
            cont = new glGrib::containerFA (file);
            break;
        }
      if (keep)
        contCache.add (file, cont);
    }

  return cont;
}

void glGrib::Container::clear ()
{
  contCache.clear ();
}

void glGrib::containerPlain::buildIndex ()
{
  if (index.size () > 0)
    return;

  int err = 0;
  
  this->open ();

  fseek (fp, offset, SEEK_SET);

  while (1)
    {
      long off = ftell (fp);
      codes_handle * h = codes_handle_new_from_file (0, fp, PRODUCT_GRIB, &err);
      if (h == nullptr)
        break;
      std::string ext;
      for (const auto & k : keys)
        {
	  size_t len = 256;
          char tmp[len];
          if (ext != "")
            ext += ",";
          if (codes_is_defined (h, k.c_str ()))
            {
              codes_get_string (h, k.c_str (), tmp, &len);
	      ext += k + "=\"" + std::string (tmp) + "\"";
	    }
	}
      index.add (ext, off);
      codes_handle_delete (h);
      h = nullptr;
    }

  offset = ftell (fp);

  this->close ();
}

codes_handle * glGrib::containerPlain::searchHandleByExt (const std::string & ext)
{
  int err = 0;
  codes_handle * h = nullptr;

  this->open ();

  while ((h = codes_handle_new_from_file (0, fp, PRODUCT_GRIB, &err)))
    {
      std::string e = ext;

      // Parse GRIB selector and see if current handle complies
      while (e.length ())
        {
          int p;
	  p = e.find_first_of (',');
	  std::string m;

	  if (p == std::string::npos)
            {
              m = e;
	      e = "";
	    }
	  else
	    {
	      m = e.substr (0, p);
	      e = e.substr (p+1);
	    }

          p = m.find_first_of ('=');

          if (p == std::string::npos)
            throw std::runtime_error (std::string ("Malformed GRIB selector ") + ext);

	  // Key, value
	  std::string key = m.substr (0, p), val0 = m.substr (p+1);
          
          if (! codes_is_defined (h, key.c_str ()))
            goto next;

	  // Handle string & integer values
	  bool string = (val0[0] == '"') && (val0[val0.length ()-1] == '"');

	  if (string)
            val0 = val0.substr (1, val0.length () - 2);

	  if (string)
            {
	      size_t len = 128;
              char tmp[len];
	      grib_get_string (h, key.c_str (), tmp, &len);
	      std::string val1 = std::string (tmp);
	      if (val0 != val1)
                goto next;
	    }
	  else
	    {
	      long int v0, v1;

	      try
                {
	          v0 = std::stoi (val0);
	        }
	      catch (...)
	        {
                  throw std::runtime_error (std::string ("Malformed GRIB selector ") + ext);
	        }

              codes_get_long (h, key.c_str (), &v1);
	      if (v0 != v1)
	        goto next;
	    }
        }

      // If we got here, then the GRIB message complies with the selector : keep GRIB handle and exit the loop
      
      break;
next:
      // Free GRIB handle
      codes_handle_delete (h);
      h = nullptr;
    }

  this->close ();

  return h;
}

codes_handle * glGrib::containerPlain::getHandleByExt (const std::string & ext)
{
  int err = 0;
  codes_handle * h = nullptr;

  if (index.size () > 0)
    for (const auto & e : index)
      if (e.ext == ext)
        {
          int err = 0;
          this->open ();
          fseek (fp, e.off, SEEK_SET);
          h = codes_handle_new_from_file (0, fp, PRODUCT_GRIB, &err);
          this->close ();
	}

  if (h == nullptr)
    h = searchHandleByExt (ext);

  if (h == nullptr)
    throw std::runtime_error (std::string ("No match for ") + ext + std::string (" in file ") + getFile ());
 
  return h;
}

void glGrib::containerFA::open () 
{
  LFI = &lficomm;
  strncpy (lficomm.cmagic, "LFI_FORT", 8);
  lficomm.lfihl = nullptr;

  integer64 IREP, INUMER = 77, INIMES = 0, INBARP = 0, INBARI = 0;
  logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_FALSE;
  character * CLNOMF = (character*)getFile ().c_str (), * CLSTTO = (character*)"OLD";
  character_len CLNOMF_len = getFile ().length (), CLSTTO_len = 3;
  lfiouv_mt64_ (LFI, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, 
      	  &INIMES, &INBARP, &INBARI, CLNOMF_len, CLSTTO_len);
  if (IREP != 0)
    throw std::runtime_error (std::string ("Error opening file ") + getFile ());

}
void glGrib::containerFA::close ()
{
  integer64 IREP;
  character * CLSTTC = (character*)"KEEP"; 
  character_len CLSTTC_len = 4;
  lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);
}

void glGrib::containerFA::buildIndex ()
{
  if (names.size () != 0)
    return;

  logical LLAVAN = fort_TRUE;
  integer64 IREP, ILONG, IPOSEX;

  this->open ();

  lfipos_mt64_ (LFI, &IREP, &INUMER);
  std::vector<std::string> nn;
  
  while (1)
    {
      std::string clnoma (17, '\0');
      lficas_mt64_ (LFI, &IREP, &INUMER, (character*)clnoma.c_str (), 
    		    &ILONG, &IPOSEX, &LLAVAN, clnoma.length ());
      if (ILONG == 0)
        break;
      nn.push_back (clnoma);
    } 

  logical LLERFA = fort_FALSE;
  lfierf_mt64_ (LFI, &IREP, &INUMER, &LLERFA);

  for (const auto & n : nn)
    {
      character * CLNOMA = (character*)n.c_str ();
      character_len CLNOMA_len = n.length ();
      integer64 ILONGD = 3;
      integer64 ITAB[3] = {0, 0, 0};
      lfilec_mt64_ (LFI, &IREP, &INUMER, CLNOMA, ITAB, &ILONGD, CLNOMA_len);
      if ((ITAB[0] == 120) && (ITAB[1] == 0)) // Grid-point GRIB2
        names.push_back (n);
    }

  this->close ();
}

codes_handle * glGrib::containerFA::getHandleByExt (const std::string & ext)
{
  codes_handle * h = nullptr;

  character * CLNOMA = (character*)ext.c_str ();
  character_len CLNOMA_len = ext.length ();

  integer64 IREP = 0, ILONG = 0, IPOSEX = 0;
  integer64 * ITAB = nullptr;

  this->open ();


  lfinfo_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &ILONG, &IPOSEX, CLNOMA_len);

  if (IREP != 0)
    throw std::runtime_error (std::string ("File ") + getFile () + 
		              std::string (" does not contains ") + ext);

  ITAB = new integer64[ILONG];
  lfilec_mt64_ (LFI, &IREP, &INUMER, CLNOMA, ITAB, &ILONG, CLNOMA_len);

  if (IREP != 0)
    throw std::runtime_error (std::string ("Reading ") + ext + 
		    std::string (" in ") + getFile () + std::string (" failed"));

  this->close ();

  if (IREP != 0)
    throw std::runtime_error (std::string ("Closing ") + getFile () + std::string (" failed"));

  h = codes_handle_new_from_message_copy (0, ITAB + 3, 8 * (ILONG - 3));
  delete [] ITAB;

  if (h == nullptr)
    throw std::runtime_error (std::string ("Article ") + ext + std::string (" of file ") + getFile () +
                              std::string (" does not contain a GRIB message"));
 
  return h;
}

