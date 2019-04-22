#ifndef _GLGRIB_LOAD_H
#define _GLGRIB_LOAD_H

void glgrib_load_z (const char *, int *, float **, unsigned int *, unsigned int **);
void glgrib_load_rgb (const char *, unsigned char **, int);
void glgrib_load (const char *, float **, int );

#endif
