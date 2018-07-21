//
//  graphics.h
//  N-body
//
//  Created by Daniel Lindfors on 21/03/15.
//  Copyright (c) 2015 Daniel Lindfors. All rights reserved.
//

#ifndef __N_body__graphics__
#define __N_body__graphics__

#include <SFML/Graphics.hpp>

#include "physics.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define PIX_VEL 1


class Frame {
public:
	sf::RenderWindow window;
    double x, y;             // position
	double xMax, yMax;
    double scale;            // pix/dist
    double zoom;
    int velx, vely;
    int width, height;
    sf::Font font;
    sf::Text text; 
    
	Frame();
    Frame(int w, int h, double s, int fps, char* name);
    ~Frame();
    
	void create(int w, int h, double s, int fps, char* name);
    int isOpen();
    void transpose(int dx, int dy);
    void zoomWindow(double z);
    void push(int dir);
    void stop();
    void drawBodies(Body** bodies, int n, double maxMass);
    void drawQuadTree(QuadTree* qt);
    void drawText();
    void frameEventHandler();
    char rainbowRed(double d);
    char rainbowGreen(double d);
    char rainbowBlue(double d);
    int handleEvents();
    void updateFrame(Body** bodies, QuadTree* tree, int n, double maxMass);
    void destroyFrame();
    void drawLine(double x1, double y1, double x2, double y2);
    void drawQuadTreeRec(QuadTree* tree);
};


#endif /* defined(__N_body__graphics__) */
