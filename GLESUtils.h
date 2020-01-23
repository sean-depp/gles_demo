//
// Created by sean on 2020/1/20.
//

#ifndef GLES_DEMO_GLESUTILS_H
#define GLES_DEMO_GLESUTILS_H

#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <cstdio>
#include <GLES3/gl32.h>
#include <string>
#include <vector>

class GLESUtils {
public:
    bool testEGLError(const char *functionLastCalled);

    bool testGLError(const char *functionLastCalled);

    bool createNativeDisplay();

    bool createNativeWindow();

    bool createEGLDisplay();

    bool chooseEGLConfig();

    bool createEGLSurface();

    bool setupEGLContext();

    void setWindowWH(unsigned int width, unsigned int height);

    unsigned int getWindowWidth();

    unsigned int getWindowHeight();

    void setAppName(std::string appName);

    void releaseEGLState(EGLDisplay eglDisplay);

    void releaseNativeResources(Display *nativeDisplay, Window nativeWindow);

    void deInitGLState();

    GLuint getTextureID();

    void setTextureID(GLuint tid);

    std::vector<GLuint> getVectorTextureID(const int size);

    void setVectorTextureID(std::vector<GLuint> vectorTid);

    void setSamplerLoc(GLint sl);

    GLint getSamplerLoc();

    Display *getNativeDisplay();

    EGLDisplay &getEglDisplay();

    EGLConfig &getEglConfig();

    Window getNativeWindow();

    EGLSurface getEglSurface();

    EGLContext getContext();

    GLuint loadTexture(std::string fileName);

    std::vector<GLuint> loadMoreTexture(std::vector<std::string> fileNames);

    bool renderScene();

    bool initShaders();

    std::string readShader(std::string path);

    int getTextureSize();

    void setTextureSize(int i);

    bool createShader(std::string shaderPath, int i);

    GLuint &getFragmentShader();

    GLuint &getVertexShader();

    GLuint getShaderProgram();

    void setFragmentShader(GLuint shader);

    void setVectorShader(GLuint shader);

    void cleanProc();

    void initNativeAndEGL();

private:
    // Width and height of the window
    unsigned int _winWidth;
    unsigned int _winHeight;
    // Name of the application
    char *_appName;

    int _textureSize;
    GLuint _textureID;
    std::vector<GLuint> _vectorTextureID;
    GLint _samplerLoc;
    GLuint _fragmentShader = 0, _vertexShader = 0;
    GLuint _shaderProgram = 0;

    // X11 variables
    Display *_nativeDisplay = NULL;
    Window _nativeWindow = 0;

    // EGL variables
    EGLDisplay _eglDisplay = NULL;
    EGLConfig _eglConfig = NULL;
    EGLSurface _eglSurface = NULL;
    EGLContext _context = NULL;

};


#endif //GLES_DEMO_GLESUTILS_H
