#include "glgrib_sqlite.h"

int main (int argc, char * argv[])
{
  glgrib_sqlite db ("glgrib.db");
  float x, y, z;
  std::string CLNOMA = "SFX.SST";
  std::string CLNOMB;
  std::string OPTIONS;
  char tmp[32];
  int k;
  glgrib_sqlite::stmt st = db.prepare ("SELECT CLNOMA, OPTIONS FROM CLNOMA2OPTIONS WHERE CLNOMA = ?;");
  st.ibind (&CLNOMA);
  st.obind (&CLNOMB, &OPTIONS);
  db.execute (&st);
  return 0;
}
