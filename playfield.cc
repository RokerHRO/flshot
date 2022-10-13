#include "playfield.hh"
#include "flshot.h"
#include <FL/fl_draw.H>
#include <FL/Fl_PNG_Image.H>

#include <algorithm>
#include <cmath>
#include <cstdlib> // for drand48()
#include <unistd.h> // for getpid()
#include <string>


XY<int> Sprite_Min, Sprite_Max;


static Fl_Image* geist_lila = nullptr;
static Fl_Image* geist_rot  = nullptr;
static Fl_Image* geist_gelb = nullptr;
static Fl_Image* geist_eis  = nullptr;

static Fl_Image* geist_schatten = nullptr;
static Fl_Image* cannon_png = nullptr;


Fl_Image* generateCannon(double angle)
{
	const int W = cannon_png->w();
	const int H = cannon_png->h();

	const double s = sin(-angle);
	const double c = cos(-angle);
	
	const char* const cdata = cannon_png->data()[0];
	unsigned char* roti_data = new unsigned char[ cannon_png->w() * cannon_png->h() * 4 ];
//	unsigned char* roti_data = roti.data();
	
	for(int y=0; y < H; ++y)
	for(int x=0; x < W; ++x)
	{
		const int ym = y - H/2;
		const int xm = x - W/2;
		
		const int xo = round(xm*c - ym*s + W/2);
		const int yo = round(xm*s + ym*c + H/2);
		
		if(xo>=0 && xo<W && yo>=0 && yo<H)
		{
			const char* src = cdata + yo*W*4 + xo*4;
			std::copy(src, src+4, roti_data + y*W*4 + x*4);
		}
	}
	
	Fl_RGB_Image*  r_img = new Fl_RGB_Image(roti_data, W, H, 4, 0);
	
	return r_img->copy(64,64);
}


void Sprite::normalize()
{
	dx += (drand48()-0.5)*2;
	dy += (drand48()-0.5)*2;
	
	const double d_len = hypot(dx,dy)/6;
	dx = dx/d_len;
	dy = dy/d_len;
}


void Sprite::step()
{
	if(!inside && x>Sprite_Min.x && y>Sprite_Min.y && x<Sprite_Max.x && y<Sprite_Max.y)
		inside = true;

	x += dx;
	y += dy;
	
	if(inside && (x<=Sprite_Min.x || x>=Sprite_Max.x) )
	{
		x -= dx;
		dx = -dx;
		normalize();
	}
	
	if(inside && (y<=Sprite_Min.y || y>=Sprite_Max.y) )
	{
		y -= dy;
		dy = -dy;
		normalize();
	}
}


Sprite generateSprite(int w, int h)
{
	const double distance = std::max(w,h)*0.25;
	const double direction = drand48()*2*M_PI;
	const double pos_x = w/2 + cos(direction)*distance;
	const double pos_y = h/2 - sin(direction)*distance;
	
	const double s = std::min(w,h);
	
	// target coordinate:
	const double tx = w/2 + (drand48()-0.5)*s;
	const double ty = h/2 - (drand48()-0.5)*s;
	
	// direction, normalized:
	double dx = pos_x - tx;
	double dy = pos_y - ty;
	double d_len = hypot(dx,dy);
	dx = dx/d_len;
	dy = dy/d_len;
	
//	Fl_Image* img = new Fl_PNG_Image("geist-lila.png");
	return Sprite(pos_x, pos_y, 6*dx, 6*dy);
}


PlayField::PlayField(int x, int y, int w, int h, const char* title)
: Fl_Box(x,y,w,h,title)
, w2{Fl::w()/2}
, h2{Fl::h()/2}
, direction{0}
{
	srand48( time(0) + getpid() );

	geist_lila =  new Fl_PNG_Image("geist-lila.png");
	geist_rot  =  new Fl_PNG_Image("geist-rot.png");
	geist_gelb =  new Fl_PNG_Image("geist-gelb.png");
	geist_eis  =  new Fl_PNG_Image("geist-eis.png");
	
	geist_schatten = new Fl_PNG_Image("geist-schatten.png");
	
	cannon_png = new Fl_PNG_Image("kanone.png");
	
	fprintf(stderr, "### Kanone: d=%d count=%d, ld=%d ###\n",
		cannon_png->d(),
		cannon_png->count(),
		cannon_png->ld()
		);
	
	cannon.reserve(Directions);
	for(unsigned d=0; d<Directions; ++d)
	{
		const double dd = d * 2*M_PI / Directions;
		Fl_Image* cc = generateCannon(dd);
		cannon.push_back(cc);
	}
	
	for(unsigned u=0; u<7; ++u)
	{
		sprites.push_back( generateSprite(Fl::w(),Fl::h()) );
	}
	
	Fl::focus(this);
	Fl::add_timeout(1.0/30.5, &PlayField::static_callback, this);
}


PlayField::~PlayField()
{
	for(auto& c : cannon)
	{
		delete c;
		c = nullptr;
	}
}


template<class T>
XY<T> PlayField::to_xy(double radius)
{
	const double r = direction*(M_PI/180);
	const double s = sin(r);
	const double c = cos(r);
	
	return XY<T>{ T(w2 + c*radius + 0.5), T(h2 - s*radius + 0.5) };
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
	double min_d = 19999999;
	double max_d = 0;
	unsigned s0=0, s1=0;
	unsigned t0=0, t1=0;
	
	
	if(shooting)
	{
		fl_draw_box(FL_FLAT_BOX, shot.x-9, shot.y-9, 18, 18, Fl_Color(16) );
	}
	
	if(cannon_pos_old != cannon_pos)
	{
		fl_draw_box(FL_FLAT_BOX, cannon_pos_old.x-32, cannon_pos_old.y-32, 64,64, Fl_Color(16) );
		cannon_pos_old = cannon_pos;
	}
	
	
	for(unsigned u=0; u<sprites.size(); ++u)
	{
		Sprite& su = sprites[u];
		if(!su.alive)
			continue;
		
//		fl_draw_box(FL_FLAT_BOX, su.x, su.y, 64,64, Fl_Color(16) );
		fl_draw_box(FL_FLAT_BOX, su.x-32-8, su.y-32-8, 89,85, Fl_Color(16) );
		su.step();
		for(unsigned v=0; v<u; ++v)
		{
			const Sprite& sv = sprites[v];
			const double d = hypot(su.x - sv.x, su.y-sv.y);
			if(d<min_d)
			{
				s0=u; s1=v; min_d=d;
			}
			if(d>max_d)
			{
				t0 = u; t1=v; max_d = d;
			}
		}
//		s.img->draw( s.x, s.y );
	}
	
	cannon[ direction ]->draw( cannon_pos.x-32, cannon_pos.y-32 );
	
	
	for(unsigned u=0; u<sprites.size(); ++u)
	{
		Sprite& su = sprites[u];
		if(shooting && std::fabs(su.x-shot.x) <= 40 && std::fabs(su.y-shot.y) <= 40) // HIT!
		{
			points += su.alive;
			shooting = false;
			su.alive = false;
			su.dx = su.dy = 0;
			out_points->value(std::to_string(points).c_str());
		}
		
		if(su.alive)
		{
			geist_schatten->draw(su.x-32-8, su.y-32-8);
		}
	}
	
	if(shooting)
	{
		shot.x += shot_dir.x;
		shot.y += shot_dir.y;
		if(shot.x > Shot_Max.x || shot.x < Shot_Min.x || shot.y > Shot_Max.y || shot.y < Shot_Min.y)
		{
			shooting = false;
		}else{
			fl_color( Fl_Color(4) );
			fl_pie(shot.x-8, shot.y-8, 16, 16, 0,360);
		}
	}
	
	for(unsigned u=0; u<sprites.size(); ++u)
	{
		const Sprite& su = sprites[u];
		if(su.alive == false)
		{
			geist_eis->draw(su.x-32, su.y-32);
		}else if(u==s0 || u==s1)
		{
			geist_rot->draw(su.x-32, su.y-32);
		}else if(u==t0 || u==t1)
		{
			geist_gelb->draw(su.x-32, su.y-32);
		}else{
			geist_lila->draw(su.x-32, su.y-32);
		}
	}
}


void PlayField::make_current()
{
	mainwin->make_current();
}

void PlayField::draw()
{
	Sprite_Min.x = x()+32;
	Sprite_Min.y = y()+32;
	Sprite_Max.x = (x() + w())-32;
	Sprite_Max.y = (y() + h())-32;
	
	Shot_Min.x = x();
	Shot_Min.y = y();
	Shot_Max.x = (x() + w());
	Shot_Max.y = (y() + h());
	
	cannon_pos = jump_cannon();
	
	Fl_Box::draw();
}


void PlayField::fire()
{
	shot.x = cannon_pos.x;
	shot.y = cannon_pos.y;
	
	const double angle = direction * 2 * M_PI / Directions;
	shot_dir.x = sin(angle)*16;
	shot_dir.y = -cos(angle)*16;
	shooting   = true;
}


int PlayField::handle(int event)
{
 	switch(event)
	{
		case FL_KEYDOWN:
			switch(Fl::event_key())
			{
				case FL_Left:
					--direction;
					while(direction<0)
						direction += Directions;
					break;
				case FL_Right:
					++direction;
					while(direction>=Directions)
						direction -= Directions;
					break;
				case FL_Control_L:
					cannon_pos = jump_cannon();
					break;
				case ' ':
					if(shooting)
					{
						make_current();
						fl_draw_box(FL_FLAT_BOX, shot.x-9, shot.y-9, 18, 18, Fl_Color(16) );
					}
					fire();
					break;
			}
			return 1;
	}
	// pass other events to the base class...
	return Fl_Box::handle(event);
}


template<class Iter, class Comp>
double min_distance(Iter begin, Iter end, Comp cmp)
{
	double d = HUGE_VAL;
	for(; begin!= end; ++begin)
	{
		const double dd = cmp(*begin);
		if(dd<d) d=dd;
	}
	return d;
}


XY<int> PlayField::jump_cannon()
{
	struct D{ XY<int> xy; double d; };
	
	std::vector<D> v;
	v.reserve(10);
	for(unsigned u=0; u<10; ++u)
	{
		const int x = drand48()*(Sprite_Max.x-Sprite_Min.x) + Sprite_Min.x;
		const int y = drand48()*(Sprite_Max.y-Sprite_Min.y) + Sprite_Min.y;
		
		const D d{ XY<int>{x, y}, min_distance( sprites.cbegin(), sprites.cend(), [=](const Sprite& sp)
				{
					return hypot(x-sp.x, y-sp.y);
				})
			};
		v.push_back(d);
	}
	
	std::sort(v.begin(), v.end(), [](const D& d1, const D& d2) { return d1.d > d2.d; } );
	
	return v[0].xy;
}

