#include "CommonImport.h"
#include "Component.h"
#include "IShape.h"


Component::Component(const TopoDS_Shape& shape)
	: m_parentComponent(nullptr),
	m_hasUniqueName(false),
	m_shape(shape),
	m_stepID(-1) {}

Component::~Component(void) {
	Clear();
}

void Component::AddIShape(IShape*& iShape) {
	m_iShapes.push_back(iShape);
	iShape->SetComponent(this);
}

void Component::Clean(void) {
	CleanEmptyIShapes();
}

void Component::CleanEmptyIShapes(void) {
	int iShapeSize = GetIShapeSize();

	for (int i = iShapeSize - 1; i >= 0; --i) {
		IShape* iShape = GetIShapeAt(i);

		// Skip if the shape is not tessellated yet
		if (!iShape->IsTessellated())
			continue;

		// Remove empty IShapes after tessellation
		if (iShape->IsEmpty()) {
			m_iShapes.erase(m_iShapes.begin() + i);
			delete iShape;
		}
	}
}

bool Component::IsRoot(void) const {
	if (!m_parentComponent)
		return true;

	return false;
}

bool Component::IsEmpty(void) const {
	if (GetIShapeSize() == 0) {
		return true;
	}

	return false;
}

const Bnd_Box Component::GetBoundingBox(bool sketch) const {
	Bnd_Box bndBox;

	// Add sub bounding boxes for iShapes
	for (const auto& iShape : m_iShapes) {
		bool isSketchGeometry = iShape->IsSketchGeometry();

		// Skip sketchGeometry when the sketch option is off
		if (!sketch
			&& isSketchGeometry)
			continue;

		const TopoDS_Shape& shape = iShape->GetShape();

		const Bnd_Box& subBndBox = OCCUtil::ComputeBoundingBox(shape);
		bndBox.Add(subBndBox);
	}

	// Get the finite bounding box (mandatory)
	bndBox = bndBox.FinitePart();

	return bndBox;
}

void Component::Clear(void) {
	for (auto iShape : m_iShapes) {
		delete iShape;
	}
	m_iShapes.clear();
}