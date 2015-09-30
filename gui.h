#ifndef __SIMPLE_GUI_H
#define __SIMPLE_GUI_H
//-------------------------------------------------------------------------
#ifdef USE_GUI
void gui_init(int argc, char **argv, char *buffer, int W, int H);
void gui_show(int n);
void gui_done();
#else
#define gui_init(argc, argv, buffer, W, H)
#define gui_show(n)
#define gui_done()
#endif
//-------------------------------------------------------------------------
#endif
