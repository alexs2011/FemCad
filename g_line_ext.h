#pragma once
#include "fg_math.h"
#include "g_line.h"
#include "g_shape_2d.h"
#include "g_meshing.h"

namespace fg {
	class MeshedLine {
		double q;
		size_t N;
	public:
		const ILine& line;
		std::vector<double> points;
		inline MeshedLine(const ILine& ref_line) : line{ ref_line } {
			q = ref_line.getSetting()->getParameter<DoubleParameter>("q");
			N = (size_t)ref_line.getSetting()->getParameter<DoubleParameter>("N");
		}
		inline MeshedLine(const MeshedLine&) = default;
		inline MeshedLine(MeshedLine&&) = default;
		MeshedLine& operator =(const MeshedLine&) = default;
		MeshedLine& operator =(MeshedLine&&) = default;
		void build() {
			points.resize(N - 1);
			//points.front() = 0.0;
			if (std::fabs(q - 1.0) < 1e-12) {
				double b = 1.0 / N;
				for (size_t i = 1U; i < N; ++i)
					points[i - 1] = i*b;
			}
			else {
				double b = (q - 1) / (std::pow(q, (double)N) - 1);
				points[0] = b;
				for (size_t i = 2U; i < N; ++i)
					points[i - 1] = b + (points[i - 2] * q);// (std::pow(q, i) - 1);
			}
			//points.back() = 1.0;
		}
		vector3 sample(double t) const { return line.sample(t); }
		vector3 p0()const { return line.P0(); }
		vector3 p1()const { return line.P1(); }
	};
	class LineView {
		vector3 p0;
		vector3 tangent;
		mutable matrix4x4 cached_matrix;
	public:
		std::vector<MeshedLine> lines;
		LineView(const std::vector<const ILine*>& ref_lines) {
			p0 = ref_lines.front()->P0();
			tangent = ref_lines.back()->P1() - p0;
			lines.reserve(ref_lines.size());
			for (size_t i = 0U; i < ref_lines.size(); ++i) {
				lines.emplace_back(*ref_lines[i]);
				lines.back().build();
			}
			cached_matrix = matrix4x4::diag(1.0, 1.0, 1.0, 1.0);
		}
		std::vector<double> mesh() const {
			std::vector<double> result;
			auto s = 1.0 / lines.size();
			for (size_t i{}; i < lines.size(); i++) {
				if (i > 0) result.push_back(i * s);
				for (size_t j{}; j < lines[i].points.size(); j++)
					result.push_back((i + lines[i].points[j]) * s);
			}
			result.shrink_to_fit();
			return result;
		}
		void cache_identity() const {
			cached_matrix = matrix4x4::diag(1.0, 1.0, 1.0, 1.0);
		}
		void cache_transform(const vector3& q0, const vector3& q1) const {
			auto qn = (q1 - q0);
			double tx = q0.x - p0.x;
			double ty = q0.y - p0.y;
			auto qnorm = 1.0 / qn.length();
			auto tnorm = 1.0 / tangent.length();
			qn = qn * qnorm;
			vector3 tn = tangent * tnorm;
			double s = tnorm / qnorm;
			double sn = s * (qn ^ tn).z;
			double cs = s * (tn & qn);
			cached_matrix = matrix4x4();
			cached_matrix.m00 = cs;
			cached_matrix.m10 = -sn;
			cached_matrix.m01 = sn;
			cached_matrix.m11 = cs;
			cached_matrix.m03 = tx;
			cached_matrix.m13 = ty;
			cached_matrix.m22 = 1.0;
			cached_matrix.m33 = 1.0;
		}
		vector3 sample(double t) const {
			assert(t >= 0.0);
			t *= lines.size();
			int line = (int)t;
			if (line == lines.size()) line--;
			t -= line;
			auto p = lines[line].sample(t);
			return cached_matrix * (p - p0)+p0;
		}
	};
	class RectView {
		const primitive::Shape& shape;
		const GHANDLE p00;
		const GHANDLE p01;
		const GHANDLE p10;
		const GHANDLE p11;
	public:
		std::vector<LineView> lines;
		std::vector<double> params[4];
		RectView() = delete;
		inline RectView(const primitive::Shape& shape, const GHANDLE p00, const GHANDLE p01, const GHANDLE p10, const GHANDLE p11) :
			shape{ shape }, p00{ p00 }, p01{ p01 }, p10{ p10 }, p11{ p11 } {
			std::vector<const ILine*> side[4];
			size_t side_points[4];
			const auto& bnd = shape.getConstBoundary();
			size_t i{};
			for (; i < bnd.size(); i++) {
				const ILine& p = shape.getConstContext().get<const ILine>(bnd[i]);
				if (p.p0Handle() == p00) {
					break;
				}
			}
			size_t current{};
			for (size_t j{}; j < bnd.size(); j++) {
				const ILine* p = static_cast<const ILine*>(shape.getConstContext().get_ptr(bnd[(i + j) % bnd.size()]));
				if (p->p0Handle() == p01) current = 1;
				if (p->p0Handle() == p11) current = 2;
				if (p->p0Handle() == p10) current = 3;
				side[current].emplace_back(p);
			}
			for (size_t j{}; j < 4; j++) {
				lines.emplace_back(LineView{ side[j] });
				params[j] = lines[j].mesh();
				params[j].insert(params[j].begin(), 0.0);
				params[j].push_back(1.0);
			}
			if (params[0].size() != params[2].size() || params[1].size() != params[3].size()) {
				throw FGException("Bad initial meshing data");
			}
		}
		RectView(const RectView&) = default;

		RawMesh2 mesh() const {
			lines[0].cache_identity();
			lines[2].cache_identity();
			auto N = params[0].size();
			auto M = params[1].size();
			std::vector<vector3> points(N * M);
			for (size_t i{}; i < N; i++) {
				double xi0 = params[0][i];
				double xi1 = params[2][-i + N - 1];
				double w = (xi0 + 1.0 - xi1) * 0.5;
				vector3 beg = lines[0].sample(xi0);
				vector3 end = lines[2].sample(xi1);
				lines[1].cache_transform(beg, end);
				lines[3].cache_transform(end, beg);
				for (size_t j{}; j < M; j++) {
					double eta0 = params[1][j];
					double eta1 = params[3][M - j - 1];
					auto pp0 = lines[1].sample(eta0);
					auto pp1 = lines[3].sample(eta1);
					points[M * i + j] = (w) * pp0 + (1.0 - w) * pp1;
				}
			}
			std::vector<std::tuple<size_t, size_t, size_t>> tris;
			for (size_t i{}; i < N - 1; i++) {
				for (size_t j{}; j < M - 1; j++) {
					size_t pt[4] = { M * i + j, 0,0,0 };
					pt[1] = pt[0] + 1;
					pt[2] = pt[0] + M + 1;
					pt[3] = pt[0] + M;
					//std::pair<size_t, size_t> diag(0xFFFFFFFF, 0xFFFFFFFF);
					int p = 0;
					double minmidl = 1e300;
					for (size_t k = 0; k < 4; k++) {
						auto next = points[pt[(k + 1) % 4]] - points[pt[k]];
						auto prev = points[pt[(k + 3) % 4]] - points[pt[k]];
						auto midl = points[pt[(k + 2) % 4]] - points[pt[k]];
						auto nn = next ^ vector3::Z();
						auto np = prev ^ vector3::Z();

						if ((nn & prev) * (nn & midl) < 0 && (np & next) * (np & midl) < 0) {
							p = k;
							//diag = std::make_pair(pt[k], pt[(k + 2) % 4]);
							break;
						}
						if (midl.lengthSq() < minmidl) {
							p = k;
							//diag = std::make_pair(pt[k], pt[(k + 2) % 4]);
							minmidl = midl.lengthSq();
						}
					}
					tris.push_back(std::make_tuple(pt[p], pt[(p + 1) % 4], pt[(p + 2) % 4]));
					tris.push_back(std::make_tuple(pt[p], pt[(p + 3) % 4], pt[(p + 2) % 4]));
				}
			}
			return RawMesh2(points, tris);
		}
	};
}