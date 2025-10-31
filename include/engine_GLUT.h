// helper header for multi-platform support
// my main development platform is macOS
#pragma once

#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>

#if defined(__APPLE__) && defined(__MACH__)
# include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif