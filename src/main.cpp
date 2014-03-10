#include "../include/graphics.h"
#include "../include/physics.h"
#include <iostream>

using namespace std;

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

class Rectangle_Texture : public Texture {
public:
	Rectangle_Texture(int w, int h) {
		SDL_Rect rect = { 0, 0, w, h };
		create_blank(w, h, SDL_TEXTUREACCESS_TARGET);

		Renderer::set_render_target(this->texture());
		Renderer::set_draw_color(Color::RED);
		if (SDL_RenderFillRect(Renderer::renderer(), &rect) != 0)
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

struct Player : public Slab, public Event_handler {
	Player (Rectangle_Texture* texture, Vec2 top_left_position, Vec2 velociy = Vec2(), float inv_mass = 1.0f) :
	       Slab(texture, top_left_position, velociy, inv_mass),
	       Event_handler(SDL_KEYDOWN | SDL_KEYUP)
	{	
		body.gravity_scale = GRAVITY_DEFAULT;
		body.restitution = 0.2f;
		body.inv_mass = 0.1f;

		body.static_friction = 0.9f;
		body.dynamic_friction = 0.5f;
	}

	void handle_event (const SDL_Event& e) {
		Vec2 VEL2(30.0f, 0);
		Vec2 VEL1(0, 10.0f);
		const Vec2 F2(1000.0f, 0);
		const Vec2 F1(0, 40000.0f);
		const Vec2 Zero(0, 0);

		VEL1 = Zero;
		//VEL2 = Zero;

		//if (e.key.repeat) return;

		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym) {
				case SDLK_UP :
					if (abs(body.velocity.y) <= 0.0f + VELOCITY_EPSILON * 2) {
						body.velocity -= VEL1;
						body.force -= F1;
					}
					break;
					/*
				case SDLK_DOWN :
					body.velocity += VEL1;
					body.force += F1;
					break;
					*/
				case SDLK_LEFT :
					if (-(VEL2.x) < body.velocity.x) {
						body.velocity -= (body.velocity + VEL2);
						body.force -= F2;
					}
					break;
				case SDLK_RIGHT :
					if (body.velocity.x < (VEL2.x)) {
						body.velocity += (VEL2 - body.velocity);
						body.force += F2;
					}
					break;
			}
		}
		/*
		else if (e.type == SDL_KEYUP) {
			switch (e.key.keysym.sym) {
				case SDLK_UP :
					body.velocity += VEL1;
					break;
				case SDLK_DOWN :
					body.velocity -= VEL1;
					break;
				case SDLK_LEFT :
					body.velocity += VEL2;
					break;
				case SDLK_RIGHT :
					body.velocity -= VEL2;
					break;
			}
		}	
		*/
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

		const int WIN_HEIGHT = 1000;
		const int WIN_WIDTH = 1800;

		win.resize(WIN_WIDTH, WIN_HEIGHT);

		Renderer::create_renderer(win);
		
		const int ITEM_SIZE = 9;
		const int ISIZE = 20;
		const int SSIZE = 50;
		const int SPEED = 10;
		const int SLICE = 5;

		Circle_Texture* tex = new Circle_Texture(ISIZE);

		Sphere *item[ITEM_SIZE];
		for (int i = 0; i < ITEM_SIZE; ++i) {
			item[i] = new Sphere(tex, Vec2((i*ISIZE*3)%(WIN_WIDTH-SSIZE*4) + SSIZE*2, (i*ISIZE*3)/(WIN_WIDTH-SSIZE*4) * ISIZE*3 + SSIZE*2), Vec2(i%(SPEED*2) - SPEED, i%(SPEED*2) - SPEED));
		}

		Rectangle_Texture* rtex1 = new Rectangle_Texture(WIN_WIDTH, SSIZE);
		Rectangle_Texture* rtex2 = new Rectangle_Texture(SSIZE, WIN_HEIGHT - 2 * SSIZE);
		Rectangle_Texture* rtex3 = new Rectangle_Texture(WIN_WIDTH / 2, SSIZE);
		Rectangle_Texture* rtex4 = new Rectangle_Texture(WIN_WIDTH / 4, SLICE);

		const int SLAB_SIZE = 12;
		Slab *slab[SLAB_SIZE];
		slab[0] = new Slab(rtex1, Vec2(0, WIN_HEIGHT - SSIZE));
		slab[1] = new Slab(rtex1, Vec2(0, 0));
		slab[2] = new Slab(rtex2, Vec2(0, SSIZE));
		slab[3] = new Slab(rtex2, Vec2(WIN_WIDTH - SSIZE, SSIZE));
		slab[4] = new Slab(rtex3, Vec2(WIN_WIDTH / 2 - SSIZE, WIN_HEIGHT / 2 + SSIZE * 2));
		slab[5] = new Slab(rtex3, Vec2(SSIZE, WIN_HEIGHT / 2 - SSIZE * 2));
		slab[6] = new Slab(rtex4, Vec2(SSIZE, WIN_HEIGHT - SSIZE - SLICE * 6));
		slab[7] = new Slab(rtex4, Vec2(SSIZE + SLICE, WIN_HEIGHT - SSIZE - SLICE * 5));
		slab[8] = new Slab(rtex4, Vec2(SSIZE + SLICE * 2, WIN_HEIGHT - SSIZE - SLICE * 4));
		slab[9] = new Slab(rtex4, Vec2(SSIZE + SLICE * 3, WIN_HEIGHT - SSIZE - SLICE * 3));
		slab[10] = new Slab(rtex4, Vec2(SSIZE + SLICE * 4, WIN_HEIGHT - SSIZE - SLICE * 2));
		slab[11] = new Slab(rtex4, Vec2(SSIZE + SLICE * 5, WIN_HEIGHT - SSIZE - SLICE));

		//Circle_Texture* ptex = new Circle_Texture(ISIZE * 2, Color::GREEN);
		Rectangle_Texture* ptex = new Rectangle_Texture(ISIZE * 2, ISIZE * 3);
		Player p(ptex, Vec2(WIN_WIDTH/2 , WIN_HEIGHT/2 - ISIZE));
		
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

			if (win.is_hidden())
				continue;

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

			cerr<<p.body.velocity.x<<' '<<p.body.velocity.y<<'\n';
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
