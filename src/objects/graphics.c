#include "graphics.h"

void Graphics_draw_rect(void *self, SDL_Rect rect, GLfloat *color,
        GLuint texture, float rotation) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Point center = {
        rect.x + rect.w / 2,
        rect.y + rect.h / 2
    };
    glTranslatef(-SCREEN_WIDTH/2,-SCREEN_HEIGHT/2, 0.f );
    glTranslatef(center.x,center.y, 0.f );
    glRotatef(rotation,0,0,1);
    //printf("%f\n", rotation);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_TRIANGLE_STRIP);
        glColor3fv(color);
        glTexCoord2f(0, 0);
        glVertex2f(-rect.w / 2.0, -rect.h / 2.0);
        glTexCoord2f(1, 0);
        glVertex2f(rect.w / 2.0, -rect.h / 2.0);
        glTexCoord2f(0, 1);
        glVertex2f(-rect.w / 2.0, rect.h / 2.0);
        glTexCoord2f(1, 1);
        glVertex2f(rect.w / 2.0, rect.h / 2.0);
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
