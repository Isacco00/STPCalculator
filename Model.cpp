#include "CommonImport.h"
#include "Model.h"
#include "Component.h"
#include "IShape.h"

Model::Model(void) {}

Model::~Model(void) {
	Clear();
}

void Model::GetAllComponents(vector<Component*>& comps) const {
	for (const auto& rootComp : m_rootComponents) {
		comps.push_back(rootComp);
	}
}

const Bnd_Box Model::GetBoundingBox(bool sketch) const {
	Bnd_Box bndBox;

	for (const auto& rootComp : m_rootComponents) {
		const Bnd_Box& subBndBox = rootComp->GetBoundingBox(sketch);
		bndBox.Add(subBndBox);
	}

	return bndBox;
}

const ShapeType Model::GetShapeType(void) const {
	vector<Component*> comps;
	GetAllComponents(comps);
	int shapeCount = 0;
	int sketchCount = 0;
	ShapeType shapeType = ShapeType::Face_Geom;
	for (const auto& comp : comps) {
		for (int i = 0; i < comp->GetIShapeSize(); ++i) {
			IShape* iShape = comp->GetIShapeAt(i);
			shapeCount++;
			if (iShape->IsSketchGeometry()) {
				sketchCount++;
			}
		}
	}

	comps.clear();

	if (sketchCount > 0) {
		if (shapeCount == sketchCount) {
			shapeType = ShapeType::Sketch_Geom;
		} else {
			shapeType = ShapeType::Hybrid_Geom;
		}
	}

	return shapeType;
}

bool Model::IsEmpty(void) const {
	int size = 0;
	for (const auto& rootComp : m_rootComponents) {
		if (rootComp->IsEmpty()) {
			size++;
		}
	}
	if (size == (int)m_rootComponents.size()) {
		return true;
	}

	return false;
}

void Model::Update(void) {
	Clean();
	UpdateNames();
}

void Model::Clean(void) {
	int rootCompSize = GetComponentSize();
	for (int i = rootCompSize - 1; i >= 0; --i) {
		Component* rootComp = GetComponentAt(i);
		if (rootComp->IsEmpty()) {
			m_rootComponents.erase(m_rootComponents.begin() + i);
			delete rootComp;
		} else
			rootComp->Clean();
	}
}

void Model::UpdateNames(void) const {
	UpdateComponentNames();
	UpdateIShapeNames();
	UpdateGlobalIShapeIndex();
}


void Model::UpdateComponentNames(void) const {
	wstring connector = L"_";	// Character connecting Group name and order

	// Make all components' names unique to avoid conflict
	vector<Component*> comps;
	GetAllComponents(comps);

	map<wstring, int> tmpNameCountMap, nameCountMap;

	// Count all component names first
	for (const auto& comp : comps) {
		wstring compName = comp->GetName();

		// Skip if the name is empty
		if (compName.empty())
			continue;

		if (tmpNameCountMap.find(compName) == tmpNameCountMap.end())
			tmpNameCountMap.insert({ compName, 1 });
		else {
			int count = tmpNameCountMap[compName];
			count++;
			tmpNameCountMap[compName] = count;
		}
	}

	// Update all component names
	for (const auto& comp : comps) {
		wstring compName = comp->GetName();

		// Skip if the name is empty
		if (compName.empty())
			continue;

		// Skip if the name is one and only
		if (tmpNameCountMap.find(compName)->second == 1)
			continue;

		if (nameCountMap.find(compName) == nameCountMap.end()) {
			nameCountMap.insert({ compName, 1 });
			comp->SetName(compName + connector + to_wstring(1));
		} else {
			int count = nameCountMap[compName];
			count++;
			nameCountMap[compName] = count;
			comp->SetName(compName + connector + to_wstring(count));
		}
	}

	tmpNameCountMap.clear();
	nameCountMap.clear();

	// Update subcomponent names
	for (const auto& comp : comps) {
		if (comp->IsRoot()
			&& !comp->HasUniqueName())
			comp->SetName(L"unnamed");

		wstring compName = comp->GetName();
		int compIndex = 1;
	}

	comps.clear();
}

void Model::UpdateIShapeNames(void) const {
	wstring connector = L"_";	// Character connecting IShape name and order

	vector<Component*> comps;
	GetAllComponents(comps);

	for (const auto& comp : comps) {
		wstring compName = comp->GetName();
		int shapeIndex = 1;

		for (int i = 0; i < comp->GetIShapeSize(); ++i) {
			IShape* iShape = comp->GetIShapeAt(i);

			if (iShape->IsSketchGeometry())
				continue;

			iShape->SetName(compName + connector + to_wstring(shapeIndex));
			shapeIndex++;
		}
	}

	comps.clear();
}

void Model::UpdateGlobalIShapeIndex(void) const {
	vector<Component*> comps;
	GetAllComponents(comps);

	// Global shape index for Coordinate DEF/USE
	int globalIndex = 1;

	// Update names and indexes of iShapes at every update
	for (const auto& comp : comps) {
		for (int i = 0; i < comp->GetIShapeSize(); ++i) {
			IShape* iShape = comp->GetIShapeAt(i);

			if (iShape->IsSketchGeometry())
				continue;

			iShape->SetGlobalIndex(globalIndex);
			globalIndex++;
		}
	}

	comps.clear();
}

void Model::Clear(void) {
	for (auto rootComp : m_rootComponents)
		delete rootComp;
	m_rootComponents.clear();
}