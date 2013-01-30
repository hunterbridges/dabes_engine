#include "graphics.h"

void Graphics_draw_rect(void *self, SDL_Rect rect, GLfloat *color,
        GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_TRIANGLE_STRIP);
        glColor3fv(color);
        glTexCoord2f(0, 0);
        glVertex2f(rect.x, rect.y);
        glTexCoord2f(1, 0);
        glVertex2f(rect.x + rect.w, rect.y);
        glTexCoord2f(0, 1);
        glVertex2f(rect.x, rect.y + rect.h);
        glTexCoord2f(1, 1);
        glVertex2f(rect.x + rect.w, rect.y + rect.h);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

int Graphics_init(void *self) {
    Graphics *graphics = self;
    graphics->draw_rect = Graphics_draw_rect;
    return 1;
};

Object GraphicsProto = {
    .init = Graphics_init
};
