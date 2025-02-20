/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <emscripten.h>

int mouse_motions = 0;

#define abs(x) ((x) < 0 ? -(x) : (x))
#define eq(x, y) (abs((x) - (y)) <= 1)

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_MOUSEMOTION: {
        SDL_MouseMotionEvent *m = (SDL_MouseMotionEvent*)&event;
        assert(m->state == 0);
        printf("motion : %d,%d  %d,%d\n", m->x, m->y, m->xrel, m->yrel);

        if (mouse_motions == 0) {
#ifdef TEST_SDL_MOUSE_OFFSETS
          assert(eq(m->x, 5) && eq(m->y, 15) && eq(m->xrel, 5) && eq(m->yrel, 15));
#else
          assert(eq(m->x, 10) && eq(m->y, 20) && eq(m->xrel, 10) && eq(m->yrel, 20));
#endif
        } else if (mouse_motions == 1) {
#ifdef TEST_SDL_MOUSE_OFFSETS
          assert(eq(m->x, 25) && eq(m->y, 65) && eq(m->xrel, 20) && eq(m->yrel, 50));
#else
          assert(eq(m->x, 30) && eq(m->y, 70) && eq(m->xrel, 20) && eq(m->yrel, 50));
#endif
        }

        mouse_motions++;
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        if (m->button == 2) {
          emscripten_force_exit(0);
        }
        printf("button down : %d,%d  %d,%d\n", m->button, m->state, m->x, m->y);
#ifdef TEST_SDL_MOUSE_OFFSETS
        assert(eq(m->button, 1) && eq(m->state, 1) && eq(m->x, 5) && eq(m->y, 15));
#else
        assert(eq(m->button, 1) && eq(m->state, 1) && eq(m->x, 10) && eq(m->y, 20));
#endif
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        printf("button up : %d,%d  %d,%d\n", m->button, m->state, m->x, m->y);
#ifdef TEST_SDL_MOUSE_OFFSETS
        assert(eq(m->button, 1) && eq(m->state, 0) && eq(m->x, 5) && eq(m->y, 15));
#else
        assert(eq(m->button, 1) && eq(m->state, 0) && eq(m->x, 10) && eq(m->y, 20));
#endif
        // Remove another click we want to ignore
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN) == 1);
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP) == 1);
        break;
      }
    }
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_CreateWindowAndRenderer(600, 450, 0, &window, &renderer);

  SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF );
  SDL_Rect rect = { 0, 0, 600, 450 };
  SDL_RenderFillRect(renderer, &rect);

  emscripten_run_script("simulateMouseEvent(10, 20, -1)"); // move from 0,0 to 10,20
  emscripten_run_script("simulateMouseEvent(10, 20, 0)"); // click
  emscripten_run_script("simulateMouseEvent(10, 20, 0)"); // click some more, but this one should be ignored through PeepEvent
  emscripten_run_script("simulateMouseEvent(30, 70, -1)"); // move some more
  emscripten_run_script("simulateMouseEvent(30, 70, 1)"); // trigger the end

  emscripten_set_main_loop(one, 0, 0);
}
