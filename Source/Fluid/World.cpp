//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

Vec2 const World::kGravity(0,0.1f);

//////////////////////////////////////////////////////////////////////

World::World()
{
}

void World::Release()
{
	Delete(mParticles);
	mGrid.Release();
	mSpringFlags.Release();
	mNeighbourhood.Release();
	mNeighbourhoodOther.Release();
	mSprings.Release();
}

void World::Init(float w, float h)
{
	Release();

	mWidth = w;
	mHeight = h;

	mGrid.Init(w, h, ParticleSize);

	mSpringFlags.Init(NumParticles, NumParticles);

	mNeighbourhood.Init(NumParticles);
	mNeighbourhoodOther.Init(NumParticles);

	mParticles = new Particle[NumParticles];

	mSprings.Init(NumParticles * Neighbourhood::kMaxNeighboursPerParticle);

	int sq = (int)sqrtf((float)NumParticles);
	int sq2 = sq / 2;

	float midx = w / 2;
	float midy = h / 2;

	for(int i=0; i<NumParticles; ++i)
	{
		Particle &p = mParticles[i];

		float x = midx + ((i % sq) - sq2) * (SmoothingRadius / 3);
		float y = midy + ((i / sq) - sq2) * (SmoothingRadius / 3);

		p.mIndex = i;
		p.mDensity = 0;
		p.mNearDensity = 0;
		p.mMass = 0;
		p.mForce.reset();
		p.mForce.x = 2;
		p.mForce.y = -4;
		p.mViscosityBeta = ViscosityBeta;
		p.mViscositySigma = ViscositySigma;
		p.mNeighbourCount = 0;
		p.mOldPosition = p.mPosition = Vec2(x, y);
		p.mCell = null;
	}

	timeBank = 0;
	mTimer.Reset();
}

//////////////////////////////////////////////////////////////////////

void World::Step()
{
	float delta = (float)mTimer.GetDelta();
	delta = min(delta, 0.1f);

	timeBank += delta;

	deltaTime = 0.01f;
	deltaTimeScaled = deltaTime * DeltaTimeScalingFactor;
	deltaTimeScaledSquared = deltaTimeScaled * deltaTimeScaled;

#if DEBUG || 1
	timeBank = deltaTime + (deltaTime/2);
#endif

	//while(timeBank > deltaTime)
	{
		//CalculateViscoElasticity();
		UpdateGrid();
		CalculateDensity();
		CalculatePressure();
		UpdateParticles();
		DoCollision();

		timeBank -= deltaTime;
	}
}

//////////////////////////////////////////////////////////////////////

void World::UpdateGrid()
{
	mGrid.Reset();

	for(int i=0; i<NumParticles; ++i)
	{
		mGrid.CountPoint(mParticles[i].mPosition);
	}

	mGrid.CalcOffsets();

	for(int i=0; i<NumParticles; ++i)
	{
		mGrid.AddPoint(mParticles[i], i);
	}
}

//////////////////////////////////////////////////////////////////////

void World::CalculateViscoElasticity()
{
	for(int i=0; i<NumParticles; ++i)
	{
		Particle &p = mParticles[i];

		int neighbourEnd = p.mNeighbourListIndex + p.mNeighbourCount;

		for(int ni=p.mNeighbourListIndex; ni<neighbourEnd; ++ni)
		{
			Neighbour &n = mNeighbourhood.GetNeighbour(ni);
			Particle *np = n.mParticle;
			if(!mSpringFlags.IsSet(p.mIndex, np->mIndex))
			{
				mSpringFlags.Set(p.mIndex, np->mIndex);
				Spring *s = mSprings.Alloc();
				if(s != null)
				{
					s->mP0 = &p;
					s->mP1 = np;
					s->mRestLength = SmoothingRadius;
					s->mCoefficient = SpringCoefficient;
				}
			}
		}
	}

	Spring *n;

	for(Spring *s = mSprings.mBusyList.head; s != null; s = n)
	{
		n = s->next;

		float td = YieldRatio * s->mRestLength;

		float dist = (s->mP0->mPosition - s->mP1->mPosition).length();

		if(dist > s->mRestLength + td)
		{
			s->mRestLength += deltaTimeScaled * Plasticity * (dist - s->mRestLength - td);

			if(s->mRestLength > SmoothingRadius)
			{
				mSpringFlags.Clear(s->mP0->mIndex, s->mP1->mIndex);
				mSprings.mBusyList.Remove(s);
				mSprings.mFreeList.AddTail(s);
			}
		}
		else if (dist < s->mRestLength - td)
		{
			s->mRestLength -= deltaTimeScaled * Plasticity * (s->mRestLength - td - dist );
		}
	}

	for(Spring *s = mSprings.mBusyList.head; s != null; s = s->next)
	{
		Vec2 displacement = s->CalculateForce() * (1.0f - s->mRestLength / SmoothingRadius) * deltaTimeScaledSquared;

		s->mP0->mPosition += displacement;
		s->mP1->mPosition -= displacement;
	}
}

//////////////////////////////////////////////////////////////////////

void World::UpdateParticles()
{
	for(int i=0; i<NumParticles; ++i)
	{
		mParticles[i].UpdateVelocity(deltaTimeScaled);
	}

	CalculateViscosity();

	for(int i=0; i<NumParticles; ++i)
	{
		mParticles[i].UpdatePosition(deltaTimeScaled);
		mParticles[i].mForce = kGravity;		
	}
}

//////////////////////////////////////////////////////////////////////

void World::CreateNeighbours(Neighbourhood &n, Particle &p, Particle &q, float &density, float &nearDensity)
{
	Vec2 diff(p.mPosition - q.mPosition);

	float diffLengthSquared = diff.lengthSquared();

	if(diffLengthSquared < SmoothingRadiusSquared)
	{
		float diffLength = sqrtf(diffLengthSquared);

		float smoothingKernel = 1 - diffLength / SmoothingRadius;
		float smoothingKernel2 = smoothingKernel * smoothingKernel;
		float smoothingKernel3 = smoothingKernel2 * smoothingKernel;

		density += smoothingKernel2;
		nearDensity += smoothingKernel3;

		q.mDensity += smoothingKernel2;
		q.mNearDensity += smoothingKernel3;

		if(n.AddNeighbour(smoothingKernel, smoothingKernel2, diffLength, &q))
		{
			++p.mNeighbourCount;
		}
	}
}

void World::CalculateDensity()
{
	Neighbourhood &n2 = mNeighbourhood;
	Neighbourhood &n1 = mNeighbourhood;

	if(GetAsyncKeyState(VK_SHIFT))
	{
		n2.Reset();

		for(int i=0; i<NumParticles; ++i)
		{
			Particle &p = mParticles[i];

			float density = 0;
			float nearDensity = 0;

			p.mDensity = 0;
			p.mNearDensity = 0;
			p.mNeighbourCount = 0;
			p.mNeighbourListIndex = n2.mCurrentNeighbour;

			for(int j=i+1; j<NumParticles; ++j)
			{
				Particle &q = mParticles[j];

				CreateNeighbours(n2, p, q, density, nearDensity);
			}
			p.mDensity += density;
			p.mNearDensity += nearDensity;
		}
	}
	else
	{
		n1.Reset();

		for(int y = 0; y < mGrid.mHeight; ++y)
		{
			int cminy = y;
			int maxy = y + 1;
			int cmaxy = min(mGrid.mHeight - 1, maxy);

			for(int x = 0; x < mGrid.mWidth; ++x)
			{
				int maxx = x + 1;
				int cmaxx = min(mGrid.mWidth - 1, maxx);

				GridCell &cell = mGrid.GetCell(x, y);

				int end = cell.mStart + cell.mLength;

				for(int i = cell.mStart; i < end; ++i)
				{
					Particle &p = mParticles[mGrid.mIndices[i]];

					float density = 0;
					float nearDensity = 0;

					p.mDensity = 0;
					p.mNearDensity = 0;
					p.mNeighbourCount = 0;
					p.mNeighbourListIndex = n1.mCurrentNeighbour;	// unique visitation of that particle

					for(int j = i + 1; j < end; ++j)
					{
						Particle &q = mParticles[mGrid.mIndices[j]];
						CreateNeighbours(n1, p, q, density, nearDensity);
					}

					for(int y1 = cminy; y1 <= cmaxy; ++y1)
					{
						int Xoffset = x + (y - y1);

						int cxo = max(Xoffset, 0);
						cxo = min(cxo, mGrid.mWidth);

						for(int x1 = cxo; x1 <= cmaxx; ++x1)
						{
							if(x1 != x || y1 != y)
							{
								GridCell &cell2 = mGrid.GetCell(x1, y1);

								if(cell2.mLength > 0)
								{
									int end = cell2.mStart + cell2.mLength;

									for(int j = cell2.mStart; j < end; ++j)
									{
										Particle &q = mParticles[mGrid.mIndices[j]];

										CreateNeighbours(n1, p, q, density, nearDensity);
									}
								}
							}
						}
					}

					p.mDensity += density;
					p.mNearDensity += nearDensity;
				}
			}
		}
	}

	//int gh = 4;
	//int gw = 4;

	//for(int y = 0; y < gh; ++y)
	//{
	//	int miny = y;
	//	int maxy = y + 1;

	//	int cminy = max(0, miny);
	//	int cmaxy = min(gh - 1, maxy);

	//	for(int x = 0; x < gw; ++x)
	//	{
	//		int maxx = x + 1;
	//		int cmaxx = min(gw - 1, maxx);

	//		for(int y1 = cminy; y1 <= cmaxy; ++y1)
	//		{
	//			int Xoffset = x + (y - y1);

	//			int cxo = max(Xoffset, 0);
	//			cxo = min(cxo, gw-1);

	//			for(int x1 = cxo; x1 <= cmaxx; ++x1)
	//			{
	//				if(x1 != x || y1 != y)
	//				{
	//					TRACE("%d,%d,%d,%d\n", x,y,x1,y1);
	//				}
	//			}
	//		}
	//	}
	//}
}

//////////////////////////////////////////////////////////////////////

void World::CalculateViscosity()
{
	for(int i=0; i<NumParticles; ++i)
	{
		Particle &p = mParticles[i];

		int neighbourEnd = p.mNeighbourListIndex + p.mNeighbourCount;

		for(int ni=p.mNeighbourListIndex; ni<neighbourEnd; ++ni)
		{
			Neighbour &n = mNeighbourhood.GetNeighbour(ni);

			Particle *np = n.mParticle;

			Vec2 diff = np->mPosition - p.mPosition;

			float length = diff.length();

			float q = length / SmoothingRadius;

			diff /= length;

			float irv = (p.mVelocity - np->mVelocity).dot(diff);

			if(irv > 0)
			{
				Vec2 impulses = (diff * (np->mViscositySigma * irv + np->mViscosityBeta * irv * irv)) * (1-q) * deltaTimeScaled;

				p.mVelocity -= impulses;
				np->mVelocity += impulses;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////

void World::CalculatePressure()
{
	float dtsquared = deltaTime * deltaTime * DeltaTimeScalingFactor * DeltaTimeScalingFactor;

	for(int i=0; i<NumParticles; ++i)
	{
		Vec2 particlePressure(0,0);

		Particle &p = mParticles[i];

		float pressure = StiffnessK * (p.mDensity - RestDensity);

		float nearPressure = NearStiffnessK * p.mNearDensity;

		int neighbourEnd = p.mNeighbourListIndex + p.mNeighbourCount;

		for(int ni=p.mNeighbourListIndex; ni<neighbourEnd; ++ni)
		{
			Neighbour &n = mNeighbourhood.GetNeighbour(ni);

			if(n.mDistance != 0)
			{
				Particle *np = n.mParticle;

				Vec2 diff(np->mPosition - p.mPosition);

				float pressureScalar = (pressure * n.mSmoothingKernel + nearPressure * n.mSmoothingKernel2) * dtsquared;

				Vec2 pressureForce = diff / n.mDistance * pressureScalar;

				np->mForce += pressureForce;

				particlePressure -= pressureForce;
			}
		}
		p.mForce += particlePressure;
	}
}

//////////////////////////////////////////////////////////////////////
// the euler integrator allows us to directly modify the position, within reason
// this will be how come we can do proper collisions
// adding force reduces contact tension, should really be a parameter

void World::DoCollision()
{
	for(int i=0; i<NumParticles; ++i)
	{
		Particle &p = mParticles[i];

		if(p.mPosition.x < 0)
		{
			p.mPosition.x = 0;
			p.mForce.x = WallSpringCoefficient;
		}

		if(p.mPosition.x > mWidth)
		{
			p.mForce.x = -WallSpringCoefficient;
			p.mPosition.x = mWidth;
		}

		if(p.mPosition.y < 0)
		{
			p.mForce.y = WallSpringCoefficient;
			p.mPosition.y = 0;
		}

		if(p.mPosition.y > mHeight)
		{
			p.mForce.y = -WallSpringCoefficient;
			p.mPosition.y = mHeight;
		}
	}
}