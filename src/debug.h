#ifndef DEBUG_H
#define DEBUG_H


GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

#endif // DEBUG_H