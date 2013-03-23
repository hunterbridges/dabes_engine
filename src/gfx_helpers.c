#include "gfx_helpers.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static Uint32 rmask = 0xff000000;
    static Uint32 gmask = 0x00ff0000;
    static Uint32 bmask = 0x0000ff00;
    static Uint32 amask = 0x000000ff;
#else
    static Uint32 rmask = 0x000000ff;
    static Uint32 gmask = 0x0000ff00;
    static Uint32 bmask = 0x00ff0000;
    static Uint32 amask = 0xff000000;
#endif

int init_GL(int swidth, int sheight) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
#ifndef DABES_IOS
    glEnable(GL_MULTISAMPLE);
#endif
    glDisable(GL_DEPTH_TEST);
    GLenum error = glGetError();
    check(error == GL_NO_ERROR, "OpenGL init error...");
    return 1;
error:
#ifndef DABES_IOS
    printf("Error initializing OpenGL! %s\n", gluErrorString(error));
#endif
    return 0;
}

#ifdef DABES_IOS
GLuint load_CGImage_as_texture(CGImageRef image) {
    check(image != NULL, "No CGImage to load");

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLenum color_format = GL_RGBA;
#if SDL_BYTEORDER != SDL_BIG_ENDIAN && !defined(DABES_IOS)
    color_format = GL_BGRA;
#endif
    int width = CGImageGetWidth(image);
    int height = CGImageGetHeight(image);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    unsigned char *rawData = calloc(1, height * width * 4);
    int bytesPerPixel = 4;
    int bytesPerRow = bytesPerPixel * width;
    int bitsPerComponent = 8;
    CGContextRef context =
        CGBitmapContextCreate(rawData, width, height, bitsPerComponent,
                              bytesPerRow, colorSpace,
                              kCGImageAlphaPremultipliedLast |
                              kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
                 height, 0, color_format,
                 GL_UNSIGNED_BYTE, rawData);
    CGContextRelease(context);
    free(rawData);
    CGImageRelease(image);
    return texture;
error:
    return 0;
}
#endif

GLuint load_surface_as_texture(SDL_Surface *surface) {
    check(surface != NULL, "No surface to load");

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLenum color_format = GL_RGBA;
#if SDL_BYTEORDER != SDL_BIG_ENDIAN && !defined(DABES_IOS)
    color_format = GL_BGRA;
#endif
#ifndef DABES_IOS
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0,
        color_format, GL_UNSIGNED_BYTE, surface->pixels);
    SDL_FreeSurface(surface);
    return texture;
error:
    return 0;
}

// TODO: Hashmap
GLuint load_image_as_texture(char *image_name) {
#ifdef DABES_IOS
    unsigned long int *data = NULL;
    GLint size = 0;
    read_file_data(image_name, &data, &size);
    CFDataRef cf_data = CFDataCreate(NULL, (Uint8 *)data, size);
    free(data);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(cf_data);
    CGImageRef cg_image =
         CGImageCreateWithPNGDataProvider(provider, NULL, true,
                                          kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);
    CFRelease(cf_data);
    return load_CGImage_as_texture(cg_image);
#else
    SDL_Surface *image = IMG_Load(image_name);
    return load_surface_as_texture(image);
#endif
    return 0;
}

SDL_Surface *gradient(unsigned int width, unsigned int height) {
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width,
            height, 32, rmask, gmask, bmask, amask);
    unsigned int i = 0;
    for (i = 0; i < height; i++) {
        int gradation = 255 * (height - i) / height;
        Uint32 color = SDL_MapRGBA(surface->format, 0,
                255 - gradation, gradation, 255);
        SDL_Rect rect = {0, i, width, 1};
        SDL_FillRect(surface, &rect, color);
    }
    return surface;
}

