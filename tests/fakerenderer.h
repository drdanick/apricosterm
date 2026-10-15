#ifndef FAKE_RENDERER_H
#define FAKE_RENDERER_H

/* The calls made to the renderer since the last reset, separated by spaces */
const char* fakeRendererLog();

void fakeRendererReset();

#endif /* FAKE_RENDERER_H */
