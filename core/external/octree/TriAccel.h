#pragma once

/**
* \brief Pre-computed triangle representation based on Ingo Wald's TriAccel layout.
*
* Fits into three 16-byte cache lines if single precision floats are used.
* The k parameter is also used for classification during kd-tree construction.
* \ingroup librender
*/
template<typename Real, typename Vector3>
struct TriAccel {
	unsigned int k;
	Real n_u;
	Real n_v;
	Real n_d;

	Real a_u;
	Real a_v;
	Real b_nu;
	Real b_nv;

	Real c_nu;
	Real c_nv;

	/// Construct from vertex data. Returns '1' if there was a failure
	inline int load(const Vector3 &A, const Vector3 &B, const Vector3 &C) 
	{
		static const int waldModulo[4] = { 1, 2, 0, 1 };

		Vector3 b = C-A, c = B-A, N = cross(c, b);

		k = 0;
		/* Determine the largest projection axis */
		for (int j=0; j<3; j++) {
			if (fabs(N[j]) > fabs(N[k]))
				k = j;
		}

		unsigned int u = waldModulo[k],	v = waldModulo[k+1];
		const Real n_k = N[k], denom = b[u]*c[v] - b[v]*c[u];

		if (denom == 0) {
			k = 3;
			qDebug() << "warning: load failed";
			return 1;
		}

		/* Pre-compute intersection calculation constants */
		n_u   =  N[u] / n_k;
		n_v   =  N[v] / n_k;
		n_d   =  dot(A, N) / n_k;
		b_nu  =  b[u] / denom;
		b_nv  = -b[v] / denom;
		a_u   =  A[u];
		a_v   =  A[v];
		c_nu  =  c[v] / denom;
		c_nv  = -c[u] / denom;
		return 0;
	}

	struct TriAccelRay{ Vector3 o,d; TriAccelRay(const Vector3& origin, const Vector3& direction) : o(origin), d(direction){} };

	/// Fast ray-triangle intersection test
	inline bool rayIntersect(const TriAccelRay &ray, Real mint, Real maxt, Real &u, Real &v, Real &t) const {
		Real o_u, o_v, o_k, d_u, d_v, d_k;
		switch (k) {
		case 0:
			o_u = ray.o[1];
			o_v = ray.o[2];
			o_k = ray.o[0];
			d_u = ray.d[1];
			d_v = ray.d[2];
			d_k = ray.d[0];
			break;
		case 1:
			o_u = ray.o[2];
			o_v = ray.o[0];
			o_k = ray.o[1];
			d_u = ray.d[2];
			d_v = ray.d[0];
			d_k = ray.d[1];
			break;
		case 2:
			o_u = ray.o[0];
			o_v = ray.o[1];
			o_k = ray.o[2];
			d_u = ray.d[0];
			d_v = ray.d[1];
			d_k = ray.d[2];
			break;
		default:
			return false;
		}

		/* Calculate the plane intersection (Typo in the thesis?) */
		t = (n_d - o_u*n_u - o_v*n_v - o_k) /
			(d_u * n_u + d_v * n_v + d_k);

		if (t < mint || t > maxt)
			return false;

		/* Calculate the projected plane intersection point */
		const Real hu = o_u + t * d_u - a_u;
		const Real hv = o_v + t * d_v - a_v;

		/* In barycentric coordinates */
		u = hv * b_nu + hu * b_nv;
		v = hu * c_nu + hv * c_nv;
		return u >= 0 && v >= 0 && u+v <= 1;
	}
};

typedef TriAccel<double, Eigen::Vector3d> TriAcceld;
