#include "../include/graphics.h"
#include "physics.cpp"
#include <iostream>

using namespace std;
const int WIN_HEIGHT = 1000;
const int WIN_WIDTH = 1000;

class Solid_Box : public Box, public Texture {
public:
	Solid_Box(Renderer& renderer, int x, int y, int w, int h, Vec2 vel) : Box(Vec2(x, y), Vec2(x+w, y+h)), Texture(renderer)
	{
		velocity = vel;
		SDL_Rect r = { 0, 0, w, h };
		set_render_target(w, h);
		if (vel.x != 0)
			SDL_SetRenderDrawColor(renderer.renderer(), 255, 0, 0, 0);
		SDL_RenderFillRect(renderer.renderer(), &r);
		renderer.reset_render_target();
	}

	void render() {
		Texture::render(Point(pos.x-width()/2, pos.y-height()/2));
	}
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
		win.resize(WIN_WIDTH, WIN_HEIGHT);

		Renderer ren(win);
		
		const int ITEM_SIZE = 106;
		const int SIZE = 15;
		const int SPEED = 70;

		Solid_Box *item[ITEM_SIZE];
		item[0] = new Solid_Box(ren, 0, 0, 10, 1000, Vec2(0,0));
		item[1] = new Solid_Box(ren, 10, 0, 980, 10, Vec2(0,0));
		item[2] = new Solid_Box(ren, 990, 0, 10, 1000, Vec2(0,0));
		item[3] = new Solid_Box(ren, 10, 990, 980, 10, Vec2(0,0));
		for (int i = 4; i < ITEM_SIZE; ++i) {
			item[i] = new Solid_Box(ren, CLAMP(20, WIN_WIDTH-20, ((i-3)*(2*SIZE))%WIN_WIDTH), CLAMP(20, WIN_HEIGHT-20, (i-3)*(2*SIZE)/WIN_WIDTH), SIZE, SIZE, Vec2(i%SPEED-SPEED/2, i%SPEED-SPEED/2));
			item[i]->inv_mass = 0.9f;
			item[i]->restitution = 1.0f;
		}
		unsigned int frames = 0;

		const float fps = 60.0;
		const float dt = 1/fps;
		float accumulator = 0;

		unsigned int timer;
		unsigned int start_time = SDL_GetTicks();

		float frame_start = SDL_GetTicks();

		SDL_SetRenderDrawColor(ren.renderer(), 0, 0, 0, 0);
		while (!win.quit()) {
			timer = SDL_GetTicks();
			eman.poll_handle();

			accumulator += timer - frame_start;
			frame_start = timer;

			if (accumulator > 0.2f)
				accumulator = 0.2f;
			while (accumulator > dt) {
				// update physics
				for (int i = 4; i < ITEM_SIZE; ++i)
					item[i]->move(dt);

				Manifold m;
				for (int i = 4; i < ITEM_SIZE; ++i) {
					m.a = item[i];
					for (int j = 0; j < ITEM_SIZE; ++j) {
						if (i == j)
							continue;
						m.b = item[j];
						if (collision_box_box(m)) {	resolve_collision(m); positional_correction(m);	}
					}
				}

				accumulator -= dt;
			}
			
			// render
			ren.clear_screen();
			for (int i = 0; i < ITEM_SIZE; ++i)
				item[i]->render();
			ren.render_screen();

			// stats
			/*
			++frames;
			timer = SDL_GetTicks() - timer;
			cerr<<1000.0f/float(timer)<<'\n';	// print current fps
			*/

		}
		/*
		timer = SDL_GetTicks()-start_time;
		cout<<"Frames : "<<frames<<"\nTime Taken : "<<timer/1000.0f<<"\nFPS : "<<frames/(timer/1000.0f)<<'\n';
		*/
		/*
		for (int i = 0; i < ITEM_SIZE; ++i)
			cerr<<"vel-x : "<<item[i]->velocity.x<<" vel-y : "<<item[i]->velocity.y<<'\n';
		*/
	}
	catch (Exception& e) {
		cerr<<"Exception : "<<e.what()<<'\n';;
	}

	SDL_Quit();
	return 0;
}
