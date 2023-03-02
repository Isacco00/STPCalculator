#include "CommonImport.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"


IShape::IShape(const TopoDS_Shape& shape)
	: m_shape(shape),
	m_isTessellated(false),
	m_isFaceSet(false),
	m_component(nullptr),
	m_globalIndex(0),
	m_stepID(-1) {
	// Check if the shape is a face set
	if (OCCUtil::HasFace(m_shape)) {
		m_isFaceSet = true;
	}
}

IShape::~IShape(void) {
	Clear();
}

const Quantity_ColorRGBA& IShape::GetColor(const TopoDS_Shape& shape) const {
	int shapeID = OCCUtil::GetID(shape);
	const Quantity_ColorRGBA& color = m_shapeIDcolorMap.find(shapeID)->second;

	return color;
}

bool IShape::IsEmpty(void) const {
	if (GetMeshSize() == 0)
		return true;

	return false;
}

wstring IShape::GetUniqueName(void) const {
	// Get the closest component with a unique name
	Component* comp = GetComponent();
	wstring uniqueName = comp->GetUniqueName();

	while (!comp->IsRoot()
		   && !comp->HasUniqueName()) {
		comp = comp->GetParentComponent();
		uniqueName = comp->GetUniqueName();
	}

	return uniqueName;
}

void IShape::Clear(void) {
	for (auto mesh : m_meshList)
		delete mesh;

	m_meshList.clear();
	m_colorList.clear();
	m_shapeIDcolorMap.clear();
}