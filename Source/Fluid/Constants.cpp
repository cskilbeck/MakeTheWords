//////////////////////////////////////////////////////////////////////

#include "pch.h"	

//////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
int NumParticles					= 600;
#else
int NumParticles					= 6000;
#endif

float ParticleSize					= 20;
float StiffnessK					= 0.01f;
float StiffnessRatio				= 10.0f;
float RestDensity					= 4.0f;
float SpringCoefficient				= 0.1f;
float YieldRatio					= 0.1f;
float Plasticity					= 0.5f;
float DeltaTimeScalingFactor		= 100.0f;
float ViscositySigma				= 0.01f;
float ViscosityBeta					= 0.02f;
float WallSpringCoefficient			= 1.0f;

float NearStiffnessK				= StiffnessK * StiffnessRatio;

float SmoothingRadius				= ParticleSize;
float SmoothingRadiusSquared		= SmoothingRadius * SmoothingRadius;
