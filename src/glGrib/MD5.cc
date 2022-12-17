#include "glGrib/MD5.h"

#include <iostream>

namespace glGrib
{

MD5::MD5 ()
{
  mdctx = EVP_MD_CTX_new ();
  EVP_DigestInit_ex (mdctx, EVP_md5 (), nullptr);
}

MD5::~MD5 ()
{
  EVP_MD_CTX_free (mdctx);
}

const std::string MD5::asString () const
{
  const char * const lut = "0123456789ABCDEF";
  unsigned int md5_digest_length = EVP_MD_size (EVP_md5 ());
  unsigned char md5_digest[md5_digest_length];

  EVP_DigestFinal_ex (mdctx, md5_digest, &md5_digest_length);

  std::string str;
  str.reserve (2 * md5_digest_length);

  for (size_t i = 0; i < md5_digest_length; i++)
    {   
      const unsigned char c = md5_digest[i];
      str.push_back (lut[c >> 4]);
      str.push_back (lut[c & 15]);
    }   

  return str;
}

}
