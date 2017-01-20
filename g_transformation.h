#pragma once
#include "stdafx.h"
#include "fg_math.h"

namespace fg {

	class FEMCADGEOMSHARED_EXPORT Transformation {
	private:
		Transformation(const Transformation& trans) : t(trans.t), parent(trans.parent), depth(trans.depth) {
			if (parent != nullptr) {
				//parent->children.emplace_back(this);
				//std::copy(trans.children.begin(), trans.children.end(), children.begin());
			}
			else {
				throw std::logic_error("Cant clone root");
			}

		}
		Transformation& operator=(const Transformation& t) = default;
	public:
		using pointer = std::unique_ptr<Transformation>;
	protected:
		transform t;
		Transformation* parent;
		std::vector<pointer> children;
		int depth;

		Transformation() : t(), parent(nullptr), depth(0) {}
	public:
		Transformation* GetParent() const { return parent; }
		Transformation(const transform& t) : t(t), parent(nullptr), depth(0) {}
	public:
		~Transformation() {
			parent = nullptr;
			children.clear();
		}
		transform GetLocalTransformation() const { return t; }

		Transformation(Transformation&& trans) {
			t = trans.t;
			children = std::move(trans.children);
			//trans.children.clear();
			depth = trans.depth;
			parent = trans.parent;
		}

		bool IsParent(Transformation* t) const {
			return (t->depth < depth) && (parent == t || (parent != nullptr && parent->IsParent(t)));
		}

		void UpdateDepth() {
			if (parent) depth = parent->depth + 1;
			else depth = 0;
			for (size_t i = 0U; i < children.size(); i++) {
				children[i]->UpdateDepth();
			}
		}

		Transformation* GetChild(int index) {
			return children[index].get();
		}

		Transformation* SetParent(Transformation* t) {
			if (t->IsParent(this)) throw std::logic_error("Can't become self child");
			int index = -1;
			depth = t->depth + 1;
			if (parent != nullptr) {
				for (size_t i = 0U; i < parent->children.size(); i++) {
					if (parent->children[i].get() == this) {
						index = i; break;
					}
				}
				if (index == -1) throw std::logic_error("Already not a child");

				std::swap(parent->children[index], parent->children.back());
				Transformation* old_parent = parent;
				parent = t;
				UpdateDepth();
				//t->children.push_back(Transformation::pointer());
				t->children.push_back(std::move(old_parent->children.back()));
				//std::swap(t->children.back(), old_parent->children.back());

				old_parent->children.pop_back();
			}
			else {
				parent = t;
				UpdateDepth();
				pointer ptr(std::make_unique<Transformation>(std::move(*this)));
				t->children.push_back(std::move(ptr));
			}
			return t->children.back().get();
			//parent->children.push_back();
		}

		void Set(transform& tr) {
			t = tr;
		}

		matrix4x4 GetTransform() const {
			matrix4x4 current = t.get();
			if (parent != nullptr)
				current = current * parent->GetTransform();
			return current;
		}
		matrix4x4 GetInversedTransform() const {
			matrix4x4 current = t.get_inversed();
			if (parent != nullptr)
				current = current * parent->GetInversedTransform();
			return current;
		}

		Transformation* AddChild(Transformation&& tr) {
			children.emplace_back(std::make_unique<Transformation>(std::move(tr)));
			children.back()->parent = this;
			children.back()->UpdateDepth();

			return children.back().get();
		}
		void SetPosition(const vector3& position) {
			t.position = position;
		}
		void AddPosition(const vector3& position) {
			t.position = t.position + position;
		}
		void SetRotation(const quaternion& rotation) {
			t.rotation = rotation;
		}
		void AddRotation(const quaternion& rotation) {
			t.rotation = t.rotation * rotation;
		}
		void SetScale(const vector3& scale) {
			t.scale = scale;
		}
		void AddScale(const vector3& scale) {
			t.scale = t.scale + scale; // [TODO] Multiply by magnitude
		}
		//[TODO] void SnapTransform()

	protected:

	};

	class FEMCADGEOMSHARED_EXPORT TransformPtr;

	class FEMCADGEOMSHARED_EXPORT TransformationBase {
	public:
		virtual Transformation* getRoot() const = 0;
		virtual Transformation* get() const = 0;
		virtual matrix4x4 GetTransform() const = 0;
		virtual matrix4x4 GetInversedTransform() const = 0;
	};

	class FEMCADGEOMSHARED_EXPORT TransformRoot : public TransformationBase {
		using pointer = std::unique_ptr<Transformation>;
		pointer root;
	protected:
		TransformRoot(const TransformRoot& r) {}
		TransformRoot& operator=(const TransformRoot& r) { return *this; }
	public:
		TransformRoot() {
			root = std::make_unique<Transformation>(Transformation(transform()));
		}
		Transformation* getRoot() const { return root.get(); }
		Transformation* get() const {
			return root.get();
		}
		matrix4x4 GetTransform()const {
			return root->GetTransform();
		}
		matrix4x4 GetInversedTransform() const {
			return root->GetInversedTransform();
		}
	};

	class FEMCADGEOMSHARED_EXPORT TransformPtr : public TransformationBase {
	protected:
		Transformation *root;
		Transformation *pointer;
		TransformPtr(Transformation* root, Transformation *pointer) : root(root), pointer(pointer) {}
	public:
		static TransformPtr Create(TransformationBase& parent, const transform& t = transform()) {
			return TransformPtr(parent.getRoot(), parent.get()->AddChild(Transformation(t)));
		}

		TransformPtr(const TransformationBase& root, const transform& t = transform()) {
			pointer = root.get()->AddChild(Transformation(t));
			this->root = root.get();
		}
		TransformPtr(const TransformPtr& ptr) {
			pointer = ptr.get();
			root = ptr.root;
		}
		TransformPtr(TransformPtr&& ptr) {
			pointer = ptr.get();
			root = ptr.root;

			ptr.pointer = nullptr;
			ptr.root = nullptr;
		}
		TransformPtr& operator =(const TransformPtr& ptr) {
			pointer = ptr.get();
			root = ptr.root;
			return *this;
		}
		TransformPtr(decltype(nullptr) a) {
			root = nullptr;
			pointer = nullptr;
		}
		transform GetLocalTransformation() const { return pointer->GetLocalTransformation(); }

		void SetPosition(const vector3& position) {
			pointer->SetPosition(position);
		}
		void AddPosition(const vector3& position) {
			pointer->AddPosition(position);
		}
		void SetRotation(const quaternion& rotation) {
			pointer->SetRotation(rotation);
		}
		void AddRotation(const quaternion& rotation) {
			pointer->AddRotation(rotation);
		}
		void SetScale(const vector3& scale) {
			pointer->SetScale(scale);
		}
		void AddScale(const vector3& scale) {
			pointer->AddScale(scale);
		}


		void SetParent(TransformPtr parent) {
			if (root == nullptr) throw std::logic_error("Unable to define parent of empty transform");
			pointer = pointer->SetParent(parent.pointer);
			root = parent.root;
		}
		TransformPtr GetParent() const {
			return TransformPtr(root, pointer->GetParent());
			//return TransformPtr(root->GetParent(), root);
		}

		Transformation* getRoot() const {
			if (root == nullptr) throw std::logic_error("Unable to get empty transform");
			return root;
		}
		Transformation* get() const {
			if (root == nullptr) throw std::logic_error("Unable to get empty transform");
			return pointer;
		}
		matrix4x4 GetTransform() const {
			if (root == nullptr) throw std::logic_error("Unable to get empty transform");
			return pointer->GetTransform();
		}
		matrix4x4 GetInversedTransform() const {
			if (root == nullptr) throw std::logic_error("Unable to get empty transform");
			return pointer->GetInversedTransform();
		}
		bool IsEmpty() {
			return root == nullptr;
		}
	};

	class FEMCADGEOMSHARED_EXPORT ITransformable {
	public:
		virtual TransformPtr Transformation() = 0;
		virtual matrix4x4 get() const = 0;
		virtual matrix4x4 getInversed() const = 0;
	};
}