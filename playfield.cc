#include "playfield.hh"
#include "flshot.h"
#include <FL/fl_draw.H>


PlayField::PlayField(int x, int y, int w, int h, const char* title)
: Fl_Box(x,y,w,h,title)
{
	Fl::focus(this);
	Fl::add_timeout(1.0/30.5, &PlayField::static_callback, this);
}


void PlayField::static_callback(void* userdata)
{
	PlayField* pf = static_cast<PlayField*>(userdata);
	pf->callback();
	Fl::repeat_timeout(1.0/17.7, &PlayField::static_callback, userdata);
}


void PlayField::callback()
{
	make_current();
	
	fl_draw_box(FL_FLAT_BOX, 0, 0, 18, 18, Fl_Color(16) );
}


void PlayField::make_current()
{
	mainwin->make_current();
}


void PlayField::draw()
{
	Fl_Box::draw();
}
