#include "graphics.h"

GfxRect GfxRect_from_xywh(double x, double y, double w, double h) {
    GfxPoint tl = { x, y };
    GfxPoint tr = { x + w, y };
    GfxPoint bl = { x, y + h };
    GfxPoint br = { x + w, y + h };
    GfxRect rect = {
        .tl = tl,
        .tr = tr,
        .bl = bl,
        .br = br
    };
    return rect;
}

GfxRect GfxRect_from_SDL_Rect(SDL_Rect rect) {
    return GfxRect_from_xywh(rect.x, rect.y, rect.w, rect.h);
}

void Graphics_draw_rect(Graphics *graphics, GfxRect rect, GLdouble color[4],
        GLuint texture, double rotation) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    double w = rect.tr.x - rect.tl.x;
    double h = rect.bl.y - rect.tl.y;

    GfxPoint center = {
        rect.tl.x + w / 2,
        rect.tl.y + h / 2
    };
    glTranslatef(-SCREEN_WIDTH/2,-SCREEN_HEIGHT/2, 0.f );
    glTranslatef(center.x,center.y, 0.f );
    glRotatef(rotation,0,0,1);
    //printf("%f\n", rotation);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_STRIP);
        glColor4fv(color);
        glTexCoord2f(0, 0);
        glVertex2f(-w / 2.0, -h / 2.0);
        glTexCoord2f(1, 0);
        glVertex2f(w / 2.0, -h / 2.0);
        glTexCoord2f(0, 1);
        glVertex2f(-w / 2.0, h / 2.0);
        glTexCoord2f(1, 1);
        glVertex2f(w / 2.0, h / 2.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

int Graphics_init(void *self) {
    Graphics *graphics = self;
    return 1;
};

Object GraphicsProto = {
    .init = Graphics_init
};
