#include "../include/graphics.h"
#include <iostream>

using namespace std;

int main() {
	if (!init()) {
		return 1;
	}
	try {
		Window win;
		win.init();

		win.pos(Point(0, 0));
		win.resize(1000, 1000);

		Renderer ren(win);

		Texture tex(ren.renderer());
		Texture tex2(ren.renderer());

		tex.load_from_file("../rsc/bk.png");
		tex2.set_color_key(Color(0xFF, 0xFF, 0xFF, 0x00));
		tex2.load_from_file("../rsc/tile.png");
		tex2.set_blend_mode(SDL_BLENDMODE_BLEND);

		ren.clear_screen();
		tex.render(Point(0,0));
		tex2.render(Point(10, 10));

		ren.render_screen();

		SDL_Delay(5000);
	}
	catch (Exception& e) {
		cerr<<"Exception : "<<e.what()<<'\n';;
	}

	SDL_Quit();
	return 0;
}

