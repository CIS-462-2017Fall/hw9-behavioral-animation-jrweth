#include "aBehaviors.h"

#include <math.h>
#include "GL/glew.h"
#include "GL/glut.h"

// Base Behavior
///////////////////////////////////////////////////////////////////////////////
Behavior::Behavior()
{
}

Behavior::Behavior( char* name) 
{
	m_name = name;
	m_pTarget = NULL;
}

Behavior::Behavior( Behavior& orig) 
{
	m_name = orig.m_name;
	m_pTarget = NULL;
}

string& Behavior::GetName() 
{
    return m_name;
}

// Behaviors derived from Behavior
//----------------------------------------------------------------------------//
// Seek behavior
///////////////////////////////////////////////////////////////////////////////
// For the given the actor, return a desired velocity in world coordinates
// Seek returns a maximum velocity towards the target
// m_pTarget contains target world position
// actor.getPosition() returns Agent's world position


Seek::Seek( AJoint* target) 
{
	m_name = "seek";
	m_pTarget = target;

}

Seek::Seek( Seek& orig) 
{
	m_name = "seek";
	m_pTarget = orig.m_pTarget;
}


Seek::~Seek()
{
}

vec3 Seek::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 targetPos = m_pTarget->getLocalTranslation();
	vec3 actorPos = actor->getPosition();

	// TODO: add your code here to compute Vdesired
	Vdesired = (targetPos - actorPos);
	Vdesired.Normalize();
	Vdesired = Vdesired * actor->gMaxSpeed;


	return Vdesired;
}


// Flee behavior
///////////////////////////////////////////////////////////////////////////////
// For the given the actor, return a desired velocity in world coordinates
// Flee calculates a a maximum velocity away from the target
// m_pTarget contains target world position
// actor.getPosition() returns Agent's world position

Flee::Flee( AJoint* target) 
{
	m_name = "flee";
	m_pTarget = target;
}

Flee::Flee( Flee& orig) 
{
	m_name = "flee";
	m_pTarget = orig.m_pTarget;
}

Flee::~Flee()
{
}

vec3 Flee::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 targetPos = m_pTarget->getLocalTranslation();
	vec3 actorPos = actor->getPosition();

	// TODO: add your code here to compute Vdesired
	Vdesired = (actorPos -targetPos);
	Vdesired.Normalize();
	Vdesired = Vdesired * actor->gMaxSpeed;




	return Vdesired;

}

// Arrival behavior
///////////////////////////////////////////////////////////////////////////////
// Given the actor, return a desired velocity in world coordinates
// Arrival returns a desired velocity vector whose speed is proportional to
// the actors distance from the target
// m_pTarget contains target world position
// actor.getPosition() returns Agent's world position
//  Arrival strength is in BehavioralController::KArrival


Arrival::Arrival( AJoint* target) 
{
	m_name = "arrival";
	m_pTarget = target;
}

Arrival::Arrival( Arrival& orig) 
{
	m_name = "arrival";
	m_pTarget = orig.m_pTarget;
}

Arrival::~Arrival()
{
}

vec3 Arrival::calcDesiredVel( BehaviorController* actor)
{
	vec3 targetPos = m_pTarget->getLocalTranslation();
	vec3 actorPos = actor->getPosition();
	vec3 Vdesired = actor->KArrival * (targetPos - actorPos);

	return Vdesired;
}


// Departure behavior
///////////////////////////////////////////////////////////////////////////////
// Given the actor, return a desired velocity in world coordinates
// Arrival returns a desired velocity vector whose speed is proportional to
// 1/(actor distance) from the target
// m_pTarget contains target world position
// actor.getPosition() returns Agent's world position
//  Departure strength is in BehavioralController::KDeparture

Departure::Departure(AJoint* target) 
{
	m_name = "departure";
	m_pTarget = target;
}

Departure::Departure( Departure& orig) 
{
	m_name = "departure";
	m_pTarget = orig.m_pTarget;
}

Departure::~Departure()
{
}

vec3 Departure::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 targetPos = m_pTarget->getLocalTranslation();
	vec3 actorPos = actor->getPosition();

	// TODO: add your code here to compute Vdesired
	Vdesired = (actorPos - targetPos);
	float speed = actor->KDeparture / Vdesired.Length();
	Vdesired = speed * Vdesired / Vdesired.Length();


	return Vdesired;
}


// Avoid behavior
///////////////////////////////////////////////////////////////////////////////
//  For the given the actor, return a desired velocity in world coordinates
//  If an actor is near an obstacle, avoid adds a normal response velocity to the 
//  the desired velocity vector computed using arrival
//  Agent bounding sphere radius is in BehavioralController::radius
//  Avoidance parameters are  BehavioralController::TAvoid and BehavioralController::KAvoid

Avoid::Avoid(AJoint* target, vector<Obstacle>* obstacles) 
{
	m_name = "avoid";
	m_pTarget = target;
	mObstacles = obstacles;
}

Avoid::Avoid( Avoid& orig) 
{
	m_name = "avoid";
	m_pTarget = orig.m_pTarget;
	mObstacles = orig.mObstacles;
}

Avoid::~Avoid()
{
}

vec3 Avoid::calcDesiredVel( BehaviorController* actor)
{

	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	m_actorPos = actor->getPosition();
	m_actorVel = actor->getVelocity();

	//TODO: add your code here
	// Step 1. compute initial value for Vdesired = Varrival so agent moves toward target
	Arrival arrival(m_pTarget);
	Vdesired = arrival.calcDesiredVel(actor);

	vec3 Vavoid(0, 0, 0);
	//TODO: add your code here to compute Vavoid 



	// Step 2. compute Lb
	//TODO: add your code here
	float Lb = actor->getVelocity().Length() * actor->TAvoid;



	// Step 3. find closest obstacle 
	//TODO: add your code here
	float closestDistance = 10000000;
	vec3 closestPos;
	float closestRad;
	for (int i = 0; i < mObstacles->size(); i++) {
		vec3 obstaclePos = mObstacles->operator[](i).m_Center.getLocalTranslation();
		//convert to body coordinates
		vec3 obstaclePosB = actor->globalPosToBodyPos(obstaclePos);

		
		//ignore if it is behind us
		if (obstaclePosB[_Z] < 0) continue;
		//ignore if it the distance is greater than closest found
		if (obstaclePosB.Length() > closestDistance) continue;

		m_obstaclePos = obstaclePos;
		closestDistance = obstaclePosB.Length();
		closestPos = obstaclePosB;
		closestRad = mObstacles->operator[](i).m_Radius;
	}

	//didn't find anything in front of us
	if (closestDistance == 10000000) return Vdesired;

	//target is closer than obstacle
	if (closestDistance > (m_actorPos - m_pTarget->getLocalTranslation()).Length()) return Vdesired;


	float rad = actor->gAgentRadius;
	// Step 4. determine whether agent will collide with closest obstacle (only consider obstacles in front of agent)
	//TODO: add your code here

	//check for collistions
	//check too far away on Z axis
	if (closestPos[_Z] > Lb + closestRad + rad) return Vdesired;

	//check too far away on Z axis
	if (closestPos[_X] > Lb + closestRad + rad) return Vdesired;


	// Step 5.  if potential collision detected, compute Vavoid and set Vdesired = Varrival + Vavoid
	//TODO: add your code here
	float Dx = closestPos[_X];
	Vavoid = vec3(-Dx,0,0);
	//translate back into global coords
	Vavoid = actor->bodyPosToGlobalPos(Vavoid) - m_actorPos;
	Vavoid.Normalize();
	
	float magnitude = 0;
	if (abs(Dx) <= closestRad + rad) {
		magnitude = actor->KAvoid * (closestRad + rad - abs(Dx)) / (closestRad + rad);
	}
	//std::cout << Vavoid << ", magnitude" << magnitude << ", " << Vdesired << std::endl;
	Vavoid = magnitude * Vavoid;


	Vdesired = Vdesired + Vavoid;



	return Vdesired;
	
}

void Avoid::display( BehaviorController* actor)
{
	//  Draw Debug info
	vec3 angle = actor->getOrientation();
	vec3 vel = actor->getVelocity();
	vec3 dir = vec3(cos(angle[1]), 0, sin(angle[1]));
	vec3 probe = dir * (vel.Length()/BehaviorController::gMaxSpeed)*BehaviorController::TAvoid;
	
	glBegin(GL_LINES);
	glColor3f(0, 0, 1);
	glVertex3f(m_actorPos[0], m_actorPos[1], m_actorPos[2]);
	glVertex3f(m_obstaclePos[0], m_obstaclePos[1], m_obstaclePos[2]);
	glColor3f(0, 1, 1);
	glVertex3f(m_actorPos[0], m_actorPos[1], m_actorPos[2]);
	glVertex3f(m_actorPos[0] + probe[0], m_actorPos[1] + probe[1], m_actorPos[2] + probe[2]);
	glEnd();
}


// Wander Behavior
///////////////////////////////////////////////////////////////////////////////
// For the given the actor, return a desired velocity in world coordinates
// Wander returns a desired velocity vector whose direction changes at randomly from frame to frame
// Wander strength is in BehavioralController::KWander

Wander::Wander() 
{
	m_name = "wander";
	m_Wander = vec3(1.0, 0.0, 0.0);
}

Wander::Wander( Wander& orig) 
{
	m_name = "wander";
	m_Wander = orig.m_Wander;
}

Wander::~Wander()
{
}

vec3 Wander::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 actorPos = actor->getPosition();

	//make sure m_Wander is scaled to the desired size
	m_Wander = actor->KWander * m_Wander / m_Wander.Length();

	// Step. 1 find a random direction
	//TODO: add your code here
	vec3 dir(rand() - RAND_MAX / 2, 0.0, rand() - RAND_MAX / 2);
  

	// Step2. scale it with a noise factor
	//TODO: add your code here
	dir = actor->KNoise * dir / dir.Length();


	// Step3. change the current Vwander  to point to the random direction
	//TODO: add your code here
	m_Wander = m_Wander + dir;


	// Step4. scale the new wander velocity vector and add it to the nominal velocity
	//TODO: add your code here
	m_Wander = actor->KWander * m_Wander / m_Wander.Length();
	vec3 v0 = actor->getVelocity();
	if (v0.Length() == 0) {
		Vdesired = m_Wander;
	}
	else {
		v0 = actor->KWander * 1.5 * v0 / v0.Length();
		Vdesired = v0 + m_Wander;
	}




	return Vdesired;
}


// Alignment behavior
///////////////////////////////////////////////////////////////////////////////
// For the given the actor, return a desired velocity vector in world coordinates
// Alignment returns the average velocity of all active agents in the neighborhood
// agents[i] gives the pointer to the ith agent in the environment
// Alignment parameters are in BehavioralController::RNeighborhood and BehavioralController::KAlign


Alignment::Alignment(AJoint* target, vector<AActor>* agents) 
{
	m_name = "alignment";
	m_pAgentList = agents;
	m_pTarget = target;
}



Alignment::Alignment( Alignment& orig) 
{
	m_name = orig.m_name;
	m_pAgentList = orig.m_pAgentList;
	m_pTarget = orig.m_pTarget;

}

Alignment::~Alignment()
{
}

vec3 Alignment::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 targetPos = m_pTarget->getLocalTranslation();
	vec3 actorPos = actor->getPosition();
	vector<AActor>& agentList = *m_pAgentList;
	

	// compute Vdesired 
	
	// Step 1. compute value of Vdesired for fist agent (i.e. m_AgentList[0]) using an arrival behavior so it moves towards the target
	 
	BehaviorController* leader = agentList[0].getBehaviorController(); // first agent is the leader
	//TODO: add your code here

	if (actor == leader) {
		Arrival arrival(m_pTarget);
		return arrival.calcDesiredVel(actor);
	}

	//sum up the 
	int numInNeighborhood = 0;
	for (int i = 0; i < agentList.size(); i++) {
		BehaviorController* agent = agentList[i].getBehaviorController();
		float distance = (agent->getPosition() - actorPos).Length();
		if (actor != agent && distance < actor->gKNeighborhood) {
			Vdesired = Vdesired + agent->getVelocity();
			numInNeighborhood++;
		}
	}
	if (numInNeighborhood > 0) {
		Vdesired = actor->KAlignment * Vdesired / numInNeighborhood;
	}
	
	

	// Step 2. if not first agent compute Valign as usual
	//TODO: add your code here
	
	
	return Vdesired;
}

// Separation behavior
///////////////////////////////////////////////////////////////////////////////
// For the given te actor, return a desired velocity vector in world coordinates
// Separation tries to maintain a constant distance between all agents
// within the neighborhood
// agents[i] gives the pointer to the ith agent in the environment
// Separation settings are in BehavioralController::RNeighborhood and BehavioralController::KSeperate

 

Separation::Separation( AJoint* target,  vector<AActor>* agents) 
{
	m_name = "separation";
	m_AgentList = agents;
	m_pTarget = target;
}

Separation::~Separation()
{
}

Separation::Separation( Separation& orig) 
{
	m_name = "separation";
	m_AgentList = orig.m_AgentList;
	m_pTarget = orig.m_pTarget;
}

vec3 Separation::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 targetPos = m_pTarget->getLocalTranslation();
	vec3 actorPos = actor->getPosition();
	vector<AActor>& agentList = *m_AgentList;
	
	//sum up the 
	for (int i = 0; i < agentList.size(); i++) {
		BehaviorController* agent = agentList[i].getBehaviorController();
		float distance = (actorPos - agent->getPosition()).Length();
		if (actor != agent && distance < actor->gKNeighborhood) {
			Vdesired = Vdesired + (actorPos - agent->getPosition()) / (distance * distance);
		}
	}
	Vdesired = actor->KSeparation * Vdesired;


	if (Vdesired.Length() < 5.0)
		Vdesired = 0.0;
	
	return Vdesired;
}


// Cohesion behavior
///////////////////////////////////////////////////////////////////////////////
// For the given actor, return a desired velocity vector in world coordinates
// Cohesion moves actors towards the center of the group of agents in the neighborhood
//  agents[i] gives the pointer to the ith agent in the environment
//  Cohesion parameters are in BehavioralController::RNeighborhood and BehavioralController::KCohesion


Cohesion::Cohesion( vector<AActor>* agents) 
{
	m_name = "cohesion";
	m_AgentList = agents;
}

Cohesion::Cohesion( Cohesion& orig) 
{
	m_name = "cohesion";
	m_AgentList = orig.m_AgentList;
}

Cohesion::~Cohesion()
{
}

vec3 Cohesion::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 actorPos = actor->getPosition();
	vector<AActor>& agentList = *m_AgentList;
	
	// compute Vdesired = Vcohesion
	vec3 center = vec3(0.0, 0.0, 0.0);
	int numInNeighborhood = 0;

	//sum up the 
	for (int i = 0; i < agentList.size(); i++) {
		BehaviorController* agent = agentList[i].getBehaviorController();
		float distance = (actorPos - agent->getPosition()).Length();
		if (actor != agent && distance < actor->gKNeighborhood) {
			center = center + agent->getPosition();
			numInNeighborhood++;
		}
	}
	if (numInNeighborhood > 0) {
		Vdesired = actor->KCohesion * (center/numInNeighborhood - actorPos);
	}



	if (Vdesired.Length() < 5.0)
		Vdesired = 0.0;

	return Vdesired;


}

// Flocking behavior
///////////////////////////////////////////////////////////////////////////////
// For the given actor, return a desired velocity vector  in world coordinates
// Flocking combines separation, cohesion, and alignment behaviors
//  Utilize the Separation, Cohesion and Alignment behaviors to determine the desired velocity vector


Flocking::Flocking( AJoint* target,  vector<AActor>* agents) 
{
	m_name = "flocking";
	m_AgentList = agents;
	m_pTarget = target;
}

Flocking::Flocking( Flocking& orig) 
{
	m_name = "flocking";
	m_AgentList = orig.m_AgentList;
	m_pTarget = orig.m_pTarget;
}

Flocking::~Flocking()
{
}

vec3 Flocking::calcDesiredVel( BehaviorController* actor)
{
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 actorPos = actor->getPosition();
	vector<AActor>& agentList = *m_AgentList;

	// compute Vdesired = Vflocking
	// TODO: add your code here

	Separation sep(m_pTarget, m_AgentList);
	Cohesion coh(m_AgentList);
	Alignment align(m_pTarget, m_AgentList);
	Vdesired = sep.calcDesiredVel(actor) + coh.calcDesiredVel(actor) + align.calcDesiredVel(actor);
	

	return Vdesired;
}

//	Leader behavior
///////////////////////////////////////////////////////////////////////////////
// For the given actor, return a desired velocity vector in world coordinates
// If the agent is the leader, move towards the target; otherwise, 
// follow the leader at a set distance behind the leader without getting to close together
//  Utilize Separation and Arrival behaviors to determine the desired velocity vector
//  You need to find the leader, who is always agents[0]

Leader::Leader( AJoint* target, vector<AActor>* agents) 
{
	m_name = "leader";
	m_AgentList = agents;
	m_pTarget = target;
}

Leader::Leader( Leader& orig) 
{
	m_name = "leader";
	m_AgentList = orig.m_AgentList;
	m_pTarget = orig.m_pTarget;
}

Leader::~Leader()
{
}

vec3 Leader::calcDesiredVel( BehaviorController* actor)
{
	
	vec3 Vdesired = vec3(0.0, 0.0, 0.0);
	vec3 actorPos = actor->getPosition();
	vector<AActor>& agentList = *m_AgentList;

	// TODO: compute Vdesired  = Vleader
	// followers should stay directly behind leader at a distance of -200 along the local z-axis

	float CSeparation = 4.0;  float CArrival = 2.0;

	BehaviorController* leader = agentList[0].getBehaviorController(); // first agent is the leader
	mat3 Rmat = leader->getGuide().getLocalRotation();  // is rotattion matrix of lead agent


	if (actor == leader) {
		Arrival arrival(m_pTarget);
		return arrival.calcDesiredVel(actor);
	}

	//get the point behind the leader
	vec3 targetPos = vec3(0.0, 0.0, -200.0);
	//convert to world coordinates
	targetPos = leader->bodyPosToGlobalPos(targetPos);

	//create the target for the arrival controller
	AJoint* target = new AJoint();
	target->setLocalTranslation(targetPos);
	Arrival arrival(target);

	//create the separation controller
	Separation sep(target, m_AgentList);
	Vdesired = arrival.calcDesiredVel(actor) + sep.calcDesiredVel(actor);





	return Vdesired;
}

///////////////////////////////////////////////////////////////////////////////

