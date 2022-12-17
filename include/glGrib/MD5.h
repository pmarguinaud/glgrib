#ifndef _GLGRIB_MD5_H
#define _GLGRIB_MD5_H

#include <openssl/evp.h>
#include <string>

namespace glGrib
{

class MD5
{
public:
  MD5 ();
  ~MD5 ();
  void update (const void * data, unsigned long len)
  {
    EVP_DigestUpdate (mdctx, data, len);
  }
  const std::string asString () const;
private:
  EVP_MD_CTX * mdctx = nullptr;
};


};

#endif
