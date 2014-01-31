#ifndef TRANS_GRAPHICS_HEAD
#define TRANS_GRAPHICS_HEAD

#include <iostream>	// for error reporting to 'cerr'
#include <string>	// for std::string
#include <vector>	// for std::vector

#include <SDL.h>
#include <SDL_image.h>

using namespace std;

const int DEFAULT_WIDTH = 640;
const int DEFAULT_HEIGHT = 480;

const int WINDOW_FLAGS = SDL_WINDOW_SHOWN;	// used by SDL_CreateWindow
const int RENDERER_FLAGS = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;	// used by SDL_CreateRenderer
const int INIT_FLAGS = SDL_INIT_VIDEO;		// used by SDL_Init
const int IMG_FLAGS = IMG_INIT_PNG | IMG_INIT_JPG;	//used by IMG_Init

/*	Point : wrapper for SDL_Point	*/
struct Point {
	Point() {
		point.x = 0;
		point.y = 0;
	}
	Point(int x, int y) {
		point.x = x;
		point.y = y;
	}

	int x() const {
		return point.x;
	}

	int y() const {
		return point.y;
	}

	SDL_Point point;	// SDL_Point { int x, int y };
};

/*	Exception : Base class of all the exceptions	*/
class Exception {
public:
	Exception() : msg("Unknown Exception") {	}

	Exception(string str) : msg(str) {	}

	virtual string what() const {		// called after 'catch'
		return msg;
	}

protected:
	string msg;	// the message displayed by what()
};

/*	Color : wrapper for SDL_Color	*/
class Color {
public:
	Color()			{	*this = Color(0, 0, 0);	}
	Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a = SDL_ALPHA_OPAQUE) {		// alpha defaults to 0xFF (opaque)
		col.r = r;
		col.g = g;
		col.b = b;
		col.a = a;
	}

	// the getters
	Uint8 red() const	{	return col.r;		}
	Uint8 green() const	{	return col.g;		}
	Uint8 blue() const	{	return col.b;		}
	Uint8 alpha() const	{	return col.a;		}

	// the setters
	void red(Uint8 r)	{	col.r = r;		}
	void green(Uint8 g)	{	col.g = g;		}
	void blue(Uint8 b)	{	col.b = b;		}
	void alpha(Uint8 a)	{	col.a = a;		}

	SDL_Color* color()	{	return &col;		}

	static Color BLACK,
		     WHITE,
		     RED,
		     GREEN,
		     BLUE;


private:
	SDL_Color col;
};

/*	Event_handler : An abstract event-handler class. All event-handlers must subclass it	*/
class Event_handler {
public:
	// handle the event passed (NOTE: keep the function small as it is called for every event received by Event_manager)
	virtual void handle_event(const SDL_Event&)	{	}

	// returns the event type the handler is interested in
	virtual Uint32 event_type() const		{	return 0;	}
};

/*	Event_manager : wrapper for SDL_Event. Combined with 'Event_handler' provides a neat way of handling events	*/
class Event_manager {
public:
	Event_manager();

	~Event_manager();

	void add_handler(Event_handler&);
	void remove_handler(Event_handler&);

	void poll_handle();
	
	//bool add_source();
		Bad_Window(string m) : Exception(m) {	}
		string what() const {	return "Bad_Window: "+Exception::what();	}
	};

	Window();
	Window(const string& title);

	~Window();	// calls 'free()'

	void init();	// 'SDL_CreateWindow()' wrapper. Here the actual window is created.
	void free();	// deletes the 'SDL_Window'

	SDL_Window* window() const	{	return win;					}

	// 'Window' position getter and setter
	Point pos() const		{	return Point(wrect.x, wrect.y);			}
	void pos(Point p);

	// 'Window' width and height getters
	int width() const		{	return wrect.w;					}
	int height() const		{	return wrect.h;					}

	// 'Window' size setter (WARNING: call it before creating a Renderer)
	void resize(int w, int h);

	// 'Window' title getter and setter
	const string& title() const	{	return wtitle;					}
	void title(const string& s)	{	wtitle = s.length() ? s : "Unnamed Window"; /* do not accept empty title */}

	// returns true if the 'Window' is minimized or is not visible
	bool is_hidden() const		{	return whidden;					}

	// returns true if user pressed 'x' button of window
	bool quit() const		{	return wquit;					}	

	void handle_event(const SDL_Event& event);	// event handler for window

	// returns the interested events' types
	Uint32 event_type() const	{	return SDL_WINDOWEVENT;				}
private:
	SDL_Window* win; // pointer to SDL_Window
	string wtitle;	// title of 'Window' displayed by Window Manager
	SDL_Rect wrect; // dimensions of 'Window'; SDL_Rect { int x, int y, int w, int h }
	bool whidden;	// hidden flag for 'Window' (useful for preventing offscreen rendering and resultant slowdown)
	bool wquit;	// quit flag; gets 'true' after receiving QUIT event
};

/*	Renderer : wrapper for SDL_Renderer	*/
class Renderer {
public:
	class Bad_Renderer : public Exception {	// prefixes 'what()' with "Bad_Renderer: "
	public:
		Bad_Renderer(string m) : Exception(m) {	}
		string what() const {	return "Bad_Renderer: "+Exception::what();	}
	};
	
	Renderer(Window& win);	// SDL_CreateRenderer is called for creating Renderer for 'Window'
	
	~Renderer();	// calls 'free()'

	void free();	// deletes the 'SDL_Renderer'

	SDL_Renderer* renderer()	{	return ren;	}

	void render_screen();	// draw the scene to screen
	void clear_screen();	// clear screen with current color
	
	// get the current draw color
	Color draw_color() const	{	return rcolor;		}
	void draw_color(Color);		// set the current draw color (NOTE : draw_color is used for drawing operations only (line, rect, clear_screen)

private:
	SDL_Renderer* ren;	// SDL's renderer
	Color rcolor;		// the draw_color of renderer
};

/*	Texture : wrapper for SDL_Texture	*/
class Texture {
public:
	class Bad_Texture: public Exception {	// prefixes what() with "Bad_Texture"
	public:
		Bad_Texture(string m) : Exception(m) {	}
		string what() const {	return "Bad_Texture: "+Exception::what();	}
	};

	Texture(SDL_Renderer*);		// initialize and set default values

	~Texture();	// calls free()

	void free();	// destroy the SDL_Texture

	void create_blank(int w, int h, SDL_TextureAccess = SDL_TEXTUREACCESS_STATIC);	// create a blank texture with default static access

	void load_from_file(string path);	// create SDL_Texture using {png, jpeg} files

	void set_color_key(Color);	// set the color key (transparent pixel) (NOTE: call it before creating texture i.e. load_from_file)

	void set_blend_mode(SDL_BlendMode);	// set the blending mode (default : none)

	void set_alpha(Uint8 alpha);	// set alpha of the texture

	// render the texture (actual rendering is done by render_screen() call of 'Renderer'
	// clip is used for clipping the Texture, angle of rotation's sematics are same as that of trig, SDL_RendererFlip { SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL }
	void render(const Point pos, const SDL_Rect* clip = nullptr, double angle = 0.0, const Point* center = nullptr, SDL_RendererFlip = SDL_FLIP_NONE);

	// set this texture as render target i.e. all the render() calls draw on this texture
	void set_render_target();

	// width, height getters
	int width() const	{	return tw;	}
	int height() const	{	return th;	}

private:
	SDL_Texture* tex;
	SDL_Renderer* ren;
	Color color_key;	// transparent color of texture

	int tw, th;
};

bool init();	// initializes the SDL system

#endif
