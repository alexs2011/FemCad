#pragma once
#include "fg_math.h"
#include "g_line.h"
#include "g_shape_2d.h"
#include "g_meshing.h"

namespace fg {
	// линия сетки с разрядкой
	class MeshedLine {
		// на сколько следующий отрезок сетки будет больше предыдущего
		double q;
		// сколько всего будет отрезков
		size_t N;
	public:
		const ILine& line;
		// отступы от начала линии, на которых будут располагаться вершины сетки
		std::vector<double> points;
	public:
		// конструктор линии сетки с разрядкой
		inline MeshedLine(const ILine& ref_line) : line{ ref_line } {
			q = ref_line.getSetting()->getParameter<DoubleParameter>("q");
			N = (size_t)ref_line.getSetting()->getParameter<DoubleParameter>("N");
		}
		inline MeshedLine(const MeshedLine&) = default;
		inline MeshedLine(MeshedLine&&) = default;
		MeshedLine& operator =(const MeshedLine&) = default;
		MeshedLine& operator =(MeshedLine&&) = default;

		// добавляется разрядка (заполняется массив points)
		void build() {
			// кол-во точек = кол-во разбиений линии
			points.resize(N - 1);
			// равномерное разбиение
			if (std::fabs(q - 1.0) < 1e-12 || N == 1) {
				double b = 1.0 / N;
				for (size_t i = 1U; i < N; ++i)
					points[i - 1] = i*b;
			} // разбиение с геометрической прогрессией
			else {
				double b = (q - 1) / (std::pow(q, (double)N) - 1);
				points[0] = b;
				for (size_t i = 2U; i < N; ++i)
					points[i - 1] = b + (points[i - 2] * q);
			}
		}
		vector3 sample(double t) const { return line.sample(t); }
		vector3 p0()const { return line.P0(); }
		vector3 p1()const { return line.P1(); }
	};

	// представление одной границы геометрии (хранит линии сетки с разрядкой, принадлежащие этой границе)
	class LineView {
		// первая вершина границы
		vector3 p0;
		// вектор касательной, т.е. вектор, который направлен от первой точки границы (p0) к последней
		vector3 tangent;
		mutable matrix4x4 cached_matrix;
	public:
		//линии сетки с разрядкой, принадлежащие этой границе
		std::vector<MeshedLine> lines;
	public:
		// линии границы превращаем в линии сетки с разрядкой
		LineView(const std::vector<const ILine*>& ref_lines) {
			p0 = ref_lines.front()->P0();
			tangent = ref_lines.back()->P1() - p0;
			lines.reserve(ref_lines.size());
			for (size_t i = 0U; i < ref_lines.size(); ++i) {
				lines.emplace_back(*ref_lines[i]);
				// в линию добавляется разрядка, т.е. у каждой линии заполняется поле points, т.е. на нее добаляются новые точки (вершины сетки)
				lines.back().build();
			}
			cached_matrix = matrix4x4::diag(1.0, 1.0, 1.0, 1.0);
		}

		// возвращает отступы от начала ГРАНИЦЫ, где будут располагаться вершины сетки
		std::vector<double> _mesh() const {
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

		// вовращает единичную трансформационную матрицу
		void cache_identity() const {
			cached_matrix = matrix4x4::diag(1.0, 1.0, 1.0, 1.0);
		}
		// q0 и q1  - точки на противоположных границах
		void cache_transform(const vector3& q0, const vector3& q1) const {
			// tangent - это вектор касательной к ещё одной границе
			// вектор от q0 к q1
			auto qn = (q1 - q0);
			// вектор от начала границы до точки q0
			double tx = q0.x - p0.x;
			double ty = q0.y - p0.y;
			// считаем длину
			auto qnorm = 1.0 / qn.length();
			auto tnorm = 1.0 / tangent.length();
			// нормируем
			qn = qn * qnorm;
			vector3 tn = tangent * tnorm;
			// отношение длины вектора касательной к другой границе к расстоянию между p0 и q0
			double s = tnorm / qnorm;
			// синус и косинус
			double sn = s * (qn ^ tn).z;
			double cs = s * (tn & qn);
			// матрица трансформации, переводящие точки границы линии в точки q0 и q1
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
		// возвращает точку, которая лежит на границе с отступом t
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
	// всё то делает rect - это задаёт противоположные границы геометрии, что нужно для построения сетки
	class RectView {
		const primitive::Shape& shape;
		const GHANDLE p00;
		const GHANDLE p01;
		const GHANDLE p10;
		const GHANDLE p11;
		//mutable std::unique_ptr<Mesh2> _mesh;
		//mutable std::vector<MeshedLine> _boundary;
	public:
		//virtual const std::vector<MeshedLine>& boundary() const {
		//	return _boundary;
		//}
		// вектор границ, т.е. наборов линий с разрядкой и преобразованиями
		std::vector<LineView> lines;
		// для каждой из 4-х границ здесь хранятся отступы от начала ГРАНИЦЫ, где будут располагаться вершины сетки
		std::vector<double> params[4];
		RectView() = delete;
		inline RectView(const primitive::Shape& shape, const GHANDLE p00, const GHANDLE p01, const GHANDLE p10, const GHANDLE p11) :
			shape{ shape }, p00{ p00 }, p01{ p01 }, p10{ p10 }, p11{ p11 }
		{
			// линии из которых состоит каждая из границ
			std::vector<const ILine*> side[4];
			// size_t side_points[4];
			// getConstBoundary() возвращает геометрию
			const auto& bnd = shape.getConstBoundary();
			size_t i{};
			// ищем в геометрии вершину, которая соотвествует p00
			for (; i < bnd.size(); i++) {
				const ILine& p = shape.getConstContext().get<const ILine>(bnd[i]);
				//_boundary.emplace_back(MeshedLine{ p });
				if (p.p0Handle() == p00) {
					break;
				}
			}
			/*for (size_t j{i+1}; j < bnd.size(); j++) {
				const ILine& p = shape.getConstContext().get<const ILine>(bnd[j]);
				_boundary.emplace_back(MeshedLine{ p });
			}*/
			// идем по границе и начинаем заполнять противоположные границы геометрии (side) элементами геометрии, составляющими эти границы
			size_t current{};
			for (size_t j{}; j < bnd.size(); j++) {
				const ILine* p = static_cast<const ILine*>(shape.getConstContext().get_ptr(bnd[(i + j) % bnd.size()]));
				if (p->p0Handle() == p01) current = 1;
				if (p->p0Handle() == p11) current = 2;
				if (p->p0Handle() == p10) current = 3;
				side[current].emplace_back(p);
			}
			// перебираем все 4 границы и заполняем вектора параметров, т.е. разрядок для каждой границы
			for (size_t j{}; j < 4; j++) {
				lines.emplace_back(LineView{ side[j] });
				// заполняем отступы от начала ГРАНИЦЫ, где будут располагаться вершины сетки
				params[j] = lines[j]._mesh();
				// в начало и конец отступов добавим 0 и 1
				params[j].insert(params[j].begin(), 0.0);
				params[j].push_back(1.0);
			}
			if (params[0].size() != params[2].size() || params[1].size() != params[3].size()) {
				throw FGException("Bad initial meshing data");
			}
		}

		RectView(const RectView&) = default;


		// строит сетку внутри объекта
		RawMesh2 mesh() const {
			// устанавливает единичную трансформационную матрицу
			lines[0].cache_identity();
			lines[2].cache_identity();
			// кол-во точек на основных границах (между которыми в первую очередь будут построены ребра сетки)
			auto N = params[0].size();
			// кол-во точек на боковых границах
			auto M = params[1].size();
			// все вершины сетки, которая будет построена
			std::vector<vector3> points(N * M);
			for (size_t i{}; i < N; i++) {
				// проходим отступы первой границы с начала
				double xi0 = params[0][i];
				// проходим отступы третьей границы с конца (по сути выберутся противоположные отступы)
				double xi1 = params[2][-i + N - 1];
				// вычисление вклада границ 1 и 3 в результирующую сетку (насколько точка ближе к 1 или 3 границе)
				double w = (xi0 + 1.0 - xi1) * 0.5;
				// рассчитаем пару точек сетки, которые лежат на противоположных границах с их отступами
				vector3 beg = lines[0].sample(xi0);
				vector3 end = lines[2].sample(xi1);
				// у боковых границ изменили систему координат (повернули и перенесли)
				// кэширование трансформации для перевода концов линий 1 и 3 в точки beg и end
				lines[1].cache_transform(beg, end);
				lines[3].cache_transform(end, beg);

				for (size_t j{}; j < M; j++) {
					// идем по всем точкам сетки для границ 1 и 3
					double eta0 = params[1][j];
					double eta1 = params[3][M - j - 1];
					// вычисляем координаты точек на линиях 1 и 3 с учетом трансформации
					auto pp0 = lines[1].sample(eta0);
					auto pp1 = lines[3].sample(eta1);
						
					points[M * i + j] = (w) * pp0 + (1.0 - w) * pp1;
				}
			}
			// все треугольники  сетки, которая строится
			std::vector<std::tuple<size_t, size_t, size_t>> tris;
			for (size_t i{}; i < N - 1; i++) {
				for (size_t j{}; j < M - 1; j++) {
					size_t pt[4] = { M * i + j, 0,0,0 };
					pt[1] = pt[0] + 1;
					pt[2] = pt[0] + M + 1;
					pt[3] = pt[0] + M;
					//std::pair<size_t, size_t> diag(0xFFFFFFFF, 0xFFFFFFFF);
					int p = 0;
					// строим диагонали, пока не построи подходящую
					// длина минимальной диагонали
					double minmidl = 1e300;
					for (size_t k = 0; k < 4; k++) {
						auto next = points[pt[(k + 1) % 4]] - points[pt[k]];
						auto prev = points[pt[(k + 3) % 4]] - points[pt[k]];
						auto midl = points[pt[(k + 2) % 4]] - points[pt[k]];
						auto nn = next ^ vector3::Z();
						auto np = prev ^ vector3::Z();
						// проверяем, что данная диагональ разбивает 4-х угольник на тр-ки
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