#include "../include/graphics.h"
#include "../include/physics.h"
#include <iostream>

using namespace std;

const int WIN_HEIGHT = 1000;
const int WIN_WIDTH = 1000;

class Circle_Texture : public Texture {
public:
	Circle_Texture(unsigned radius, Color color = Color::BLUE) {
		c.calculate_points(Point(radius, radius), radius);

		create_blank(radius * 2 + 1, radius * 2 + 1, SDL_TEXTUREACCESS_TARGET);

		Renderer::set_render_target(this->texture());
		Renderer::set_draw_color(color);
		if (SDL_RenderDrawPoints(Renderer::renderer(), (SDL_Point*)c.data(), c.size()) != 0)
			cerr<<"ERROR: "<<__func__<<" SDL_RenderDrawPoints failed!\nSDL Error: "<<SDL_GetError()<<'\n';
		Renderer::reset_render_target();
	}

private:
	Circle c;
};

struct Sphere {
	Circle_Texture* ctex;
	Body body;
	Bounding_Circle shape;

	Sphere (Circle_Texture* texture, Vec2 top_left_position, Vec2 velocity = Vec2(), float inv_mass = 1.0f)
	{
		ctex = texture;

		body.pos = top_left_position + Vec2(ctex->width()/2, ctex->width()/2);
		body.velocity = velocity;
		body.gravity_scale = GRAVITY_DEFAULT;
		body.inv_mass = inv_mass;
		body.restitution = 1.0f;
		body.static_friction = 0.2f;
		body.dynamic_friction = 0.005f;

		body.shape = &shape;
		shape.body = &body;
		shape.rad = ctex->width()/2;
	}

	void render() {
		ctex->render(Point(body.pos.x - shape.rad, body.pos.y - shape.rad));
	}
};

struct Player : public Sphere, public Event_handler {
	Player (Circle_Texture* texture, Vec2 top_left_position, Vec2 velociy = Vec2(), float inv_mass = 1.0f) :
	       Sphere(texture, top_left_position, velociy, inv_mass),
	       Event_handler(SDL_KEYDOWN | SDL_KEYUP)
	{	
		body.gravity_scale = 0.0f;
		body.restitution = 0.3f;
		body.inv_mass = 0.5f;
	}

	void handle_event (const SDL_Event& e) {
		const Vec2 VEL2(50.0f, 0);
		const Vec2 VEL1(0, 50.0f);
		const Vec2 Zero(0, 0);

		if (e.key.repeat)
			return;
		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym) {
				case SDLK_UP :
					body.velocity = -VEL1;
					break;
				case SDLK_DOWN :
					body.velocity = VEL1;
					break;
				case SDLK_LEFT :
					body.velocity = -VEL2;
					break;
				case SDLK_RIGHT :
					body.velocity = VEL2;
					break;
			}
		}
		else if (e.type == SDL_KEYUP) {
			switch (e.key.keysym.sym) {
				case SDLK_UP :
					body.velocity = Zero;
					break;
				case SDLK_DOWN :
					body.velocity = Zero;
					break;
				case SDLK_LEFT :
					body.velocity = Zero;
					break;
				case SDLK_RIGHT :
					body.velocity = Zero;
					break;
			}
		}	
	}
};

class Rectangle_Texture : public Texture {
public:
	Rectangle_Texture(int w, int h) {
		SDL_Rect rect = { 0, 0, w, h };
		create_blank(w, h, SDL_TEXTUREACCESS_TARGET);

		Renderer::set_render_target(this->texture());
		Renderer::set_draw_color(Color::RED);
		if (SDL_RenderDrawRect(Renderer::renderer(), &rect) != 0)
			cerr<<"ERROR: "<<__func__<<" SDL_RenderDrawRect failed!\nSDL Error: "<<SDL_GetError()<<'\n';
		Renderer::reset_render_target();
	}
};

struct Slab {
	Rectangle_Texture* rtex;
	Body body;
	Bounding_Box shape;

	Slab (Rectangle_Texture* texture, Vec2 top_left_position, Vec2 velocity = Vec2(), float inv_mass = 0.0f)
	{
		rtex = texture;

		body.pos = top_left_position + Vec2(rtex->width()/2, rtex->height()/2);
		body.velocity = velocity;
		//body.gravity_scale = GRAVITY_DEFAULT;
		body.gravity_scale = 0.0f;
		body.inv_mass = inv_mass;
		body.restitution = 0.9f;
		body.static_friction = 0.1f;
		body.dynamic_friction = 0.001f;

		body.shape = &shape;
		shape.body = &body;
		shape.min = top_left_position;
		shape.max = top_left_position + Vec2(rtex->width(), rtex->height());
	}

	void render() {
		rtex->render(Point(body.pos.x - rtex->width()/2, body.pos.y - rtex->height()/2));
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

		Renderer::create_renderer(win);
		
		const int ITEM_SIZE = 9;
		const int SLAB_SIZE = 4;
		const int ISIZE = 20;
		const int SSIZE = 20;
		const int SPEED = 5;

		Circle_Texture* tex = new Circle_Texture(ISIZE);

		Sphere *item[ITEM_SIZE];
		for (int i = 0; i < ITEM_SIZE; ++i) {
			item[i] = new Sphere(tex, Vec2((i*ISIZE*3)%(WIN_WIDTH-SSIZE*4) + SSIZE*2, (i*ISIZE*3)/(WIN_WIDTH-SSIZE*4) * ISIZE*3 + SSIZE*2), Vec2(i%(SPEED*2) - SPEED, i%(SPEED*2) - SPEED));
		}

		Rectangle_Texture* rtex1 = new Rectangle_Texture(WIN_WIDTH, SSIZE);
		Rectangle_Texture* rtex2 = new Rectangle_Texture(SSIZE, WIN_HEIGHT - 2 * SSIZE);

		Slab *slab[SLAB_SIZE];
		slab[0] = new Slab(rtex1, Vec2(0, WIN_HEIGHT - SSIZE));
		slab[1] = new Slab(rtex1, Vec2(0, 0));
		slab[2] = new Slab(rtex2, Vec2(0, SSIZE));
		slab[3] = new Slab(rtex2, Vec2(WIN_WIDTH - SSIZE, SSIZE));

		Circle_Texture* ptex = new Circle_Texture(ISIZE, Color::GREEN);
		Player p(ptex, Vec2(WIN_WIDTH/2, WIN_HEIGHT/2));
		
		eman.add_handler(&p);

		Scene scene;

		for (int i = 0; i < ITEM_SIZE; ++i)
			scene.add_body(&(item[i]->body));
		for (int i = 0; i < SLAB_SIZE; ++i)
			scene.add_body(&(slab[i]->body));
		scene.add_body(&p.body);

		unsigned int frames = 0;

		const float fps = 60.0;
		const float dt = 1/fps;
		float accumulator = 0;

		unsigned int timer;
		unsigned int start_time = SDL_GetTicks();

		float frame_start = SDL_GetTicks();

		Renderer::set_draw_color(Color::BLACK);
		while (!win.quit()) {
			timer = SDL_GetTicks();
			eman.poll_handle();

			accumulator += timer - frame_start;
			frame_start = timer;

			if (accumulator > 0.2f)
				accumulator = 0.2f;
			while (accumulator > dt) {
				scene.step(dt);

				accumulator -= dt;
			}
			
			// render
			Renderer::clear_screen();
			for (int i = 0; i < ITEM_SIZE; ++i)
				item[i]->render();
			for (int i = 0; i < SLAB_SIZE; ++i)
				slab[i]->render();
			p.render();
			Renderer::render_screen();

			//cerr<<item[0]->body.velocity.x<<' '<<item[0]->body.velocity.y<<'\n';
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
	// deallocate memory?

	SDL_Quit();
	return 0;
}
