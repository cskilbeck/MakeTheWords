//////////////////////////////////////////////////////////////////////

struct Point
{
	float x,y;

	Point()
	{
	}

	Point(float _x, float _y)
		: x(_x)
		, y(_y)
	{
	}
};

//////////////////////////////////////////////////////////////////////

Point operator + (Point const &a, Point const &b)
{
	return Point(a.x + b.x, a.y + b.y);
}

//////////////////////////////////////////////////////////////////////

Point operator - (Point const &a, Point const &b)
{
	return Point(a.x - b.x, a.y - b.y);
}

//////////////////////////////////////////////////////////////////////

struct Matrix
{
	float m[6];

	//////////////////////////////////////////////////////////////////////

	Matrix()
	{
	}

	//////////////////////////////////////////////////////////////////////

	Matrix(float a, float b, float c, float d, float e, float f)
	{
		m[0] = a; m[1] = b; m[2] = c; m[3] = d; m[4] = e; m[5] = f;
	}

	//////////////////////////////////////////////////////////////////////

	Matrix(Matrix const &a)
	{
		m[0] = a.m[0]; m[1] = a.m[1]; m[2] = a.m[2]; m[3] = a.m[3]; m[4] = a.m[4]; m[5] = a.m[5];
	}

	//////////////////////////////////////////////////////////////////////

	Matrix Translate(Point p) const
	{
		return Matrix(m[0], m[1], m[2], m[3], m[4] + p.x, m[5] + p.y);
	}

	//////////////////////////////////////////////////////////////////////

	Matrix Rotate(float radians) const
	{
        float cos = cosf(radians);
		float sin = sinf(radians),
		float r00 = m[0] * cos + m[2] * -sin,
		float r01 = m[1] * cos + m[3] * -sin,
		float r10 = m[0] * sin + m[2] * cos,
		float r11 = m[1] * sin + m[3] * cos;
		return Matrix(r00, r01, r10, r11, m[4], m[5]);
	}

	//////////////////////////////////////////////////////////////////////

	Matrix Scale(Point s) const
	{
		return Matrix(m[0] * s.x, m[1] * s.x, m[2] * s.y, m[3] * s.y, m[4], m[5]);
	}

	//////////////////////////////////////////////////////////////////////

	Matrix Invert() const
	{
		float det = m[0] * m[3] - m[1] * m[2];
		if (det < 1.0e-6 && det > -1.0e-6)
		{
			return Matrix(*this);
		}
		det = 1.0f / det;
		return Matrix(
			m[3] * det,
			m[1] * -det,
			m[2] * -det,
			m[0] * det,
			(m[2] * m[5] - m[3] * m[4]) * det,
			(m[0] * m[5] - m[1] * m[4]) * -det);
	}

	//////////////////////////////////////////////////////////////////////

	Point Apply(Point const &p) const
	{
		return Point(p.x * m[0] + p.y * m[2] + m[4], p.x * m[1] + p.y * m[3] + m[5]);
	}

	//////////////////////////////////////////////////////////////////////

	void Transform(Point p[], uint len) const
	{
		for(uint i = 0; i < len; ++i)
		{
			float x = p[i].x;
			float y = p[i].y;
			p[i].x = x * m[0] + y * m[2] + m[4];
			p[i].y = x * m[1] + y * m[3] + m[5];
		}
	}

	//////////////////////////////////////////////////////////////////////

	static Matrix Identity()
	{
		return Matrix(1, 0, 0, 1, 0 ,0);
	}

	//////////////////////////////////////////////////////////////////////

	static Matrix RotTransScale(float radians, Point trans, Point scale)
	{
		Matrix m;
        float cos = cosf(radians);
		float sin = sinf(radians);
		m.m[0] = cos * scale.x;
		m.m[1] = -sin * scale.x;
		m.m[2] = sin * scale.y;
		m.m[3] = cos * scale.y;
		m.m[4] = trans.x;
		m.m[5] = trans.y;
		return m;
	}

};

//////////////////////////////////////////////////////////////////////

inline Matrix operator * (Matrix const &a, Matrix const &b)
{
	float const * const m = a.m;
	float const * const y = b.m;
	return Matrix(
		m[0] * y[0] + m[2] * y[1],
		m[1] * y[0] + m[3] * y[1],
		m[0] * y[2] + m[2] * y[3],
		m[1] * y[2] + m[3] * y[3],
		m[0] * y[4] + m[2] * y[5] + m[4],
		m[1] * y[4] + m[3] * y[5] + m[5]);
}

//////////////////////////////////////////////////////////////////////

inline Matrix operator * (Matrix const &a, Point const &s)
{
	return a.Scale(s);
}

//////////////////////////////////////////////////////////////////////

struct Entity : RefCount, chs::list_node<Entity>
{
	//////////////////////////////////////////////////////////////////////

	Entity *					parent;
	chs::linked_list<Entity>	children;
	Point						position;
	Point						size;
	Point						pivot;
	Point						scale;
	float						rotation;
	int							z_index;
	Matrix						transform;
	Matrix						inverse_transform;

	//////////////////////////////////////////////////////////////////////

	Entity()
		: parent(null)
		, position(0, 0)
		, size(0, 0)
		, pivot(0, 0)
		, scale(1, 1)
		, rotation(0)
		, z_index(0)
	{
	}

	//////////////////////////////////////////////////////////////////////

	virtual ~Entity()
	{
	}

	//////////////////////////////////////////////////////////////////////

	virtual void OnUpdate(float time, float delta_time)
	{
	}

	//////////////////////////////////////////////////////////////////////

	virtual void OnDraw()
	{
	}

	//////////////////////////////////////////////////////////////////////

	void AddChild(Entity &child)
	{
		children.push_back(child);
	}

	//////////////////////////////////////////////////////////////////////

	void AddSibling(Entity &sibling)
	{
		parent->AddChild(sibling);
	}

	//////////////////////////////////////////////////////////////////////

	void RemoveChild(Entity &child)
	{
		children.remove(child);
	}

	//////////////////////////////////////////////////////////////////////

	bool operator < (Entity const &a)
	{
		return z_index < a.z_index;
	}

	//////////////////////////////////////////////////////////////////////

	void Update(float time, float delta_time)
	{
		for(auto &e : children)
		{
			e.Update(time, delta_time);
		}
		OnUpdate(time, delta_time);
	}

	//////////////////////////////////////////////////////////////////////

	void Draw(Matrix &m)
	{
		Point p(-pivot.x * size.x, -pivot.y * size.y);
		transform = Matrix::Identity().Translate(position).Rotate(rotation).Scale(scale).Translate(p) * m;
		inverse_transform = transform.Invert();
		// add matrix to render queue
		OnDraw();
		children.sort();
		for(auto &e : children)
		{
			e.Draw(transform);
		}
	}

};

//////////////////////////////////////////////////////////////////////

void Bob()
{
	Entity p;
}
