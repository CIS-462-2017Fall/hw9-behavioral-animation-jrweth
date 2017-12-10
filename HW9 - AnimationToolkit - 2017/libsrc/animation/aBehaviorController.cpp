#include "aBehaviorController.h"

#include "aVector.h"
#include "aRotation.h"
#include <Windows.h>
#include <algorithm>

#include "GL/glew.h"
#include "GL/glut.h"



#define Truncate(a, b, c) (a = max<double>(min<double>(a,c),b))

double BehaviorController::gMaxSpeed = 1000.0; 
double BehaviorController::gMaxAngularSpeed = 200.0;  
double BehaviorController::gMaxForce = 2000.0;  
double BehaviorController::gMaxTorque = 2000.0;
double BehaviorController::gKNeighborhood = 500.0;   
double BehaviorController::gOriKv = 256;    
double BehaviorController::gOriKp = 32;  
double BehaviorController::gVelKv = 100;    
double BehaviorController::gAgentRadius = 80.0;  
double BehaviorController::gMass = 1;
double BehaviorController::gInertia = 1;
double BehaviorController::KArrival = 1.0; 
double BehaviorController::KDeparture = 12000.0;
double BehaviorController::KNoise = 15.0;
double BehaviorController::KWander = 80.0;   
double BehaviorController::KAvoid = 600.0;  
double BehaviorController::TAvoid = 2000.0;   
double BehaviorController::KSeparation = 12000.0; 
double BehaviorController::KAlignment = 1.0;  
double BehaviorController::KCohesion = 1.0;  

const double M2_PI = M_PI * 2.0;

BehaviorController::BehaviorController() 
{
	m_state.resize(m_stateDim);
	m_stateDot.resize(m_stateDim);
	m_controlInput.resize(m_controlDim);

	vec3 m_Pos0 = vec3(0, 0, 0);
	vec3 m_Vel0 = vec3(0, 0, 0);
	vec3 m_lastVel0 = vec3(0, 0, 0);
	vec3 m_Euler = vec3(0, 0, 0);
	vec3 m_VelB = vec3(0, 0, 0);
	vec3 m_AVelB = vec3(0, 0, 0);
	
	m_Vdesired = vec3(0, 0, 0);
	m_lastThetad = 0.0;

	m_Active = true; 
	mpActiveBehavior = NULL;
	mLeader = false;

	reset();
}

AActor* BehaviorController::getActor()
{
	return m_pActor;
}

void BehaviorController::setActor(AActor* actor)

{
	m_pActor = actor;
	m_pSkeleton = m_pActor->getSkeleton();

}


void BehaviorController::createBehaviors(vector<AActor>& agentList, vector<Obstacle>& obstacleList)
{
	
	m_AgentList = &agentList;
	m_ObstacleList = &obstacleList;

	m_BehaviorList.clear();
	m_BehaviorList[SEEK] = new Seek(m_pBehaviorTarget);
	m_BehaviorList[FLEE] = new Flee(m_pBehaviorTarget);
	m_BehaviorList[ARRIVAL] = new Arrival(m_pBehaviorTarget);
	m_BehaviorList[DEPARTURE] = new Departure(m_pBehaviorTarget);
	m_BehaviorList[WANDER] = new Wander();
	m_BehaviorList[COHESION] = new Cohesion(m_AgentList);
	m_BehaviorList[ALIGNMENT] = new Alignment(m_pBehaviorTarget, m_AgentList);
	m_BehaviorList[SEPARATION] = new Separation(m_pBehaviorTarget, m_AgentList);
	m_BehaviorList[LEADER] = new Leader(m_pBehaviorTarget, m_AgentList);
	m_BehaviorList[FLOCKING] = new Flocking(m_pBehaviorTarget, m_AgentList);
	m_BehaviorList[AVOID] = new Avoid(m_pBehaviorTarget, m_ObstacleList);
}

BehaviorController::~BehaviorController()
{
	mpActiveBehavior = NULL;
}

void BehaviorController::reset()
{
	vec3 startPos;
	startPos[0] = ((double)rand()) / RAND_MAX;
	startPos[1] =  ((double)rand()) / RAND_MAX, 
	startPos[2] = ((double)rand()) / RAND_MAX;
	startPos = startPos - vec3(0.5, 0.5, 0.5);

	startPos[1] = 0; // set equal to zero for 2D case (assume y is up)

	m_Guide.setLocalTranslation(startPos * 500.0);
	
	for (int i = 0; i < m_stateDim; i++)
	{
		m_state[i] = 0.0;
		m_stateDot[i] = 0.0;
	}

	m_force = 0.0;
	m_torque = 0.0;
	m_thetad = 0.0;
	m_vd = 0.0;
}

///////////////////////////////////////////////////

inline void ClampAngle(double& angle)
{
	while (angle > M_PI)
	{
		angle -= M2_PI;
	}
	while (angle < -M_PI)
	{
		angle += M2_PI;
	}
}

vec3 BehaviorController::globalPosToBodyPos(vec3 globalPos)
{
	vec3 Pos = getPosition();
	//translate
	vec3 bodyPos = globalPos - Pos;

	//rotate
	mat3 rotation;
	rotation.FromEulerAngles(mat3::RotOrder::XYZ, vec3(0.0, m_Euler[_Y] - (M_PI/2.0), 0));
	bodyPos = rotation * bodyPos;
	return bodyPos;
}

vec3 BehaviorController::bodyPosToGlobalPos(vec3 bodyPos)
{
	//rotate back
	mat3 rotation;
	rotation.FromEulerAngles(mat3::RotOrder::XYZ, vec3(0.0, (M_PI / 2.0) - m_Euler[_Y], 0));
	vec3 globalPos = rotation * bodyPos;

	//translate
	vec3 Pos = getPosition();
	globalPos = globalPos + Pos;

	return globalPos;
}

void BehaviorController::sense(double deltaT)
{
	if (mpActiveBehavior)
	{
		// find the agents in the neighborhood of the current character.
	}
	
}

void BehaviorController::control(double deltaT)
// Given the active behavior this function calculates a desired velocity vector (Vdesired).  
// The desired velocity vector is then used to compute the desired speed (vd) and direction (thetad) commands

{

	if (mpActiveBehavior)
	{ 
		m_Vdesired = mpActiveBehavior->calcDesiredVel(this);
		m_Vdesired[1] = 0;

		//  force and torque inputs are computed from vd and thetad as follows:
		//              Velocity P controller : force = mass * Kv * (vd - v)
		//              Heading PD controller : torque = Inertia * (-Kv * thetaDot +Kp * (thetad - theta))
		//  where the values of the gains Kv and Kp are different for each controller

		// TODO: insert your code here to compute m_force and m_torque

		// used to calculate kV, KA, cA - just needed to do once - set in parameters
		/*
		float tSettleV = 0.4;
		float dampingV = 1;
		float natFreqV = 4.0 / (dampingV * tSettleV);
		float kV = natFreqV * natFreqV * gMass;
		float tSettleA = 0.25;
		float dampingA = 1;
		float natFreqA = 4.0 / (dampingA * tSettleA);
		float kA = natFreqA * natFreqA * gInertia;
		float cA = 2.0 * natFreqA * gInertia;
		*/

		m_vd = m_Vdesired.Length();
		m_force = gMass * gVelKv * (vec3(0,0,m_vd) - m_VelB);
		if (m_force.Length() > gMaxForce) m_force = m_force.Normalize() * gMaxForce;

		m_thetad = atan2(m_Vdesired[_Z], m_Vdesired[_X]);

		//make sure we aren't going the long way around
		float thetaDiff = m_thetad - m_Euler[_Y];
		if (abs(thetaDiff) > M_PI) {
			if (m_thetad < 0) m_thetad += M_PI * 2.0;
			else m_thetad -= M_PI * 2.0;
		}

		m_torque = gInertia * (-gOriKp * m_state[AVEL] + gOriKv* (vec3(0,m_thetad,0) - m_Euler));
		if (m_torque.Length() > gMaxTorque) m_torque.Normalize() * gMaxTorque;

		// when agent desired agent velocity and actual velocity < 2.0 then stop moving
		if (m_vd < 2.0 &&  m_state[VEL][_Z] < 2.0)
		{
			m_force[2] = 0.0;
			m_torque[1] = 0.0;
		}
	}
	else
	{
		m_force[_Z] = 2.0;
		m_torque[_Y] = 2.0;
	}

	//std::cout << "Global Vel Desired: " << m_Vdesired << "Local Vel Z desired:  " << m_vd << " current velocity body: " << m_VelB << std::endl;
	//std::cout << "t: " << m_Euler[_Y] << " tD: " << m_thetad << " avel " << m_state[AVEL] << " torque" << m_torque[_Y] << std::endl;
	// set control inputs to current force and torque values
	m_controlInput[0] = m_force;
	m_controlInput[1] = m_torque;
}

void BehaviorController::act(double deltaT)
{
	computeDynamics(m_state, m_controlInput, m_stateDot, deltaT);
	
	int EULER = 0;
	int RK2 = 1;
	updateState(deltaT, EULER);
}


void BehaviorController::computeDynamics(vector<vec3>& state, vector<vec3>& controlInput, vector<vec3>& stateDot, double deltaT)
// Compute stateDot vector given the control input and state vectors
//  This function sets derive vector to appropriate values after being called
{
	vec3& force = controlInput[0];
	vec3& torque = controlInput[1];

	// Compute the stateDot vector given the values of the current state vector and control input vector
	// TODO: add your code here
	stateDot[POS][_X] = state[VEL][_Z] * cos(state[ORI][_Y]);
	stateDot[POS][_Y] = 0;
	stateDot[POS][_Z] = state[VEL][_Z] * sin(state[ORI][_Y]);

	stateDot[VEL][_X] = 0;
	stateDot[VEL][_Y] = 0;
	stateDot[VEL][_Z] = force[_Z] / gMass;

	stateDot[ORI][_X] = 0;
	stateDot[ORI][_Y] = state[AVEL][_Y];
	stateDot[ORI][_Z] = 0;

	stateDot[AVEL][_X] = 0;
	stateDot[AVEL][_Y] = torque[_Y]/ gInertia;
	stateDot[AVEL][_Z] = 0;

}

void BehaviorController::updateState(float deltaT, int integratorType)
{
	//  Update the state vector given the m_stateDot vector using Euler (integratorType = 0) or RK2 (integratorType = 1) integratio
	//  this should be similar to what you implemented in the particle system assignment

	// TODO: add your code here
	switch (integratorType) {
	case 0:
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 3; j++) {
				m_state[i][j] += m_stateDot[i][j] * deltaT;
			}
		}

		break;
	}

	//  Perform validation check to make sure all values are within MAX values
	// TODO: add your code here
	if (m_state[VEL].Length() > gMaxSpeed) {
		m_state[VEL] = gMaxSpeed * m_state[VEL] / m_state[VEL].Length();
	}


	if (m_state[AVEL].Length() > gMaxAngularSpeed) {
		m_state[AVEL] = gMaxAngularSpeed * m_state[AVEL] / m_state[AVEL].Length();
	}

	//make sure we are between - PI and Pi for Euler Angales
	while (m_state[ORI][_Y] < -M_PI) m_state[ORI][_Y] += M_PI * 2.0;
	while (m_state[ORI][_Y] > M_PI) m_state[ORI][_Y] -= M_PI * 2.0;


	//  given the new values in m_state, these are the new component state values 
	m_Pos0 = m_state[POS];
	m_Euler = m_state[ORI];
	m_VelB = m_state[VEL];
	m_AVelB = m_state[AVEL];
	m_Vel0 = vec3(cos(m_state[ORI][_Y]) *m_state[VEL][_Z], 0, sin(m_state[ORI][_Y]) * m_state[VEL][_Z]);

	// update the guide orientation
	// compute direction from nonzero velocity vector
	vec3 dir;
	if (m_Vel0.Length() < 1.0)
	{
		dir = m_lastVel0;
		dir.Normalize();;
		m_state[ORI] = atan2(dir[_Z], dir[_X]);
	}
	else
	{
		dir = m_Vel0;
		m_lastVel0 = m_Vel0;
	}

	dir.Normalize();
	vec3 up(0.0, 1.0, 0.0);
	vec3 right = up.Cross(dir);
	right.Normalize();
	mat3 rot(right, up, dir);
	m_Guide.setLocalRotation(rot.Transpose());
	m_Guide.setLocalTranslation(m_Guide.getLocalTranslation() + m_Vel0*deltaT);

}


void BehaviorController::setTarget(AJoint& target)
{
	m_pBehaviorTarget = &target;
	for (unsigned int i = 0; i < m_BehaviorList.size(); i++)
	{
		BehaviorType index = (BehaviorType) i;
		m_BehaviorList[index]->setTarget(m_pBehaviorTarget);
	}



}

void BehaviorController::setActiveBehavior(Behavior* pBehavior)
{
	mpActiveBehavior = pBehavior;
}

void BehaviorController::setActiveBehaviorType(BehaviorType type)
{
	m_BehaviorType = type;
	Behavior* pActiveBehavior = m_BehaviorList[type];
	setActiveBehavior(pActiveBehavior);

}

void BehaviorController::display()
{ // helps with debugging behaviors.  red line is actual velocity vector, green line is desired velocity vector
	
	vec3 pos = getPosition();
	double scale = 1.0;
	vec3 vel = scale* getVelocity();
	double velMag = vel.Length();
	vec3 dvel = scale* getDesiredVelocity();
	vec3 angle = getOrientation() * (180.0 / 3.14159);

	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(pos[0], pos[1], pos[2]);
	glVertex3f(pos[0] + vel[0], pos[1] + vel[1], pos[2] + vel[2]);
	glColor3f(0, 1, 0);
	glVertex3f(pos[0], pos[1], pos[2]);
	glVertex3f(pos[0] + dvel[0], pos[1] + dvel[1], pos[2] + dvel[2]);
	glEnd();

	if (this->isLeader())
		glColor3f(0, 0, 1);
	else glColor3f(0.5, 0, 0);

	glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2]);
	glRotatef(90 - angle[1], 0, 1, 0);
	glutSolidCone(40, 80, 10, 10);
	glutSolidSphere(35, 10, 10);
	glPopMatrix();

	BehaviorType active = getActiveBehaviorType();
	Behavior* pBehavior = m_BehaviorList[active];
	pBehavior->display(this);

}
