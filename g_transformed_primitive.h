#pragma once
#include "g_primitive.h"
#include "g_transformation.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT TransformedPrimitive : public PrimitiveBase, public ITransformable {
	protected:
		Scene& _reference_scene;
		mutable GHANDLE _reference;
		TransformPtr _transform;
	public:
		PrimitiveBase* reference() const {
			try {
				auto r = dynamic_cast<PrimitiveBase*>(_reference_scene.get_ptr(_reference));
				if (r) return r;
				else {
					auto p = &_reference_scene.find_first<PrimitiveBase>();
					_reference = p->getHandle();
					return p;
				}
			}
			catch (const FGUndefinedObject& e) {
				auto p = &_reference_scene.find_first<PrimitiveBase>();
				_reference = p->getHandle();
				return p;
			}
		}

		virtual TransformPtr Transformation() {
			return _transform;
		}
		virtual void applyTransform(const matrix4x4& m) {
			throw FGException("Unable to apply transform to dynamically transformable object");
		}
		virtual matrix4x4 get() const {
			return _transform.GetTransform();
		}
		virtual matrix4x4 getInversed() const {
			return _transform.GetInversedTransform();
		}
		TransformedPrimitive(Scene& s, Scene& primitive, TransformPtr parent, const transform& t = transform())
			: PrimitiveBase(s, s.defaultCSG),
			_transform(s.getRootTransform(), t), _reference_scene(primitive), _reference(NoGeometry)
		{
			if (primitive.isPrimitive == false) throw FGException("Unable to create transformed primitive of non primitive scene");
			_transform.SetParent(parent);
			try {
				reference();
				addSelfToContext();
			}
			catch (...) { throw; }
		}
		TransformedPrimitive(Scene& s, Scene& primitive, TransformationBase& parent, const transform& t = transform())
			: PrimitiveBase(s, s.defaultCSG),
			_transform(TransformPtr::Create(parent, t)), _reference_scene(primitive), _reference(NoGeometry) {
			if (primitive.isPrimitive == false) throw FGException("Unable to create transformed primitive of non primitive scene");
			try {
				reference();
				addSelfToContext();
			}
			catch (...) { throw; }
		}
	public:
		virtual int classify(const vector3 &pp) const {
			vector3 p = getInversed()*pp;
			return reference()->classify(p);
		}
		virtual vector3 middle() const {
			return get() * reference()->middle();
		}
		virtual inline rect getBoundingRect() const {
			auto p = reference()->getBoundingRect();
			return rect{ get() * p.Min(), get() * p.Max() };
		}

	public:
		virtual GHANDLE init() {
			return _handle;
		}

		virtual void replace(GHANDLE o, GHANDLE n) {
			if (_handle == o) _handle = n;
		}

		virtual std::vector<GHANDLE> getChildren() const {
			return{};
		}
		virtual std::vector<GHANDLE> getBoundary() const {
			return getGeometry();
		}
		virtual GHANDLE copy() const {
			return TransformedPrimitive(getConstContext(), getConstContext(), _transform.GetParent(), _transform.GetLocalTransformation()).getHandle();
			//return copy(getConstContext());
		}
		virtual GHANDLE copy(Scene &context) const {
			TransformPtr t = (context == getConstContext()) ? _transform.GetParent() : context.getRootTransform();
			return TransformedPrimitive(context, context, t).getHandle();
		}
		virtual double getDistance(const ray &v) const {
			auto i = ray(getInversed() * v.p0, getInversed()&(v.l));

			return reference()->getDistance(i);
		}
		virtual bool isInPlane(const plane &p) const {
			auto i = plane(getInversed() * (p.p0), getInversed() & (p.n));

			return reference()->isInPlane(i);
		}
		virtual bool Equals(const IGeometry &geom) const {
			auto p = dynamic_cast<const TransformedPrimitive*>(&geom);
			return p != nullptr && getConstContext() == p->getConstContext() && reference() == p->reference() && get() == p->get();
		}
	protected:
		virtual void addSelfToContext() {
			_handle = getContext().add(std::move(*this));
		}

		// IPrimitive interface
	public:
		virtual GHANDLE instantiate(Scene& s) const {
			GHANDLE cp = reference()->copy(getConstContext());
			auto& p = getConstContext().get<PrimitiveBase>(cp);
			//auto m = get();
			p.applyTransform(get());
			return cp;
		}
		virtual SETTINGHANDLE inside(const vector3 point)const {
			vector3 p = getInversed()*point;

			return reference()->inside(p);
		}
		virtual std::vector<GHANDLE> getGeometry() const {
			GHANDLE cp = reference()->copy(getConstContext());
			auto& p = getConstContext().get<PrimitiveBase>(cp);
			//auto m = get();
			p.applyTransform(get());

			return p.getGeometry();
		}
		virtual bool isConvex() const {
			return reference()->isConvex();
		}
	};
}