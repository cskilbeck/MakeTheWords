//////////////////////////////////////////////////////////////////////

namespace glib
{

	//////////////////////////////////////////////////////////////////////

	struct Point
	{
		//////////////////////////////////////////////////////////////////////

		float x,y;

		//////////////////////////////////////////////////////////////////////

		Point()
		{
		}

		//////////////////////////////////////////////////////////////////////

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
		//////////////////////////////////////////////////////////////////////

		float m0, m1, m2, m3, m4, m5, m6;

		//////////////////////////////////////////////////////////////////////

		Matrix()
		{
		}

		//////////////////////////////////////////////////////////////////////

		Matrix(float a, float b, float c, float d, float e, float f)
		{
			m0 = a; m1 = b; m2 = c; m3 = d; m4 = e; m5 = f;
		}

		//////////////////////////////////////////////////////////////////////

		Matrix(Matrix const &a)
		{
			m0 = a.m0; m1 = a.m1; m2 = a.m2; m3 = a.m3; m4 = a.m4; m5 = a.m5;
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Translate(Point p) const
		{
			return Matrix(m0, m1, m2, m3, m4 + p.x, m5 + p.y);
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Rotate(float radians) const
		{
			float cos = cosf(radians);
			float sin = sinf(radians),
			float r00 = m0 * cos + m2 * -sin,
			float r01 = m1 * cos + m3 * -sin,
			float r10 = m0 * sin + m2 * cos,
			float r11 = m1 * sin + m3 * cos;
			return Matrix(r00, r01, r10, r11, m4, m5);
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Scale(Point s) const
		{
			return Matrix(m0 * s.x, m1 * s.x, m2 * s.y, m3 * s.y, m4, m5);
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Invert() const
		{
			float d = m0 * m3 - m1 * m2;
			if (d < 1.0e-6 && d > -1.0e-6)
			{
				return Matrix(*this);
			}
			d = 1.0f / d;
			return Matrix(m3 * d, m1 * -d, m2 * -d, m0 * d, (m2 * m5 - m3 * m4) * d, (m0 * m5 - m1 * m4) * -d);
		}

		//////////////////////////////////////////////////////////////////////

		Point Apply(Point const &p) const
		{
			return Point(p.x * m0 + p.y * m2 + m4, p.x * m1 + p.y * m3 + m5);
		}

		//////////////////////////////////////////////////////////////////////

		void Transform(Point p[], uint len) const
		{
			for(uint i = 0; i < len; ++i)
			{
				float x = p[i].x;
				float y = p[i].y;
				p[i].x = x * m0 + y * m2 + m4;
				p[i].y = x * m1 + y * m3 + m5;
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
			float cos = cosf(radians);
			float sin = sinf(radians);
			return Matrix(cos * scale.x, -sin * scale.x, sin * scale.y, cos * scale.y, trans.x, trans.y);
		}

	};

	//////////////////////////////////////////////////////////////////////

	inline Matrix operator * (Matrix const &a, Matrix const &b)
	{
		return Matrix(
			a.m0 * b.m0 + a.m2 * b.m1,
			a.m1 * b.m0 + a.m3 * b.m1,
			a.m0 * b.m2 + a.m2 * b.m3,
			a.m1 * b.m2 + a.m3 * b.m3,
			a.m0 * b.m4 + a.m2 * b.m5 + a.m4,
			a.m1 * b.m4 + a.m3 * b.m5 + a.m5);
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
		bool						closed;

		//////////////////////////////////////////////////////////////////////

		Entity()
			: parent(null)
			, position(0, 0)
			, size(0, 0)
			, pivot(0, 0)
			, scale(1, 1)
			, rotation(0)
			, z_index(0)
			, closed(false)
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

		virtual void OnClosing()
		{
		}

		//////////////////////////////////////////////////////////////////////

		virtual void OnClosed()
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

		void Close()
		{
			closed = true;
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
			for(auto i = children.begin(), _end = children.end(), n = i; ++n, i != _end; i = n)
			{
				if(i->closed)
				{
					i->OnClosing();
					children.remove(i);
					i->parent = null;
					i->OnClosed();
				}
			}
			children.sort();
			for(auto &e : children)
			{
				e.Draw(transform);
			}
		}
	};

	//////////////////////////////////////////////////////////////////////

}; // namespace glib

void Bob()
{
	Entity p;
}
