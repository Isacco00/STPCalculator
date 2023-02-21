#pragma once

class IShape;

class Component {
public:
	Component(const TopoDS_Shape& shape);
	~Component(void);

	void SetName(const wstring& name) { m_name = name; }
	void AddIShape(IShape*& iShape);
	const wstring& GetName(void) const { return m_name; }
	const wstring& GetUniqueName(void) const { return m_uniqueName; }
	const TopoDS_Shape& GetShape(void) const { return m_shape; }
	Component* GetParentComponent(void) const { return m_parentComponent; }
	IShape* GetIShapeAt(const int index) const { return m_iShapes[index]; }
	const int GetIShapeSize(void) const { return (int)m_iShapes.size(); }
	const Bnd_Box GetBoundingBox(bool sketch) const;

	bool HasUniqueName(void) const { return m_hasUniqueName; }
	bool IsRoot(void) const;
	bool IsEmpty(void) const;

	void Clean(void);

	// SFA-specific
	void SetStepID(int stepID) { m_stepID = stepID; }
	const int GetStepID(void) const { return m_stepID; }

protected:
	void Clear(void);
	void CleanEmptyIShapes(void);
	void ClearIShapes(void) { m_iShapes.clear(); }

private:
	wstring m_name;
	wstring m_uniqueName;
	TopoDS_Shape m_shape;
	bool m_hasUniqueName;
	int m_stepID;
	Component* m_parentComponent;
	vector<IShape*> m_iShapes;
};