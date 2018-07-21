#ifndef __N_body__sandbox__
#define __N_body__sandbox__

#include "physics.h"
#include <time.h>

typedef struct {
	QuadTree* tree;
	double updateTime;
	double treeBuildTime;
	double positionTime;
	double collisionTime;
	double guiTime;
}Result;

void newBody(double x, double y, double velx, double vely, double density, double rad);
void quit_sandbox();
Body** init_sandbox(int n, double gravkonst, double d, double maxMass);
void updatePhysics(int collisionType, double theta, double dt, Result* res);
static void initBodiesRandom(Body** b, int n, double d, double maxMass);

#endif
