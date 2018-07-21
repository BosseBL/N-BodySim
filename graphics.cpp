//
//  graphics.cpp
//  N-body
//
//  Created by Daniel Lindfors on 21/03/15.
//  Copyright (c) 2015 Daniel Lindfors. All rights reserved.
//

#include "graphics.h"


static sf::Event event;
int drawTree = 0;

char logG[32];
char logR[32];
char logB[32];

/* ---------------- UPDATE AND RENDERING  ----------------------*/


void Frame::drawBodies(Body** bodies, int n, double maxMass) {
	sf::CircleShape c;
    int rend_x, rend_y, rend_radius, mass;
    
    double maxMassLog = log10(maxMass);
    double cMaxY, cMaxX, cMinY, cMinX;
    
    for (int i = 0; i < n; i++) {						
        if (bodies[i]->mass == 0) continue;
        cMaxY = bodies[i]->position.y + bodies[i]->radius;
        cMaxX = bodies[i]->position.x + bodies[i]->radius;
        cMinY = bodies[i]->position.y - bodies[i]->radius;
        cMinX = bodies[i]->position.x - bodies[i]->radius;
        
        mass = bodies[i]->mass;
        
        if (cMaxX < x|| cMinX > xMax || cMaxY < y || cMinY > yMax) continue;
        
        else {
            rend_x = (int)((bodies[i]->position.x-x-bodies[i]->radius)*scale);
            rend_y = (int)((bodies[i]->position.y-y-bodies[i]->radius)*scale);
            rend_radius = (int)(bodies[i]->radius*scale);
            if (rend_radius < 1) {
                rend_radius = 1;
            }
            c.setRadius(rend_radius);
            sf::Vector2f v(rend_x, rend_y);
            c.setPosition(v);
            
            double scalar = log10(bodies[i]->mass)/maxMassLog;
            char r = rainbowRed(scalar);
            char g = rainbowGreen(scalar);
            char b = rainbowBlue(scalar);
            c.setFillColor(sf::Color(r, g, b));
            
            window.draw(c);
        }
    }
}

char Frame::rainbowRed(double d) {
	if(d < 0.25) return 255;
	else if(d >= 0.5) return 0;
	else return (int)(255 - (d-0.25)*255.0/0.25);
}
char Frame::rainbowGreen(double d) {
	if(d < 0.25) return (int)(255 - d*255.0/0.25);
	else if(d > 0.75 && d < 1) return (int)(255 - (d-0.75)*255.0/0.25);
	else if(d > 1) return 0;
	else return 255;
}
char Frame::rainbowBlue(double d) {
	if(d > 0.75) return 255;
	else if(d <= 0.5) return 0;
	else return (int)(255 - (d-0.50)*255.0/0.25);
}

void Frame::drawQuadTree(QuadTree* tree) {
	if(tree->min.x > x && tree->min.x < xMax) drawLine(tree->min.x, tree->min.y, tree->min.x, tree->max.y);
	if(tree->min.y > y && tree->min.y < yMax) drawLine(tree->min.x, tree->min.y, tree->max.x, tree->min.y);
	if(tree->max.x > x && tree->max.x < xMax) drawLine(tree->max.x, tree->min.y, tree->max.x, tree->max.y);
	if(tree->max.y > x && tree->min.y < yMax) drawLine(tree->min.x, tree->max.y, tree->max.x, tree->max.y);

	drawQuadTreeRec(tree);
}

void Frame::drawQuadTreeRec(QuadTree* tree) {
	if(tree->qt[0]) {
		if(tree->mid.x > x && tree->mid.x < xMax && tree->min.y < yMax && tree->max.y > y) drawLine(tree->mid.x, tree->min.y, tree->mid.x, tree->max.y);
		if(tree->mid.y > y && tree->mid.y < yMax && tree->min.x < xMax && tree->max.x > x) drawLine(tree->min.x, tree->mid.y, tree->max.x, tree->mid.y);
		for(int i = 0; i < 4; i++) drawQuadTreeRec(tree->qt[i]);
	}
}

void Frame::drawLine(double x1, double y1, double x2, double y2) {
	sf::Vertex vertex[2];
	vertex[0].position.x = (int)((x1-x)*scale);
	vertex[0].position.y = (int)((y1-y)*scale);
	vertex[1].position.x = (int)((x2-x)*scale);
	vertex[1].position.y = (int)((y2-y)*scale);
	vertex[0].color = sf::Color::White;
	vertex[0].color.a = 100;
	vertex[1].color = sf::Color::White;
	vertex[1].color.a = 100;
	window.draw(vertex, 2, sf::Lines);
}

void Frame::drawText() {
	char str[200];
    sprintf(str, "Scale: %f\nVelX: %d, VelY: %d\nX: %f, Y: %f\n", scale, velx, vely, x, y);
    text.setString(str);
    window.draw(text);
}


void Frame::updateFrame(Body** bodies, QuadTree* tree, int n, double maxMass) {
	transpose(velx, vely);
	window.clear(sf::Color::Black);
	
	if(tree && drawTree) drawQuadTree(tree);
    if(bodies) drawBodies(bodies, n, maxMass);
	drawText();
	window.display();
}

int Frame::handleEvents() {
	while(window.pollEvent(event)) frameEventHandler();
	if(!window.isOpen()) return 1;
	return 0;
}

/* --------------------EVENT HANDLING ---------------------------*/

void Frame::frameEventHandler() {
    // Close window: exit
    if (event.type == sf::Event::Closed) window.close();
    
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                window.close(); break;
            case sf::Keyboard::Left:
                push(LEFT); break;
            case sf::Keyboard::Right:
                push(RIGHT); break;
            case sf::Keyboard::Up:
                push(UP); break;
            case sf::Keyboard::Down:
                push(DOWN); break;
            case sf::Keyboard::Space:
                stop(); break;
            //case sf::Keyboard::W:
            //    timescale = 2*timescale; break;
            //case sf::Keyboard::Q:
            //    timescale = timescale/2; break;
            case sf::Keyboard::S:
                zoomWindow(2.0);
				transpose(width/2, height/2);
                break;
            case sf::Keyboard::A:
                zoomWindow(0.5);
				transpose(-width/4, -height/4);
                break;
			case sf::Keyboard::T:
                if(drawTree) drawTree = 0;
				else drawTree = 1;
                break;
        default:
            break;
        }
    }
}




		  
		  
/* ---------------RESOURCE MANAGEMENT ------------------------*/

Frame::Frame(int w, int h, double s, int fps, char* name) {
	x = 0; y = 0; velx = 0; vely = 0;
    scale = s; width = w; height = h;
    if(!(font.loadFromFile("sansation.ttf") )) {
        printf("could not load font"); exit(1);
    }
	text = sf::Text();
    text.setFont(font);
    text.setColor(sf::Color(234, 245, 243));
    text.setCharacterSize(15);
    sf::Vector2f v(5, 5);
    text.setPosition(v);
    text.setStyle(sf::Text::Bold);
    zoom = 1;

	window.create(sf::VideoMode(w, h, 32), name);
	window.setFramerateLimit(fps);
}

Frame::~Frame() {}

Frame::Frame() {}
		  
void Frame::create(int w, int h, double s, int fps, char* name) {
	x = 0; y = 0; velx = 0; vely = 0;
    scale = s; width = w; height = h;
    if(!(font.loadFromFile("sansation.ttf") )) {
        printf("could not load font"); exit(1);
    }
	text = sf::Text();
    text.setFont(font);
    text.setColor(sf::Color(234, 245, 243));
    text.setCharacterSize(15);
    sf::Vector2f v(5, 5);
    text.setPosition(v);
    text.setStyle(sf::Text::Bold);
    zoom = 1;

	window.create(sf::VideoMode(w, h, 32), name);
	window.setFramerateLimit(fps);
}

		  
/* ----------------- FRAME POSITIONING ---------------- */
		  
void Frame::transpose(int dx, int dy) {
    x += (double)dx/scale;
    y += (double)dy/scale;
	xMax = x + (double)width/scale;
    yMax = y + (double)height/scale;
}

void Frame::zoomWindow(double z) {
    scale *= z;
}



/* -------------------- FRAME DYNAMICS -------------------- */
		  
void Frame::push(int dir) {
    switch (dir) {
        case UP:
            vely -= PIX_VEL;
            break;
        case RIGHT:
            velx += PIX_VEL;
            break;
        case DOWN:
            vely += PIX_VEL;
            break;
        case LEFT:
            velx -= PIX_VEL;
            break;
        default:
            break;
    }
}

void Frame::stop() {
    vely = 0; velx = 0;
}

