#pragma once

class Component;
class Mesh;
class IShape;

class Tessellator
{
public:
	Tessellator(InputOptions* opt);
	~Tessellator(void);

	void Tessellate(Model*& model) const;

protected:
	void TessellateModel(Model*& model) const;
	void TessellateShape(IShape*& iShape) const;
	
	void AddMeshForFaceSet(IShape*& iShape) const;
	void AddMeshForSketchGeometry(IShape*& iShape) const;

	Mesh* GetMeshForFace(const TopoDS_Face& face) const;
	Mesh* GetMeshForEdge(const TopoDS_Edge& edge) const;

	bool IsTriangleValid(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) const;

private:
	InputOptions* m_opt;

	double m_linDeflection;
	double m_angDeflection;

	bool m_isRelative;
};