#include "Physics.h"

float Physics::fMaxSimulationTimestep = DEFAULT_MAX_SIMULATION_TIMESTEP;
float Physics::fGravity = DEFAULT_GRAVITY;
float Physics::fAirDensity = DEFAULT_AIR_DENSITY;
float Physics::fDragCoefficient = DEFAULT_DRAG_COEFFICIENT;
float Physics::fWindSpeed = DEFAULT_WIND_SPEED;
float Physics::fRestitution = DEFAULT_RESTITUTION;
float Physics::fThrustForce = DEFAULT_THRUST_FORCE;
float Physics::fLinearDragCoefficient = DEFAULT_LINEAR_DRAG_COEFFICIENT;
int   Physics::iSolverIterationCount = DEFAULT_SOLVER_ITERATION_COUNT;
float Physics::fSleepAngularVelocity = DEFAULT_SLEEP_ANGULAR_VELOCITY;
float Physics::fSleepVelocity = DEFAULT_SLEEP_VELOCITY;