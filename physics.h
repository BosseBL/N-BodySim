//
//  physics.h
//  N-body
//
//  Created by Daniel Lindfors on 21/03/15.
//  Copyright (c) 2015 Daniel Lindfors. All rights reserved.
//

#ifndef __N_body__physics__
#define __N_body__physics__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctime>
#include <omp.h>

#define PI 3.141592653589

#define ELASTIC 1
#define INELASTIC 3
#define INELASTIC_MERGE 2
#define NONE 0


extern int n_threads;

typedef struct {
    double x, y;
} Vector;

typedef struct {
    double mass;
    double radius;
    Vector position;
    Vector velocity;
    Vector force;
    // for page swap optimization. line size are usually 64 byte. if overflow use 128 byte
    //int padding[( 128-11*sizeof(double) - sizeof(int) )/4];
    char padding[64-8*sizeof(double)];
} Body;

typedef struct QuadTree {
    double mass;
	struct QuadTree* qt[4];
	Vector min, max, mid, com;
	Body* body;
	
	// for page swap optimization. line size are usually 64 byte. if overflow use 128 byte
    char padding[128 - 9*sizeof(double) - sizeof(QuadTree*)*4 - sizeof(Body*)];
} QuadTree;


void updateForceBarnesHutRec(Body* b, QuadTree* tree, double theta, double G);
void updateForceBarnesHut(Body** b, int n, QuadTree* tree, double theta, double G);
static QuadTree* getCorespondingNode(Body* b, QuadTree* root);
static void addToTree(Body* b, QuadTree* root);
double timer();
void updateForce(Body** b, int n, double theta, double G);
void updatePosition(Body** b, int n, double dt);
int collisionDetection(Body* a, Body* b);
void collisionResolve(Body* a, Body* b, int type);
QuadTree* newQuadTree();
void destroyQuadTree(QuadTree* tree);
double escapeVelocity(double mass, double distance, double G);
double totalMass(Body** b, int n);
void freeBodyArray(Body** b, int n);
Body* allocateBody();
Body** allocateBodyArray(int n);
Vector centerOfMass(Body** b, int n);
QuadTree* buildTree(Body** b, int n);
void collisionHandling(Body** b, int n, int type);
void printTree(QuadTree* tree);
void printTree(QuadTree* tree, int l, int c);

#endif /* defined(__N_body__physics__) */

