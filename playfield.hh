#ifndef PLAYFIELD_HH
#define PLAYFIELD_HH

#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <vector>

struct Sprite
{
	Sprite(double _x, double _y, double _dx, double _dy)
	: x(_x), y(_y), dx(_dx), dy(_dy)
	{}

	double x, y;
	double dx, dy;
	bool inside = false;
	bool alive = true;
//	Fl_Image* img;
	
	void step();
	void normalize();
};


template<class T = int>
struct XY { T x,y; };

template<class T>
inline
bool operator!=( const XY<T> a, const XY<T> b) { return a.x!=b.x || a.y!=b.y; }


class PlayField : public Fl_Box
{
public:
	static const int box_size = 88;
	static const int Directions = 20; // number of different directions the cannon can shoot

	PlayField(int x, int y, int w, int h, const char* title="");
	~PlayField();

protected:
	
	virtual void draw() override;
	virtual int handle(int event) override;
	
	template<class T>
	XY<T> to_xy(double radius);
	
	static void static_callback(void* userdata);
	void callback();
	
	void fire();
	void make_current();
	
	XY<int> jump_cannon();

private:
	const int w2;
	const int h2;
	XY<int> Shot_Min, Shot_Max;
	int direction = 0;
	bool shooting = false;
	XY<double> shot, shot_dir;
	XY<int> cannon_pos, cannon_pos_old{-100,-100};
	std::vector<Sprite> sprites;
	std::vector<Fl_Image*> cannon;
	unsigned level  = 1;
	unsigned points = 0;
	unsigned lives  = 3;
};

#endif
