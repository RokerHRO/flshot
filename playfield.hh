#ifndef PLAYFIELD_HH
#define PLAYFIELD_HH

#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <vector>


class PlayField : public Fl_Box
{
public:
	PlayField(int x, int y, int w, int h, const char* title="");
	virtual ~PlayField() = default;

protected:
	
	virtual void draw() override;
	
	static void static_callback(void* userdata);
	void callback();
	
	void make_current();
	

private:
};

#endif
