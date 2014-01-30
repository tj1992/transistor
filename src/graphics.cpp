#include "../include/graphics.h"

//////////////////////////////////////////////////////////////////////
//	Color
//////////////////////////////////////////////////////////////////////

// some basic Colors
Color Color::BLACK = Color(0, 0, 0);
Color Color::WHITE = Color(0xFF, 0xFF, 0xFF);
Color Color::RED = Color(0xFF, 0, 0);
Color Color::GREEN = Color(0, 0xFF, 0);
Color Color::BLUE = Color(0, 0, 0xFF);

//////////////////////////////////////////////////////////////////////
//	class Window
//////////////////////////////////////////////////////////////////////


//	default constructor for 'Window'. Just sets the different params to default values
Window::Window() {
	*this = Window("Unnamed Window");
}

//	Window : sets the title to string 's'
Window::Window(const string& s) :
	win(nullptr),
	whidden(true)		// 'Window' is hidden
{
	wrect.x = SDL_WINDOWPOS_UNDEFINED;
	wrect.y = SDL_WINDOWPOS_UNDEFINED;
	wrect.w = DEFAULT_WIDTH;
	wrect.h = DEFAULT_HEIGHT;
	title(s);	// sets the title of current window to 's' (defaults to 'Unnamed Window')
}

Window::~Window() {
	free();		// delete the SDL_Window
}

//	Window::init : SDL_Window is created and position and size is set
void Window::init() {
	win = SDL_CreateWindow(wtitle.c_str(), wrect.x, wrect.y, wrect.w, wrect.h, WINDOW_FLAGS);
	if (win == nullptr)
		throw Bad_Window(__func__ + string(": SDL_CreateWindow failed.\nSDL Error: ") + SDL_GetError());

	whidden = false;	// Window is visible now
	SDL_GetWindowPosition(win, &wrect.x, &wrect.y);
	SDL_GetWindowSize(win, &wrect.w, &wrect.h);
}

//	Window::free : destroy the SDL_Window and reset the params
void Window::free() {
	if (win != nullptr) {
		SDL_DestroyWindow(win);
	}

	wrect.w = wrect.h = 0;
	title("");	// reset title
	whidden = true;
}

//	Window::pos : set the Window position to Point p (global coordinates are used)
void Window::pos(Point p) {
	wrect.x = p.x() >= 0 ? p.x() : SDL_WINDOWPOS_CENTERED;	// set to CENTERED if -ve
	wrect.y = p.y() >= 0 ? p.y() : SDL_WINDOWPOS_CENTERED;

	SDL_SetWindowPosition(win, wrect.x, wrect.y);	// update position
}

//	Window::resize : change the Window's width and height
void Window::resize(int w, int h) {
	wrect.w = w > 0 ? w : DEFAULT_WIDTH;	// do not accept -ve width
	wrect.h = h > 0 ? h : DEFAULT_HEIGHT;	// do not accept -ve height

	SDL_SetWindowSize(win, wrect.w, wrect.h);	// upadate size
}

//////////////////////////////////////////////////////////////////////
//	class Renderer
//////////////////////////////////////////////////////////////////////

//	Renderer : create the SDL_Renderer
Renderer::Renderer(Window& win) :
	rcolor(Color())
{
	ren = SDL_CreateRenderer(win.window(), -1, RENDERER_FLAGS);	// -1 for specifying default rendering driver
	if (ren == nullptr)
		throw Bad_Renderer(__func__ + string(": SDL_CreateRenderer failed.\nSDL Error: ") + SDL_GetError());

	if (SDL_GetRenderDrawColor(ren, &rcolor.color()->r, &rcolor.color()->g, &rcolor.color()->b, &rcolor.color()->a) != 0) {
		cerr<<"WARNING: "<<__func__<<": SDL_GetRenderDrawColor failed!\nSDL Error: "<<SDL_GetError()<<'\n';
		rcolor = Color::BLACK;
	}
}

//	~Renderer : delete the SDL_Renderer
Renderer::~Renderer() {
	free();
}

//	Renderer::free : destroy the SDL_Renderer
void Renderer::free() {
	if (ren != nullptr)
		SDL_DestroyRenderer(ren);
}

//	Renderer::render_screen : render the current scene
//	NOTE : Use this function after all calls to 'render' have been made. Actual drawing takes place here.
void Renderer::render_screen() {
	SDL_RenderPresent(ren);
}

//	Renderer::clear_screen : clear the screen to draw color
void Renderer::clear_screen() {
	SDL_RenderClear(ren);
}

//	Renderer::draw_color : set the current draw color for renderer
void Renderer::draw_color(Color c) {
	if (SDL_SetRenderDrawColor(ren, c.red(), c.green(), c.blue(), c.alpha()) != 0) {		// Set the draw color
		cerr<<"WARNING: "<<__func__<<": SDL_SetRenderDrawColor failed!\nSDL Error: "<<SDL_GetError()<<'\n';	// rcolor unchanged!
	}
	else {
		rcolor = c;		// update the rcolor value
	}
}

//////////////////////////////////////////////////////////////////////
//	class Texture
//////////////////////////////////////////////////////////////////////

//	Texture : sets the defaults
Texture::Texture(SDL_Renderer* renderer) :
	tex(nullptr),
	color_key(Color::BLACK),
	tw(0),
	th(0)
{
	ren = renderer;
}

//	~Texture : delete the Texture
Texture::~Texture() {
	free();
}

//	Texture::free : destroy the SDL_Texture
void Texture::free() {
	if (tex != nullptr) {
		SDL_DestroyTexture(tex);
		tw = 0;
		th = 0;
		color_key = Color::BLACK;
	}
}

//	Texture::create_blank : create a blank texture
void Texture::create_blank(int w, int h, SDL_TextureAccess access) {
	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, access, w, h);	// create the texture
	if (tex == nullptr) {
		throw Bad_Texture(__func__ + string(": SDL_CreateTexture failed!\nSDL Error: ") + SDL_GetError());
	}
	else {
		tw = w;
		th = h;
	}
}

//	Texture::load_from_file : load the texture from 'path'
void Texture::load_from_file(string path) {
	free();		// free the current texture

	SDL_Texture* ntex = nullptr;	// the new texture
	SDL_Surface* surf = IMG_Load(path.c_str());	// get the SDL_Surface for the image
	if (surf == nullptr) {
		throw Bad_Texture(__func__ + string(": unable to load image: "+path+"\nIMG Error: ") + IMG_GetError());
	}
	
	SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, color_key.red(), color_key.green(), color_key.blue()));	// set the color key for surface

	ntex = SDL_CreateTextureFromSurface(ren, surf);		//convert the surface to texture
	if (ntex == nullptr) {
		throw Bad_Texture(__func__ + string(": unable to convert surface to texture!\nSDL Error: ") + SDL_GetError());
	}

	// get the width and height of the texture
	tw = surf->w;
	th = surf->h;
	
	SDL_FreeSurface(surf);		// free the surface

	tex = ntex;	// set the new texture
}

//	Texture::set_color_key : set the transparent pixel of the Texture
void Texture::set_color_key(Color new_color) {
	color_key = new_color;
}

//	Texture::set_blend_mode : set the blending mode of Texture
void Texture::set_blend_mode(SDL_BlendMode blend) {
	SDL_SetTextureBlendMode(tex, blend);
}

//	Texture::set_alpha : set the alpha of Texture (used in blending ops)
void Texture::set_alpha(Uint8 alpha) {
	SDL_SetTextureAlphaMod(tex, alpha);
}

//	Texture::render : render the texture
void Texture::render(const Point p, const SDL_Rect* clip, double angle, const Point* center, SDL_RendererFlip flip) {
	SDL_Rect ren_quad = { p.x(), p.y(), tw, th };		// the render quad of the texture (position in the current renderer's coordinates)

	if (clip != nullptr) {
		ren_quad.w = clip->w;	// clip the Texure
		ren_quad.h = clip->h;
	}

	if (SDL_RenderCopyEx(ren, tex, clip, &ren_quad, angle, &center->point, flip) != 0) {		//	clip provides source clipping while ren_quad provide destination clipping
		throw Bad_Texture(__func__ + string(": SDL_RenderCopyEx failed\nSDL error: ") + SDL_GetError());
	}
}

//	Texture::set_render_target : set the texture as rendering target
void Texture::set_render_target() {
	free();		// free the current Texture

	create_blank(tw, th, SDL_TEXTUREACCESS_TARGET);		// create a blank target texture
	if (SDL_SetRenderTarget(ren, tex) != 0)		// set as target
		throw Bad_Texture(__func__ + string(": SDL_SetRenderTarget failed!\nSDL Error: ") + SDL_GetError());
}

//	init : initializes the SDL systems
bool init() {
	bool ret = true;

	if (SDL_Init(INIT_FLAGS) < 0) {		// initialize SDL system
		cerr<<"FATAL: Could not initialize SDL!\nSDL error: "<< SDL_GetError()<<'\n';
		ret = false;
	}
	else {
		if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1")) {		// set VSYNC
			cerr<<"WARNING: VSync not enabled!\n";
		}

		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {		// set linear texture filtering
			cerr<<"WARNING: Linear texture filtering not enabled!\n";
		}
		
		if (!(IMG_Init(IMG_FLAGS) & IMG_FLAGS)) {	// initialize SDL_Image subsystem
			cerr<<"FATAL: Could not initialize SDL_image!\nSDL_ error: "<<SDL_GetError()<<'\n';
			ret = false;
		}
	}

	return ret;
}
