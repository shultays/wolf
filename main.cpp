#define _CRT_SECURE_NO_DEPRECATE
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#undef MOUSE_MOVED
#include<curses.h>
#include <math.h>

#define PI 3.14159265f
#define PI_2 (2*3.14159265f)

bool running = true;

WINDOW * win;

char* map;

int map_width;
int map_height;

int getColorIndex(int r, int g, int b) {
	return (b >> 1) | (g << 2) | (r << 5);
}

char get_tile(int i, int j){
	return map[i + j * map_height];
}

void set_tile(int i, int j, char c){
	map[i + j * map_height] = c;
}

int w = 40;
int h = 120;

struct Coor {
	float x;
	float y;
};

struct Coor_hit {
	int x, y;
	Coor hit_point;
};


struct Color{
	int r, g, b;
};

Color colors[] = { {7, 0, 0}, {0, 7, 0}, {0, 0, 7}, {7, 7, 0}, {7, 0 ,7}, {0, 7, 7}, {4, 4, 4} };


Coor p;


float distance(const Coor* c0, const Coor* c1){
	float dx = c0->x - c1->x;
	float dy = c0->y - c1->y;
	return sqrtf(dx*dx+dy*dy);
}

float dir_angle = 0.0f;

int camera = 0;

void control(char c){
	switch (c) {
	case 'q':
		running = false;
		break;
	case 'c':
		camera = 1 - camera;
		break;

	case 'd':
		dir_angle += 0.03f;
		if(dir_angle >= PI_2){
			dir_angle -= PI_2;
		}
		break;
	case 'a':
		dir_angle -= 0.03f;
		if(dir_angle < 0.0f ){
			dir_angle += PI_2;
		}
		break;


	case 'w':
		p.x += cos(dir_angle)*0.1f;
		p.y += sin(dir_angle)*0.1f;
		break;
	}
}

Coor_hit do_ray_test(const Coor* start, float angle){
	float dx = cos(angle);
	float dy = sin(angle);

	int x = (int)start->x;
	int y = (int)start->y;
	Coor_hit c;
	if(fabs(dx) > fabs(dy)){
		float dyx = dy/dx;
		if(dx > 0){
			do{
				x++;
				y = (int)((x-start->x)*dyx+start->y);
			}while(get_tile(x, y) == ' ');
			c.x = x;
			c.y = y;
			c.hit_point.x = (float)x;
			c.hit_point.y = (c.hit_point.x-start->x)*dyx+start->y;
		} else {
			do{
				x--;
				y = (int)((x-start->x)*dyx+start->y);
			}while(get_tile(x, y) == ' ');
			c.x = x;
			c.y = y;
			c.hit_point.x = (float)(x+1);
			c.hit_point.y = (c.hit_point.x-start->x)*dyx+start->y;
		}
	}else{
		float dxy = dx/dy;
		if(dy > 0){
			do{
				y++;
				x = (int)((y-start->y)*dxy+start->x);
			}while(get_tile(x, y) == ' ');

			c.x = x;
			c.y = y;
			c.hit_point.y = (float)y;
			c.hit_point.x = (c.hit_point.y-start->y)*dxy+start->x;
		} else {
			do{
				y--;
				x = (int)((y-start->y)*dxy+start->x);
			}while(get_tile(x, y) == ' ');
			c.x = x;
			c.y = y;
			c.hit_point.y = (float)(y+1);
			c.hit_point.x = (c.hit_point.y-start->y)*dxy+start->x;
		}
	}
	return c;
}

void init_colors(){
	for (int r = 0; r < 8; r++) {
		for (int g = 0; g < 8; g++) {
			for (int b = 0; b < 4; b++) {
				int index = getColorIndex(r, g, b * 2);
				int rc = (int)(r*(1000.0f / 7));
				int gc = (int)(g*(1000.0f / 7));
				int bc = (int)(b*(1000.0f / 3));

				init_color(index, rc, gc, bc);

				init_pair(index, index, COLOR_BLACK);
			}
		}
	}
}

void read_map(){
	FILE* file = fopen("map.txt", "r");
	fscanf(file, "%d", &map_width);
	fscanf(file, "%d", &map_height);
	map = (char*)malloc(map_width*map_height*sizeof(char));
	for(int i=0; i<map_height; i++){
		for(int j=0; j<map_width; j++){
			char c;
			do{
				c = fgetc(file);
			}while(c == '\n' || c == '\r');

			set_tile(i, j, c);
		}
	}
	int icx, icy;
	fscanf(file, "%d", &icx);
	fscanf(file, "%d", &icy);
	p.x = icx+0.5f;
	p.y = icy+0.5f;
	fclose(file);
}


void gameRefresh() {
	erase();
	attrset(COLOR_PAIR(getColorIndex(7, 7, 7)));

	if(camera == 0){
		attrset(COLOR_PAIR(getColorIndex(7, 7, 3)));

		Coor_hit hit = do_ray_test(&p, dir_angle);
		
		for(int i=0; i<map_height; i++){
			for(int j=0; j<map_width; j++){
				if(i == (int)p.x && j == (int)p.y){
					mvaddch(i, j, '@');
				}else if(hit.x == i && hit.y == j){
					mvaddch(i, j, '!');
				}else{
					mvaddch(i, j, get_tile(i, j));
				}
			}
		}

		float d = distance(&hit.hit_point, &p);
		mvprintw(map_height+1, 0, "%f", d);
	}else{
		for(int j=0; j<h; j++){
			float ang = (j-h/2) * 0.01f + dir_angle;

			Coor_hit hit = do_ray_test(&p, ang);
			float d = distance(&hit.hit_point, &p);
			float length = 3.0f/d;
			if(j == 30 || j == 31){
				int a= 5;
			}
			if(length > 1.0f){
				length = 1.0f;
			}

			int cnt = length*w;
			int x_start = (w-cnt)/2;
			char c = get_tile(hit.x, hit.y);
			if(c >= 'a' && c <= 'f'){
				int ci = c-'a';
				attrset(COLOR_PAIR(getColorIndex(colors[ci].r, colors[ci].g, colors[ci].b)));
			}else{
				attrset(COLOR_PAIR(getColorIndex(7, 7, 3)));
			}
			for(int i=x_start; i<x_start+cnt; i++){
				mvaddch(i, j, 'X');
			}
		}
	}
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow) {

	ShowWindow(GetActiveWindow(), SW_SHOW);
	win = initscr();
	raw();

	mmask_t old;
	mousemask (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, &old);

	keypad(stdscr, TRUE);
	start_color();

	scrollok(win, true);
	keypad(win, true);
	nodelay(win, true);
	curs_set(0);

	resize_term(w, h);

	init_colors();
	read_map();

	while (running) {
		char c;
		while ((c = getch()) != ERR) {
			control(c);
		}

		gameRefresh();

		Sleep(20);
	}
	endwin();

	return 0;
}