#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Component;
class IShape;

struct AppearanceJson {
	Quantity_Color diffuseColor;
	Quantity_Color specularColor;
	Quantity_Color emissiveColor;
	double shininess = 0.0;
	double ambientIntensity = 0.0;
	double transparency = 0.0;

	bool isDiffuseOn = false;
	bool isEmissiveOn = false;
	bool isSpecularOn = false;
	bool isShininessOn = false;
	bool isAmbientIntensityOn = false;
	bool isTransparencyOn = false;
};

class JsonWriter {
public:
	JsonWriter(InputOptions* opt);
	~JsonWriter(void);

	void WriteJson(Model*& model);

protected:
	json GetBoundingBox(Model*& model) const;

	json GetComponents(Model*& model);
	json WriteComponent(Component*& comp);

	wstring WriteTransformAttributes(const gp_Trsf& trsf) const;
	json WriteShape(IShape*& iShape);
	vector<json> WriteIndexedFaceSet(IShape*& iShape);
	wstring WriteIndexedLineSet(IShape*& iShape, int level);

	json WriteAppearance(IShape*& iShape, const Quantity_Color& diffuseColor, bool isDiffuseOn,
							const Quantity_Color& emissiveColor, bool isEmissiveOn,
							const Quantity_Color& specularColor, bool isSpecularOn,
							double& shininess, bool isShininessOn,
							double& ambientIntensity, bool isAmbientIntensityOn,
							double& transparency, bool isTransparencyOn);
	json WriteMesh(IShape*& iShape) const;
	wstring WriteCoordinateIndex(IShape*& iShape, bool faceMesh) const;
	wstring WriteNormalIndex(IShape*& iShape) const;
	wstring WriteColor(IShape*& iShape) const;
	wstring WriteNormal(IShape*& iShape) const;

	const wstring Indent(int level) const;
	const wstring CleanString(wstring str) const;

	bool CheckSameAppearance(const Quantity_Color& diffuseColor, bool isDiffuseOn,
							 const Quantity_Color& emissiveColor, bool isEmissiveOn,
							 const Quantity_Color& specularColor, bool isSpecularOn,
							 double& shininess, bool isShininessOn,
							 double& ambientIntensity, bool isAmbientIntensityOn,
							 double& transparency, bool isTransparencyOn,
							 int& appID);
	void Clear(void);

	// SFA-specific functions
	wstring WriteSketchGeometry(IShape*& iShape, int level);
	wstring WriteHiddenGeometry(Component*& comp, int level);
	void CountIndent(int level);
	void PrintIndentCount(void);
	void PrintMaterialCount(void) const;

private:
	InputOptions* m_opt;

	Quantity_Color m_diffuseColor;
	Quantity_Color m_emissiveColor;
	Quantity_Color m_specularColor;
	Quantity_Color m_gdtColor, m_gdtColor2;

	double m_shininess;
	double m_ambientIntensity;
	double m_transparency;

	double m_creaseAngle;

	vector<AppearanceJson> m_appearances;

	// SFA-specific variables
	map<int, int> m_indentCountMap;
};