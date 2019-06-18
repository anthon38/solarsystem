#ifndef PATH_H
#define PATH_H

#include <QString>

#ifndef Q_OS_ANDROID
inline static const QString resPath() {return "";}
inline static const QString shadersDir() {return "shadersES2/";}
#else
inline static const QString resPath() {return "assets:/";}
inline static const QString shadersDir() {return "shadersES2/";}
#endif

#endif // PATH_H
