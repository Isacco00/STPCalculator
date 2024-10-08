#include "CommonImport.h"
#include "Tessellator.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"

Tessellator::Tessellator(InputOptions* opt)
	: m_opt(opt) {
	double angDeflection_max = 0.8, angDeflection_min = 0.2, angDeflection_gap = (angDeflection_max - angDeflection_min) / 10;
	m_angDeflection = max(angDeflection_max - (m_opt->GetQuality() * angDeflection_gap), angDeflection_min);

	//m_linDeflection = 2.5 / m_opt->Quality();
	//m_angDeflection = 5.0 / m_opt->Quality();

	m_isRelative = false; // If TRUE, linear deflection is automatically set.
}

Tessellator::~Tessellator(void) {}

void Tessellator::Tessellate(Model*& model) const {
	TessellateModel(model);

	model->Update();
}

void Tessellator::TessellateModel(Model*& model) const {
	for (int i = 0; i < model->GetComponentSize(); ++i) {
		Component* rootComp = model->GetComponentAt(i);
		const TopoDS_Shape& shape = rootComp->GetShape();

		// Get the relative linear deflection for a shape
		double linDeflection = OCCUtil::GetDeflection(shape);

		// Tessellate and add mesh data of a shape
		if (!OCCUtil::TessellateShape(shape, linDeflection, m_isRelative, m_angDeflection, true))
			wcout << "\tTessellation has failed on Shape: " << rootComp->GetName() << endl;
	}

	vector<Component*> comps;
	model->GetAllComponents(comps);

	for (const auto& comp : comps) {
		for (int i = 0; i < comp->GetIShapeSize(); ++i) {
			IShape* iShape = comp->GetIShapeAt(i);
			TessellateShape(iShape);
		}
	}

	comps.clear();
}

void Tessellator::TessellateShape(IShape*& iShape) const {
	if (iShape->IsFaceSet())
		AddMeshForFaceSet(iShape);
	else
		AddMeshForSketchGeometry(iShape);
}

void Tessellator::AddMeshForFaceSet(IShape*& iShape) const {
	const TopoDS_Shape& shape = iShape->GetShape();

	TopExp_Explorer ExpEdge;
	for (ExpEdge.Init(shape, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next()) {
		const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());

	}
	// Traverse faces
	TopExp_Explorer ExpFace;
	for (ExpFace.Init(shape, TopAbs_FACE); ExpFace.More(); ExpFace.Next()) {
		const TopoDS_Face& face = TopoDS::Face(ExpFace.Current());
		Mesh* mesh = GetMeshForFace(face);

		// Save the faceMesh
		if (mesh)
			iShape->AddMesh(mesh);
	}
	GProp_GProps props;
	BRepGProp::VolumeProperties(shape, props);
	Standard_Real volume = props.Mass();
	iShape->SetVolume(volume);
	iShape->SetTessellated(true);
}

void Tessellator::AddMeshForSketchGeometry(IShape*& iShape) const {
	const TopoDS_Shape& shape = iShape->GetShape();

	// Traverse edges
	TopExp_Explorer ExpEdge;
	for (ExpEdge.Init(shape, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next()) {
		const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());
		Mesh* mesh = GetMeshForEdge(edge);

		// Save the edgeMesh
		if (mesh)
			iShape->AddMesh(mesh);
	}

	iShape->SetTessellated(true);
}

Mesh* Tessellator::GetMeshForFace(const TopoDS_Face& face) const {
	TopLoc_Location loc;

	const Handle(Poly_Triangulation)& myT = BRep_Tool::Triangulation(face, loc);

	// Skip if triangulation has failed
	if (!myT || myT.IsNull())
		return nullptr;

	Mesh* mesh = new Mesh(face);

	const Poly_ArrayOfNodes& Nodes = myT->InternalNodes();

	// Add coordinates
	for (int i = Nodes.Lower(); i <= Nodes.Upper(); ++i) {
		const gp_Pnt& pnt = Nodes[i].Transformed(loc.Transformation());
		mesh->AddCoordinate(pnt.XYZ());
	}

	const TopAbs_Orientation& orientation = face.Orientation();
	const Poly_Array1OfTriangle& triangles = myT->InternalTriangles();

	// Add triangle indexes
	for (int i = 1; i <= myT->NbTriangles(); ++i) {
		int n1, n2, n3;
		triangles(i).Get(n1, n2, n3);

		// If a face is reversed, change the direction.
		if (orientation == TopAbs_REVERSED)
			swap(n1, n2);

		if (!IsTriangleValid(Nodes[n1 - 1], Nodes[n2 - 1], Nodes[n3 - 1]))
			continue;

		mesh->AddFaceIndex(n1, n2, n3);
	}
	double perimeterFace = 0.0;
	// Add boundary edges
	if (m_opt->GetEdge()) {
		TopExp_Explorer ExpEdge;
		for (ExpEdge.Init(face, TopAbs_EDGE); ExpEdge.More(); ExpEdge.Next()) {
			const TopoDS_Edge& edge = TopoDS::Edge(ExpEdge.Current());
			const Handle(Poly_PolygonOnTriangulation)& polygon = BRep_Tool::PolygonOnTriangulation(edge, myT, loc);
			const TColStd_Array1OfInteger& edgeNodes = polygon->Nodes();
			vector<int> edgeIndex;
			vector<double> edgePerimeter;
			for (int i = edgeNodes.Lower(); i <= edgeNodes.Upper(); ++i)
				edgeIndex.push_back(edgeNodes(i));

			mesh->AddEdgeIndex(edgeIndex);
			GProp_GProps System;
			BRepGProp::LinearProperties(edge, System);
			double perimeter = System.Mass();
			mesh->AddEdgePerimeter(perimeter);
			perimeterFace += System.Mass();
			edgeIndex.clear();
		}
	}
	mesh->SetPerimeter(perimeterFace);
	return mesh;
}

Mesh* Tessellator::GetMeshForEdge(const TopoDS_Edge& edge) const {
	TopLoc_Location loc;

	// Get a tessellated edge
	const Handle(Poly_Polygon3D)& myP = BRep_Tool::Polygon3D(edge, loc);

	if (!myP || myP.IsNull())
		return nullptr;

	Mesh* mesh = new Mesh(edge);

	const TColgp_Array1OfPnt& Nodes = myP->Nodes();

	vector<int> edgeIndex;

	// Add coordinates and edge index
	for (int i = Nodes.Lower(); i <= Nodes.Upper(); ++i) {
		const gp_Pnt& pnt = Nodes(i).Transformed(loc.Transformation());
		mesh->AddCoordinate(pnt.XYZ());
		edgeIndex.push_back(i);
	}

	// Save the edge index
	mesh->AddEdgeIndex(edgeIndex);
	edgeIndex.clear();

	return mesh;
}

bool Tessellator::IsTriangleValid(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) const {
	gp_Vec v1(p1, p2);
	gp_Vec v2(p2, p3);
	gp_Vec v3(p3, p1);
	gp_Vec v_norm = v1 ^ v2;

	// Tolerance (1.e-7)^2
	double tol = Precision::SquareConfusion();

	// Valid if the magnitudes are greater than tolerance
	if (v1.SquareMagnitude() > tol
		&& v2.SquareMagnitude() > tol
		&& v3.SquareMagnitude() > tol
		&& v_norm.SquareMagnitude() > tol)
		return true;

	return false;
}
