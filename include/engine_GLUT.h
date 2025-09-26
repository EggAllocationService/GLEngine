// helper header for multi-platform support
#pragma once

#define GL_SILENCE_DEPRECATION
#if defined(__APPLE__) && defined(__MACH__)
# include <GLUT/glut.h>
#else
# include <freeglut.h>
#endif