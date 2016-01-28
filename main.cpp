#define _CRT_SECURE_NO_DEPRECATE
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#undef MOUSE_MOVED
#include<curses.h>
#include<math.h>

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


#define PI 3.14159265f
#define PI_2 (2*3.14159265f)

bool running = true;

WINDOW * win;

char* map;

int map_width;
int map_height;

int w = 40;
int h = 120;

Coor p;

float dir_angle = 0.0f;

int camera = 0;


int getColorIndex(int r, int g, int b) {
	return (b >> 1) | (g << 2) | (r << 5);
}

char get_tile(int i, int j){
	return map[i + j * map_height];
}

void set_tile(int i, int j, char c){
	map[i + j * map_height] = c;
}

float distance(const Coor* c0, const Coor* c1){
	float dx = c0->x - c1->x;
	float dy = c0->y - c1->y;
	return sqrtf(dx*dx+dy*dy);
}

int round(float t){
	int t_int = (int)t;
	if(t-t_int > 0.5f){
		return t_int+1;
	}else{
		return t_int;
	}
}

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
	float adx = fabs(dx);
	float ady = fabs(dy);
	float x_cur = start->x;
	float y_cur = start->y;
	int x = (int)x_cur;
	int y = (int)y_cur;
	Coor_hit c;
	do{
		float tx;
		if(dx > 0){
			tx = (x+1) - x_cur;
		}else{
			tx = x_cur - x;
		}
		float ty;
		if(dy > 0){
			ty = (y+1) - y_cur;
		}else{
			ty = y_cur - y;
		}

		if(tx/adx < ty/ady){
			if(dx > 0){
				x++;
				x_cur = (float)x;
				y_cur = (x_cur - start->x)*dy/dx + start->y;
			}else{
				x--;
				x_cur = (float)x+1.0f;
				y_cur = (x_cur - start->x)*dy/dx + start->y;
			}
		}else{

			if(dy > 0){
				y++;
				y_cur = (float)y;
				x_cur = (y_cur - start->y)*dx/dy + start->x;
			}else{
				y--;
				y_cur = (float)y+1.0f;
				x_cur = (y_cur - start->y)*dx/dy + start->x;
			}
		}

	}while(get_tile(x, y) == ' ');
	c.x = x;
	c.y = y;
	c.hit_point.x = x_cur;
	c.hit_point.y = y_cur;

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


void game_refresh() {
	erase();
	attrset(COLOR_PAIR(getColorIndex(7, 7, 7)));

	if(camera == 0){
		attrset(COLOR_PAIR(getColorIndex(4, 4, 4)));

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
	}else{
		for(int j=0; j<h; j++){
			float ang = (j-h/2) * 0.01f + dir_angle;

			Coor_hit hit = do_ray_test(&p, ang);
			float d = distance(&hit.hit_point, &p);
			float length = 3.0f/d;
			if(length > 1.0f){
				length = 1.0f;
			}

			int cnt = round(length*w);
			int x_start = (w-cnt)/2;
			char c = get_tile(hit.x, hit.y);
			if(c >= 'a' && c <= 'f'){
				int ci = c-'a';
				static Color colors[] = { {7, 0, 0}, {0, 7, 0}, {0, 0, 7}, {7, 7, 0}, {7, 0 ,7}, {0, 7, 7}, {4, 4, 4} };
				attrset(COLOR_PAIR(getColorIndex(colors[ci].r, colors[ci].g, colors[ci].b)));
			}else{
				attrset(COLOR_PAIR(getColorIndex(4, 4, 4)));
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

		game_refresh();
		move(w-1, 0);
		Sleep(20);
	}
	endwin();

	return 0;
}