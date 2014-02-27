#include "../include/graphics.h"
#include <iostream>

using namespace std;

class Ball : public Event_handler, public Texture {
public:
	Ball(Renderer& renderer, int x, int y, int w, int h) : Event_handler(SDL_KEYDOWN | SDL_KEYUP), Texture(renderer), vx(0), vy(0)
	{
		r.x = x;
		r.y = y;
		r.w = 10;
		r.h = 10;
		set_render_target(r.w, r.h);
		SDL_RenderFillRect(renderer.renderer(), &r);
		renderer.reset_render_target();
		r.w = w;
		r.h = h;
	}

	void handle_event(const SDL_Event& e) {
		if (e.key.repeat)
			return;
		if (e.type == SDL_KEYDOWN ) {
			switch( e.key.keysym.sym ) {
				case SDLK_UP: vy -= 5; break;
				case SDLK_DOWN: vy += 5; break;
				case SDLK_LEFT: vx -= 5; break;
				case SDLK_RIGHT: vx += 5; break;
			}
		}
		else if( e.type == SDL_KEYUP ) {
			switch( e.key.keysym.sym ) {
				case SDLK_UP: vy += 5; break;
				case SDLK_DOWN: vy -= 5; break;
				case SDLK_LEFT: vx += 5; break;
				case SDLK_RIGHT: vx -= 5; break;
			}
        	}
	}

	void move() {
		r.y = (r.y+vy) >= 0 ? (r.y+vy) % r.h : r.h;
		r.x = (r.x+vx) >= 0 ? (r.x+vx) % r.w : r.w;
	}

	void render() {
		Texture::render(Point(r.x, r.y));
	}
		
private:
	SDL_Rect r;
	int vx, vy;
};

int main() {
	if (!init()) {
		return 1;
	}
	try {
		Event_manager eman;

		Window win;
		win.init();
		eman.add_handler(&win);

		win.pos(Point(0, 0));
		win.resize(1000, 1000);

		Renderer ren(win);

		Texture tex(ren);
		Texture tex2(ren);
		Ball b(ren, 0, 0, 1000, 1000);
		eman.add_handler(&b);

		tex.load_from_file("../rsc/bk.png");
		tex2.set_color_key(Color(0xFF, 0xFF, 0xFF, 0x00));
		tex2.load_from_file("../rsc/tile.png");
		tex2.set_blend_mode(SDL_BLENDMODE_BLEND);


		while (!win.quit()) {
			eman.poll_handle();

			b.move();

			ren.clear_screen();
			tex.render(Point(0,0));
			tex2.render(Point(10, 10));
			b.render();
			ren.render_screen();
		}

	}
	catch (Exception& e) {
		cerr<<"Exception : "<<e.what()<<'\n';;
	}

	SDL_Quit();
	return 0;
}
