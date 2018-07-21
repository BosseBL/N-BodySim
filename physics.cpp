//
//  physics.c
//  N-body
//
//  Created by Daniel Lindfors on 21/03/15.
//  Copyright (c) 2015 Daniel Lindfors. All rights reserved.
//


#include "physics.h"


//static struct timeval startTime, stopTime; // start and stop time for timer
static double startTime, stopTime;
int n_threads = 1;

/* --------------------- BARNES HUT QUAD TREE BUILDNING ------------------------------*/

// build tree based on a list of bodies
QuadTree* buildTree(Body** b, int n) {
	QuadTree* root = newQuadTree();
	double width, height, correction;
	// Set initial bounds for root node
	

	// set the width and height for the root node based on the positions of the bodies.

	/* This was a try for more parallelization. The job is too small to overcome the overhead though
	double minx, miny, maxx, maxy;
	#pragma omp parallel for reduction(max:maxx, maxy) reduction(min:minx, miny) schedule(static)
	for(int i = 0; i < n; i++) {
    	    if(b[i]->position.x < minx) minx = b[i]->position.x;
    	    if(b[i]->position.x > maxx) maxx = b[i]->position.x;
    	    if(b[i]->position.y < miny) miny = b[i]->position.y;
    	    if(b[i]->position.y > maxy) maxy = b[i]->position.y;
   	}
	root->max.x = maxx; root->min.x = minx; root->max.y = maxy; root->min.y = miny;
	*/

	for(int i = 0; i < n; i++) {
    	    if(b[i]->position.x < root->min.x) root->min.x = b[i]->position.x;
    	    if(b[i]->position.x > root->max.x) root->max.x = b[i]->position.x;
    	    if(b[i]->position.y < root->min.y) root->min.y = b[i]->position.y;
    	    if(b[i]->position.y > root->max.y) root->max.y = b[i]->position.y;
   	}

	width = root->max.x - root->min.x;
	height = root->max.y - root->min.y;
	if(width < height) {
		correction = (height-width)/2;
		root->min.x -= correction;
		root->max.x += correction;
	}
	else{
		correction = (width-height)/2;
		root->min.y -= correction;
		root->max.y += correction;
	}
	
	// recursively adds bodies to the tree
	for(int i = 0; i < n; i++) {
		if(b[i]->mass != 0) addToTree(b[i], root);
	}

	return root;
}

// recursively adds a new body to a node
// reference http://portillo.ca/nbody/implementation/
static void addToTree(Body* b, QuadTree* root) {
	// Someone was in this node before
	if(root->mass != 0) {
		// update mass and center of mass for node
		double newMass = root->mass + b->mass;
		root->com.x = (root->com.x*root->mass + b->position.x*b->mass)/newMass;
		root->com.y = (root->com.y*root->mass + b->position.y*b->mass)/newMass;
		root->mass = newMass;
		
		// Another body is here, need to split and distribute
		if(root->body) {
			// create child nodes
			for(int i = 0; i < 4; i++) root->qt[i] = newQuadTree();
			
			// calculate mid point. The only time we realy need the midpoint is to speed up the spliting
			root->mid.x = (root->max.x - root->min.x)/2 + root->min.x;
			root->mid.y = (root->max.y - root->min.y)/2 + root->min.y;
		
			// initialize child node positions
			root->qt[0]->min.x = root->min.x;
			root->qt[0]->min.y = root->min.y;
			root->qt[0]->max.x = root->mid.x;
			root->qt[0]->max.y = root->mid.y;
			
			root->qt[1]->min.x = root->mid.x;
			root->qt[1]->min.y = root->min.y;
			root->qt[1]->max.x = root->max.x;
			root->qt[1]->max.y = root->mid.y;
			
			root->qt[2]->min.x = root->mid.x;
			root->qt[2]->min.y = root->mid.y;
			root->qt[2]->max.x = root->max.x;
			root->qt[2]->max.y = root->max.y;
			
			root->qt[3]->min.x = root->min.x;
			root->qt[3]->min.y = root->mid.y;
			root->qt[3]->max.x = root->mid.x;
			root->qt[3]->max.y = root->max.y;
			
			// recursively add both root and b to their coresponding nodes
			addToTree(b, getCorespondingNode(b, root));
			addToTree(root->body, getCorespondingNode(root->body, root));
			
			// forget last body
			root->body = NULL;
		}
		// The node has already been split. recursive call to coresponding child.
		else addToTree(b, getCorespondingNode(b, root));		
	}
	// First here. Only need to copy the body variables and add it.
	else {
		root->mass = b->mass; root->com.x = b->position.x; root->com.y = b->position.y;
		root->body = b;
	}
	// if node contains body
}

// Helper that returns the coresponding child node a body should be placed in
static QuadTree* getCorespondingNode(Body* b, QuadTree* root) {
	if(b->position.x > root->mid.x) {
		if(b->position.y > root->mid.y) return root->qt[2];
		else return root->qt[1];
	}
	else {
		if(b->position.y > root->mid.y) return root->qt[3];
		else return root->qt[0];
	}
}



/* ------------------------ FORCE CALCULATION --------------------------- */


// recursively update forces with the the COM and mass of the tree 
void updateForceBarnesHut(Body** b, int n, QuadTree* tree, double theta, double G) {
	if(tree->qt[0]) {
		//#pragma omp parallel for schedule(static)
		for(int i = 0; i < n; i++) {
			if(b[i]->mass != 0) {
				for(int j = 0; j < 4; j++) updateForceBarnesHutRec(b[i], tree->qt[j], theta, G);
			}
		}
	}	
}

void updateForceBarnesHutRec(Body* b, QuadTree* tree, double theta, double G) {
	if(tree->body == b) return;
	Vector d; double dist, mag, sizeDistRatio;
	// calculate distance from the body to com of the node
	d.x = tree->com.x - b->position.x;
	d.y = tree->com.y - b->position.y;
	dist = sqrt(d.x*d.x + d.y*d.y);
	// if there is children to this node
	if(tree->qt[0]) {
		// se if the approximation is good enough and calculate force
		sizeDistRatio = (tree->max.x - tree->min.x)/dist;
		if(sizeDistRatio < theta) {
			mag = G*(b->mass*tree->mass)/(dist*dist*dist);
			b->force.x += mag*d.x;
			b->force.y += mag*d.y;
		}
		// if not good enough we traverse the children
		else {
			for(int i = 0; i < 4; i++) {
				updateForceBarnesHutRec(b, tree->qt[i], theta, G);
			}
		}
	}
	// if no children then this is a leaf and we can calculate force directly
	else {
		mag = G*(b->mass*tree->mass)/(dist*dist*dist);
		b->force.x += mag*d.x;
		b->force.y += mag*d.y;
	}
}

// updates the acting force upon all bodies in bodies array
void updateForce(Body** b, int n, double theta, double G) {
/*	if(n_threads == 1) { 
    	for (int i = 0 ; i < n ; i++) {
			double d; Vector v;
    	    if (b[i]->mass == 0) continue;
    	    for (int j = i+1; j < n; j++) {
    	        if (b[j]->mass == 0) continue;
    	        v.x = b[j]->position.x - b[i]->position.x;
    	        v.y = b[j]->position.y - b[i]->position.y;
    	        d = sqrt(v.x*v.x + v.y*v.y);
    	        d = G*(b[i]->mass*b[j]->mass)/(d*d*d);
				v.x *= d;
				v.y *= d;
    		 	b[i]->force.x += v.x;
    			b[i]->force.y += v.y;
				b[j]->force.x -= v.x;
   				b[j]->force.y -= v.y;
    		}
    	}
	}
	else {*/
//		#pragma omp parallel for schedule(static)
    	for (int i = 0 ; i < n ; i++) {
			double d; Vector v;
    	    if (b[i]->mass == 0) continue;
    	    for (int j = 0; j < n; j++) {
    	        if (b[j]->mass == 0 || j == i) continue;
    	        v.x = b[j]->position.x - b[i]->position.x;
    	        v.y = b[j]->position.y - b[i]->position.y;
    	        d = sqrt(v.x*v.x + v.y*v.y);
    	        d = G*(b[i]->mass*b[j]->mass)/(d*d*d);
				v.x *= d;
				v.y *= d;
    	        b[i]->force.x += v.x;
    	        b[i]->force.y += v.y;
    	    }
    	}
//	}	
}	

void printTree(QuadTree* tree) {printTree(tree, 0, 0);}
void printTree(QuadTree* tree, int l, int c) {
	if(tree) {
		for(int j = 0; j < l; j++) printf("\t");
		if(l == 0) printf("root: %.3lf kg, (%.3lf m, %.3lf m)\n", tree->mass, tree->com.x, tree->com.y);
		else printf("child %d: %.3lf kg, (%.3lf m, %.3lf m)\n", c, tree->mass, tree->com.x, tree->com.y);
		for(int i = 0; i < 4; i++) printTree(tree->qt[i], l+1, i);
	}	
}



/* ------------------ POSITION UPDATE ------------------------- */


void updatePosition(Body** b, int n, double dt) {	
	//#pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
		Vector dv, d;
        if (b[i]->mass == 0) continue;
        dv.x = (b[i]->force.x/b[i]->mass)*dt;
        dv.y = (b[i]->force.y/b[i]->mass)*dt;
        d.x = (b[i]->velocity.x + dv.x/2)*dt;
        d.y = (b[i]->velocity.y + dv.y/2)*dt;
        b[i]->velocity.x += dv.x;
        b[i]->velocity.y += dv.y;
        b[i]->position.x += d.x;
        b[i]->position.y += d.y;
        b[i]->force.x = 0;
        b[i]->force.y = 0;
    }
}


/*-------------------- COLISION HANDLING --------------------------*/

void collisionHandling(Body** b, int n, int type) {
   	for (int i = 0; i < n; i++) {
       	if (b[i]->mass == 0) continue;
       	for (int j = i+1; j < n; j++) {
       	    if (b[j]->mass == 0) continue;
       	    if (collisionDetection(b[i], b[j])) collisionResolve(b[i], b[j], type);
       	}
   	}	
}

// Detect collision
int collisionDetection(Body* a, Body* b) {
    double dist; Vector d;
    d.x = a->position.x - b->position.x;
    d.y = a->position.y - b->position.y;
    dist = sqrt(d.x*d.x + d.y*d.y);
            
    if (a->radius + b->radius >= dist) return 1;
    else return 0;
}

// Resolve collision between two bodies
void collisionResolve(Body* a, Body* b, int type) {

    
// when colliding, bodies merge to one. aka perfect inelastic collision
	if(type == INELASTIC_MERGE) {
    	double vol = (pow(a->radius, 3) + pow(b->radius, 3))*PI*4/3;
    	double rad = pow((float)(vol*3/(4*PI)), (float)1/3);
    	double mass = a->mass + b->mass;
    	
		Vector mv;
    	mv.x = a->velocity.x*a->mass + b->velocity.x*b->mass;
    	mv.y = a->velocity.y*a->mass + b->velocity.y*b->mass;
    	
    	Body* eater;
    	Body* food;
    	if (a->mass > b->mass) { eater = a; food = b; }
    	else { eater = b; food = a; }
    	
    	eater->mass = mass;
    	eater->radius = rad;
    	eater->velocity.x = mv.x/mass;
    	eater->velocity.y = mv.y/mass;
    	food->mass = 0;

	}

	// Poorly made and unrealistic, but more efficient collision. Bodies seem to stick to each other after collision. 
	if(type == INELASTIC) {
    	double mass = a->mass + b->mass;
   
		Vector mv;
    	mv.x = a->velocity.x*a->mass + b->velocity.x*b->mass;
    	mv.y = a->velocity.y*a->mass + b->velocity.y*b->mass;
    	
		Vector d;
		d.x = a->position.x - b->position.x;
		d.y = a->position.y - b->position.y;

		double dist = sqrt(d.x*d.x + d.y*d.y);
		double overlap = a->radius + b->radius - dist;

		d.x = (d.x/dist)*overlap;
		d.y = (d.y/dist)*overlap;

		a->position.x += d.x; a->position.y += d.y;
		b->position.x -= d.x; b->position.y -= d.y;
	
		a->velocity.x = mv.x/mass;
    	a->velocity.y = mv.y/mass;
		b->velocity.x = mv.x/mass;
    	b->velocity.y = mv.y/mass;
	}
    
// when colliding, bodies bounce off each other. aka perfect elastic collision
// un = unitNormal, ut = unitTangent. 3D needs to be fixed
	if(type == ELASTIC) {
    	double dist, pushbackDist; Vector d, un, ut;
    	d.x = a->position.x - b->position.x;
    	d.y = a->position.y - b->position.y;
    	dist = sqrt(d.x*d.x + d.y*d.y);
    	un.x = d.x/dist;
    	un.y = d.y/dist;
    	
    	pushbackDist = a->radius + b->radius - dist;
    	
    	a->position.x += pushbackDist*un.x;
    	a->position.y += pushbackDist*un.y;
		
    	ut.x = -un.y;
    	ut.y = un.x;
    	
    	double ux1 = a->velocity.x;
    	double uy1 = a->velocity.y;
    	double m1 = a->mass;
    	
    	double ux2 = b->velocity.x;
    	double uy2 = b->velocity.y;
    	double m2 = b->mass;
    	
    	double u1n = un.x*ux1+un.y*uy1;
    	double v1t = ut.x*ux1+ut.y*uy1;
    	double u2n = un.x*ux2+un.y*uy2;
    	double v2t = ut.x*ux2+ut.y*uy2;
    	
    	double v1n = (u1n*(m1-m2)+2*m2*u2n)/(m1+m2);
    	double v2n = (u2n*(m2-m1)+2*m1*u1n)/(m1+m2);
    	
    	a->velocity.x = v1t*ut.x + v1n*un.x;
    	a->velocity.y = v1t*ut.y + v1n*un.y;
    	
    	b->velocity.x = v2t*ut.x + v2n*un.x;
   		b->velocity.y = v2t*ut.y + v2n*un.y;
	}	

}



/* --------------------- TOOLS ---------------------- */


// return time in seconds since the last call of this function
double timer() {
  stopTime = omp_get_wtime();
  double returnval = (double)(stopTime - startTime);
  startTime = omp_get_wtime();
  return returnval;
}


double escapeVelocity(double mass, double distance, double G) {
	return sqrt(2*G*mass/distance);
}


double totalMass(Body** b, int n) {
	double mass = 0;
	for(int i = 0; i < n; i++) {
		mass += b[i]->mass;
	}
	return mass;
}

Vector centerOfMass(Body** b, int n) {
	Vector com; com.x=0; com.y=0;
	double totalMass = 0;
	for(int i = 0; i < n; i++) {
		com.x += b[i]->position.x*b[i]->mass;
		com.y += b[i]->position.y*b[i]->mass;
		totalMass += b[i]->mass;
	}
	com.x = com.x/totalMass;
	com.y = com.y/totalMass;
	return com;
}



/* ------------------ RESOURCE MANAGEMENT ------------------*/


void freeBodyArray(Body** b, int n) {
	for(int i = 0; i < n; i++) {
		free(b[i]);
	}
	free(b);
}


Body* allocateBody() {
	return (Body*) malloc(sizeof(Body));
}

Body** allocateBodyArray(int n) {
	Body** b = (Body**) malloc(n*sizeof(Body*));
	for(int i = 0; i < n; i++) {
		b[i] = NULL;
	} 
	return b;
}


QuadTree* newQuadTree() {
    QuadTree* root = (QuadTree*) malloc(sizeof(QuadTree));
    root->min.x = 0; root->max.x = 0; root->min.y = 0; root->max.y = 0; root->mid.x = 0; root->mid.y = 0; root->mass = 0; root->com.x = 0; root->com.y = 0; 
    for (int i = 0; i < 4; i++) {
        root->qt[i] = NULL;
    }
    return root;
}

void destroyQuadTree(QuadTree* tree) {
	for(int i = 0 ; i < 4 ; i++) {
		if(tree->qt[i] == NULL) continue;
		else destroyQuadTree(tree->qt[i]);
	}
	free(tree);
 	
}

