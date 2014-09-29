//////////////////////////////////////////////////////////////////////

namespace glib
{
	//////////////////////////////////////////////////////////////////////

	struct Point
	{
		//////////////////////////////////////////////////////////////////////

		float x, y;

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

	Point operator + (Point const &a, Size const &s)
	{
		return Point(a.x + s.w, a.y + s.h);
	}

	//////////////////////////////////////////////////////////////////////

	Point operator - (Point const &a, Point const &b)
	{
		return Point(a.x - b.x, a.y - b.y);
	}

	//////////////////////////////////////////////////////////////////////

	struct Size
	{
		float w, h;

		Size()
		{
		}

		Size(float _w, float _h)
			: w(_w)
			, h(_h)
		{
		}
	};

	//////////////////////////////////////////////////////////////////////

	struct Rect: Point, Size
	{
		Rect()
		{
		}

		Rect(float _x, float _y, float _w, float _h)
			: Point(_x, _y)
			, Size(_w, _h)
		{
		}

		bool Contains(Point p)
		{
			float dx = x - p.x;
			float dy = y - p.y;
			return dx >= 0 && dy >= 0 && dx < w && dy < h;
		}
	};

	//////////////////////////////////////////////////////////////////////

	struct Matrix
	{
		//////////////////////////////////////////////////////////////////////

		float a, b, c, d, e, f;

		//////////////////////////////////////////////////////////////////////

		Matrix()
		{
		}

		//////////////////////////////////////////////////////////////////////

		Matrix(float a, float b, float c, float d, float e, float f)
			: a(a) , b(b) , c(c) , d(d) , e(e) , f(f)
		{
		}

		//////////////////////////////////////////////////////////////////////

		Matrix(Matrix const &m)
			: a(m.a) , b(m.b) , c(m.c) , d(m.d) , e(m.e) , f(m.f)
		{
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Translate(Point p) const
		{
			return Matrix(a, b, c, d, e + p.x, f + p.y);
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Rotate(float radians) const
		{
			float C = cosf(radians);
			float S = sinf(radians);
			return Matrix(a * C + c * -S, b * C + d * -S, a * S + c * C, b * S + d * C, e, f);
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Scale(Point s) const
		{
			return Matrix(a * s.x, b * s.x, c * s.y, d * s.y, e, f);
		}

		//////////////////////////////////////////////////////////////////////

		Matrix Invert() const
		{
			float x = a * d - b * c;
			if (x < 1.0e-6 && x > -1.0e-6)
			{
				return Matrix(*this);
			}
			x = 1.0f / x;
			return Matrix(d * x, b * -x, c * -x, a * x, (c * f - d * e) * x, (a * f - b * e) * -x);
		}

		//////////////////////////////////////////////////////////////////////

		Point Apply(Point const &p) const
		{
			return Point(p.x * a + p.y * c + e, p.x * b + p.y * d + f);
		}

		//////////////////////////////////////////////////////////////////////

		void Transform(Point p[], uint len) const
		{
			for(uint i = 0; i < len; ++i)
			{
				float x = p[i].x;
				float y = p[i].y;
				p[i].x = x * a + y * c + e;
				p[i].y = x * b + y * d + f;
			}
		}

		//////////////////////////////////////////////////////////////////////

		static Matrix Identity()
		{
			return Matrix(1, 0, 0, 1, 0 ,0);
		}

		//////////////////////////////////////////////////////////////////////

		static Matrix RotTransScale(float radians, Point t, Point s)
		{
			float cs = cosf(radians);
			float sn = sinf(radians);
			return Matrix(cs * s.x, -sn * s.x, sn * s.y, cs * s.y, t.x, t.y);
		}
	};

	//////////////////////////////////////////////////////////////////////

	inline Matrix operator * (Matrix const &a, Matrix const &b)
	{
		return Matrix(
			a.a * b.a + a.c * b.b,
			a.b * b.a + a.d * b.b,
			a.a * b.c + a.c * b.d,
			a.b * b.c + a.d * b.d,
			a.a * b.e + a.c * b.f + a.e,
			a.b * b.e + a.d * b.f + a.f);
	}

	//////////////////////////////////////////////////////////////////////

	inline Matrix operator * (Matrix const &a, Point const &s)
	{
		return a.Scale(s);
	}

	//////////////////////////////////////////////////////////////////////

	struct Entity
	{
		//////////////////////////////////////////////////////////////////////

		Entity *										parent;				// can be null
		chs::list_node<Entity>							list_node;			// list_node for children list
		chs::linked_list<Entity, &Entity::list_node>	children;			// list of children
		Matrix											transform;			// calculated during Draw()
		Matrix											inverse_transform;	// used for Pick()
		bool											closed;				// close requested

		Point											position;
		Size											size;
		Point											pivot;
		Point											scale;
		float											rotation;
		int												z_index;

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

		virtual void OnDraw(SpriteList &spriteList)
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

		void GetCorners(Point p[])
		{
			p[0] = position;
			p[1] = position + Size(size.w, 0);
			p[2] = position + size;
			p[3] = position + Size(0, size.h);
			transform.Transform(p, 4);	// the 4 corners transformed to screen coordinates
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

		void Draw(Matrix &m, SpriteList &spriteList)
		{
			static Point p[4];
			chs::linked_list<Entity> closed;
			Point p(-pivot.x * size.w, -pivot.y * size.h);
			transform = Matrix::Identity().Translate(position).Rotate(rotation).Scale(scale).Translate(p) * m;
			inverse_transform = transform.Invert();
			// add matrix to render queue
			OnDraw(spriteList);
			for(auto i = children.begin(), _end = children.end(), n = i; ++n, i != _end; i = n)
			{
				if(i->closed)
				{
					i->OnClosing();
					children.remove(i);
					i->parent = null;
					closed.push_back(i);
				}
			}
			children.sort();
			for(auto &e : children)
			{
				e.Draw(transform, spriteList);
			}
			for(auto &c : closed)
			{
				c.OnClosed();
			}
		}
	};

	//////////////////////////////////////////////////////////////////////

	struct Image : RefCount
	{
		Image()
			: RefCount()
		{
		}

		virtual ~Image()
		{
		}

		int width;
		int height;
	};

	//////////////////////////////////////////////////////////////////////

	struct Sprite : Entity
	{
		Sprite(Texture *texture)
			: Entity()
			, texture(texture)
		{
			texture->AddRef();
			size.w = (float)texture->Width();
			size.h = (float)texture->Height();
		}

		~Sprite()
		{
			Release(texture);
		}

		Texture *texture;

		void OnDraw(SpriteList &spriteList) override
		{
			static Point p[4];
			GetCorners(p);
			spriteList.SetTexture(texture);
			spriteList.BeginStrip();
			spriteList.SetOrigin

		}
	};

	//////////////////////////////////////////////////////////////////////

}; // namespace glib

void Bob()
{
	glib::Entity p;
}
