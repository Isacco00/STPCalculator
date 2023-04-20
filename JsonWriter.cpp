#include "CommonImport.h"
#include "JsonWriter.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

JsonWriter::JsonWriter(InputOptions* opt)
	: m_opt(opt) {
	// Attributes for Appearance nodes
	m_diffuseColor.SetValues(0.55, 0.55, 0.6, Quantity_TOC_RGB);
	m_emissiveColor.SetValues(1.0, 1.0, 1.0, Quantity_TOC_RGB);
	m_specularColor.SetValues(0.2, 0.2, 0.2, Quantity_TOC_RGB);
	m_gdtColor.SetValues(0.5, 0.1, 0.1, Quantity_TOC_RGB);
	m_gdtColor2.SetValues(0.1, 0.1, 1, Quantity_TOC_RGB);

	m_shininess = 0.9;
	m_ambientIntensity = 1.0;
	m_transparency = 1.0;

	// Attribute for IndexedFaceSet required for JsonOM webvis
	m_creaseAngle = 0.2;
}

JsonWriter::~JsonWriter(void) {
	Clear();
}

void JsonWriter::WriteJson(Model*& model) {
	// Initial indent level
	int level = 0;
	json jsonContainer = json::object();
	json modelJson = json::object();
	modelJson["boundingBox"] = GetBoundingBox(model);
	modelJson["components"] = GetComponents(model);
	jsonContainer["model"] = modelJson;

	std::string jsonString = jsonContainer.dump();
	// Write JSON file
	wstring filePath = m_opt->GetOutputJson();
	wofstream wof;
	// This line is required to write Unicode characters.
	wof.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t, 0x10ffff, generate_header>));

	wof.open(filePath.c_str());
	wof << jsonString.c_str();
	wof.close();
}

json JsonWriter::GetBoundingBox(Model*& model) const {
	Bnd_Box bndBox = model->GetBoundingBox(m_opt->GetSketch());
	bndBox.SetGap(0.0);
	assert(!bndBox.IsVoid());

	double X_min = 0.0, Y_min = 0.0, Z_min = 0.0;
	double X_max = 0.0, Y_max = 0.0, Z_max = 0.0;

	bndBox.Get(X_min, Y_min, Z_min, X_max, Y_max, Z_max);
	json boundingBox = json::object();
	boundingBox["xMin"] = X_min;
	boundingBox["yMin"] = Y_min;
	boundingBox["zMin"] = Z_min;
	boundingBox["xMax"] = X_max;
	boundingBox["yMax"] = Y_max;
	boundingBox["zMax"] = Z_max;
	wstringstream ss_model;
	return boundingBox;
}

json JsonWriter::GetComponents(Model*& model) {
	json componentList = json::array();
	if (model->GetComponentSize() < 2) {
		for (int i = 0; i < model->GetComponentSize(); i++) {
			Component* rootComp = model->GetComponentAt(i);
			if (m_opt->GetSFA() // SFA-specific
				&& rootComp->GetIShapeSize() == 1
				&& rootComp->GetIShapeAt(0)->IsSketchGeometry()) {
				//TO MANAGE
				//CHECK IF SOME CASES HAVE SKETCH GEOMETRY
				//IShape* shape = rootComp->GetIShapeAt(0);
				//ss_model << WriteSketchGeometry(shape, level + 1);
			} else {
				componentList.push_back(WriteComponent(rootComp));
			}
		}
	}
	return componentList;
}

json JsonWriter::WriteComponent(Component*& comp) {
	//TO MANAGE
		//CHECK IF SOME CASES HAVE SUB COMPONENTS
		/*
		for (int i = 0; i < comp->GetSubComponentSize(); ++i) {
		Component* subComp = comp->GetSubComponentAt(i);
		bool isTransformed = OCCUtil::IsTransformed(subComp->GetTransformation());

		if (isTransformed) {
			ss_comp << Indent(level + 1);
			ss_comp << "<Transform";
			CountIndent(level + 1);

			if (m_opt->SFA())
				ss_comp << " id='" << subComp->GetName() << "'";

			// Transform attributes i.e. Translation and Rotation
			ss_comp << WriteTransformAttributes(subComp->GetTransformation()) << ">\n";
		} else
			level--;

		if (subComp->IsCopy()) {
			if (m_opt->SFA() // SFA-specific
				&& subComp->GetOriginalComponent()->GetSubComponentSize() == 0
				&& subComp->GetOriginalComponent()->GetIShapeSize() == 1
				&& subComp->GetOriginalComponent()->GetIShapeAt(0)->IsSketchGeometry()) {
				IShape* shape = subComp->GetOriginalComponent()->GetIShapeAt(0);
				ss_comp << WriteSketchGeometry(shape, level + 2);
			} else {
				wstring orgCompName = subComp->GetOriginalComponent()->GetName();

				ss_comp << Indent(level + 2);
				ss_comp << "<Group USE='" << orgCompName;

				if (m_opt->SFA())
					ss_comp << "'></Group>\n";
				else
					ss_comp << "'/>\n";

				CountIndent(level + 2);
			}
		} else {
			if (m_opt->SFA() // SFA-specific
				&& subComp->GetSubComponentSize() == 0
				&& subComp->GetIShapeSize() == 1
				&& subComp->GetIShapeAt(0)->IsSketchGeometry()) {
				IShape* shape = subComp->GetIShapeAt(0);
				ss_comp << WriteSketchGeometry(shape, level + 2);
			} else {
				ss_comp << Indent(level + 2);
				ss_comp << "<Group";

				if (m_opt->SFA()
					&& subComp->GetStepID() != -1)
					ss_comp << " id='msb " << subComp->GetStepID() << "'";

				ss_comp << " DEF='" << subComp->GetName() << "'>\n";
				CountIndent(level + 2);

				ss_comp << WriteComponent(subComp, level + 2); // Recursive call

				ss_comp << Indent(level + 2);
				ss_comp << "</Group>\n";
			}
		}

		if (isTransformed) {
			ss_comp << Indent(level + 1);
			ss_comp << "</Transform>\n";
		} else
			level++;
	}
	*/
	// Write shape nodes
	json shape = json::object();
	json shapeList = json::array();
	for (int i = 0; i < comp->GetIShapeSize(); ++i) {
		IShape* iShape = comp->GetIShapeAt(i);

		try {
			shapeList.push_back(WriteShape(iShape));
		} catch (...) {
			wcout << "Writing Json has failed on Shape: " << iShape->GetName() << endl;
		}
	}

	shape["shapes"] = shapeList;
	shape["componentName"] = comp->GetName().c_str();
	return shape;
}

wstring JsonWriter::WriteTransformAttributes(const gp_Trsf& trsf) const {
	wstringstream ss_trsf;

	if (OCCUtil::IsTranslated(trsf)) {
		const gp_XYZ& trans = trsf.TranslationPart();

		ss_trsf << " translation='";
		ss_trsf << NumTool::DoubleToWString(trans.X()) << " ";
		ss_trsf << NumTool::DoubleToWString(trans.Y()) << " ";
		ss_trsf << NumTool::DoubleToWString(trans.Z()) << "'";
	}

	if (OCCUtil::IsRotated(trsf)) {
		gp_Vec rotAxis;
		double rotAngle = 0.0;
		trsf.GetRotation().GetVectorAndAngle(rotAxis, rotAngle);

		ss_trsf << " rotation='";
		ss_trsf << NumTool::DoubleToWString(rotAxis.X()) << " ";
		ss_trsf << NumTool::DoubleToWString(rotAxis.Y()) << " ";
		ss_trsf << NumTool::DoubleToWString(rotAxis.Z()) << " ";
		ss_trsf << NumTool::DoubleToWString(rotAngle) << "'";
	}

	return ss_trsf.str();
}

json JsonWriter::WriteShape(IShape*& iShape) {
	json shape = json::object();

	if (iShape->IsFaceSet()) {
		wstring shapeId = iShape->GetUniqueName();
		shape["shapeID"] = shapeId.c_str();
		shape["shapeName"] = iShape->GetName().c_str();
		shape["stepID"] = iShape->GetStepID();
		shape["volume"] = iShape->GetVolume();
		vector<json> propertyList = WriteIndexedFaceSet(iShape);
		shape["appearance"] = propertyList[0];
		shape["faceSet"] = propertyList[1];
		shape["mesh"] = propertyList[2];
		/*
		if (m_opt->Edge()) // Boundary edges
		{
			ss_shape << Indent(level);
			ss_shape << "<Shape";

			if (m_opt->SFA())
				ss_shape << " id='" << shapeId << "'";

			ss_shape << " DEF='" << iShape->GetName() << "_edges'";
			ss_shape << ">\n";

			ss_shape << WriteIndexedLineSet(iShape, level + 1);

			ss_shape << Indent(level);
			ss_shape << "</Shape>\n";
		}
		*/
	} else { // Sketch geometry
		/*
		ss_shape << Indent(level);
		ss_shape << "<Shape";

		if (m_opt->SFA()
			&& iShape->GetStepID() != -1
			&& iShape->IsHidden())
			ss_shape << " id='curve 11 " << iShape->GetStepID() << "'";

		if (!m_opt->SFA())
			ss_shape << " DEF='" << iShape->GetName() << "'";

		ss_shape << ">\n";

		ss_shape << WriteIndexedLineSet(iShape, level + 1);

		ss_shape << Indent(level);
		ss_shape << "</Shape>\n";
		*/
	}

	return shape;
}

vector<json> JsonWriter::WriteIndexedFaceSet(IShape*& iShape) {
	vector<json> propertyList;
	double transparency = m_transparency;
	Quantity_ColorRGBA color(m_diffuseColor);
	propertyList.push_back(WriteAppearance(iShape, color.GetRGB(), true,
										   m_emissiveColor, false,
										   m_specularColor, true,
										   m_shininess, true,
										   m_ambientIntensity, false,
										   transparency, false));

	// Open IndexedFaceSet
	json faceSetProperty = json::object();
	faceSetProperty["creaseAngle"] = m_creaseAngle;
	faceSetProperty["solid"] = false;

	propertyList.push_back(faceSetProperty);
	json mesh = json::object();
	// Write coordinates
	propertyList.push_back(WriteMesh(iShape));
	return propertyList;
}

wstring JsonWriter::WriteIndexedLineSet(IShape*& iShape, int level) {
	wstringstream ss_ils;
	/*
	bool isMultiColored = iShape->IsMultiColored();

	// Write Appearance node
	if (iShape->IsSketchGeometry()) {
		if (!isMultiColored) {
			Quantity_ColorRGBA color(m_emissiveColor);

			if (m_opt->Color())
				color = iShape->GetColor();

			ss_ils << Indent(level);
			ss_ils << WriteAppearance(iShape, m_diffuseColor, false,
									  color.GetRGB(), true,
									  m_specularColor, false,
									  m_shininess, false,
									  m_ambientIntensity, false,
									  m_transparency, false);
		}
	} else {
		Quantity_Color color(0.0, 0.0, 0.0, Quantity_TOC_RGB);

		ss_ils << Indent(level);
		ss_ils << WriteAppearance(iShape, m_diffuseColor, false,
								  color, true,
								  m_specularColor, false,
								  m_shininess, false,
								  m_ambientIntensity, false,
								  m_transparency, false);
	}

	// Open IndexedLineSet
	ss_ils << Indent(level);
	ss_ils << "<IndexedLineSet";
	ss_ils << WriteCoordinateIndex(iShape, false) << ">\n";

	// Write coordinates
	ss_ils << Indent(level + 1);

	if (iShape->IsSketchGeometry())
		ss_ils << WriteCoordinate(iShape, false);
	else
		ss_ils << WriteCoordinate(iShape, true);

	// Write colors
	if (iShape->IsSketchGeometry()
		&& isMultiColored) {
		ss_ils << Indent(level + 1);
		ss_ils << WriteColor(iShape);
	}

	// Close IndexedLineSet
	ss_ils << Indent(level);
	ss_ils << "</IndexedLineSet>\n";
	*/
	return ss_ils.str();
}

json JsonWriter::WriteAppearance(IShape*& iShape, const Quantity_Color& diffuseColor, bool isDiffuseOn,
								 const Quantity_Color& emissiveColor, bool isEmissiveOn,
								 const Quantity_Color& specularColor, bool isSpecularOn,
								 double& shininess, bool isShininessOn,
								 double& ambientIntensity, bool isAmbientIntensityOn,
								 double& transparency, bool isTransparencyOn) {
	json appearanceProperty = json::object();
	wstringstream ss_app;
	if (isDiffuseOn) {
		json diffuseColorJson = json::object();
		diffuseColorJson["red"] = diffuseColor.Red();
		diffuseColorJson["green"] = diffuseColor.Green();
		diffuseColorJson["blue"] = diffuseColor.Blue();
		appearanceProperty["diffuseColor"] = diffuseColorJson;
	}
	if (isEmissiveOn) {
		json emissiveColorJson = json::object();
		emissiveColorJson["red"] = emissiveColor.Red();
		emissiveColorJson["green"] = emissiveColor.Green();
		emissiveColorJson["blue"] = emissiveColor.Blue();
		appearanceProperty["emissiveColor"] = emissiveColorJson;
	}
	if (isSpecularOn) {
		json specularColorJson = json::object();
		specularColorJson["red"] = specularColor.Red();
		specularColorJson["green"] = specularColor.Green();
		specularColorJson["blue"] = specularColor.Blue();
		appearanceProperty["specularColor"] = specularColorJson;
	}
	if (isShininessOn) {
		appearanceProperty["shininess"] = shininess;
	}
	if (isAmbientIntensityOn) {
		appearanceProperty["ambientIntensity"] = ambientIntensity;
	}
	if (isTransparencyOn) {
		appearanceProperty["transparency"] = transparency;
	}
	return appearanceProperty;
}

json JsonWriter::WriteMesh(IShape*& iShape) const {
	json meshListJson = json::array();
	int prevCoordCount = 0; // The number of previous coordinates
	for (int i = 0; i < iShape->GetMeshSize(); ++i) {
		Mesh* mesh = iShape->GetMeshAt(i);
		json meshJson = json::object();
		json meshCoordinates = json::array();
		for (int j = 0; j < mesh->GetCoordinateSize(); ++j) {
			const gp_XYZ& coord = mesh->GetCoordinateAt(j);
			json coordinate = json::object();
			coordinate["x"] = coord.X();
			coordinate["y"] = coord.Y();
			coordinate["z"] = coord.Z();
			meshCoordinates.push_back(coordinate);
		}
		meshJson["coordinates"] = meshCoordinates;

		// Traverse triangles
		wstringstream ss_coordIndex;
		for (int j = 0; j < mesh->GetFaceIndexSize(); ++j) {
			const vector<int>& faceIndex = mesh->GetFaceIndexAt(j);
			ss_coordIndex << to_wstring(faceIndex[0] - 1 + prevCoordCount) << " ";
			ss_coordIndex << to_wstring(faceIndex[1] - 1 + prevCoordCount) << " ";
			ss_coordIndex << to_wstring(faceIndex[2] - 1 + prevCoordCount) << " ";
			ss_coordIndex << "-1 ";
		}
		wstring coordIndex = ss_coordIndex.str();
		meshJson["coordIndex"] = coordIndex.c_str();

		// Traverse edges
		wstringstream ss_edgeIndex;
		for (int j = 0; j < mesh->GetEdgeIndexSize(); ++j) {
			const vector<int>& edgeIndex = mesh->GetEdgeIndexAt(j);

			for (size_t k = 0; k < edgeIndex.size(); ++k) {
				int index = edgeIndex[k] - 1 + prevCoordCount;
				ss_edgeIndex << to_wstring(index) << " ";
				//cout << "			" << index << endl;
			}
			ss_edgeIndex << "-1 ";
		}
		wstring edgeIndex = ss_edgeIndex.str();
		meshJson["edgeIndex"] = edgeIndex.c_str();
		meshJson["edgePerimeter"] = mesh->GetEdgePerimeter();

		prevCoordCount += mesh->GetCoordinateSize();
		meshListJson.push_back(meshJson);
	}
	return meshListJson;
}

wstring JsonWriter::WriteCoordinateIndex(IShape*& iShape, bool faceMesh) const {
	wstringstream ss_coordIndex;
	ss_coordIndex << " coordIndex='";

	int prevCoordCount = 0; // The number of previous coordinates
	int meshSize = iShape->GetMeshSize();
	for (int i = 0; i < iShape->GetMeshSize(); ++i) {
		Mesh* mesh = iShape->GetMeshAt(i);

		if (faceMesh) // Face mesh
		{
			// Traverse triangles
			for (int j = 0; j < mesh->GetFaceIndexSize(); ++j) {
				const vector<int>& faceIndex = mesh->GetFaceIndexAt(j);

				ss_coordIndex << to_wstring(faceIndex[0] - 1 + prevCoordCount) << " ";
				ss_coordIndex << to_wstring(faceIndex[1] - 1 + prevCoordCount) << " ";
				ss_coordIndex << to_wstring(faceIndex[2] - 1 + prevCoordCount) << " ";
				ss_coordIndex << "-1 ";
			}
		} else // Edge mesh (Boundary edges, sketch geometry)
		{
			// Traverse edges
			for (int j = 0; j < mesh->GetEdgeIndexSize(); ++j) {
				const vector<int>& edgeIndex = mesh->GetEdgeIndexAt(j);

				for (size_t k = 0; k < edgeIndex.size(); ++k) {
					int index = edgeIndex[k] - 1 + prevCoordCount;
					ss_coordIndex << to_wstring(index) << " ";
					//cout << "			" << index << endl;
				}

				ss_coordIndex << "-1 ";
			}
		}

		prevCoordCount += mesh->GetCoordinateSize();
	}

	ss_coordIndex << "'";

	return CleanString(ss_coordIndex.str());
}

wstring JsonWriter::WriteNormalIndex(IShape*& iShape) const {
	wstringstream ss_normalIndex;
	ss_normalIndex << " normalIndex='";

	int prevCoordCount = 0; // The number of previous coordinates

	for (int i = 0; i < iShape->GetMeshSize(); ++i) {
		Mesh* mesh = iShape->GetMeshAt(i);

		// Traverse triangles
		for (int j = 0; j < mesh->GetNormalIndexSize(); ++j) {
			const vector<int>& normalIndex = mesh->GetNormalIndexAt(j);

			ss_normalIndex << to_wstring(normalIndex[0] - 1 + prevCoordCount) << " ";
			ss_normalIndex << to_wstring(normalIndex[1] - 1 + prevCoordCount) << " ";
			ss_normalIndex << to_wstring(normalIndex[2] - 1 + prevCoordCount) << " ";
			ss_normalIndex << "-1 ";
		}

		prevCoordCount += mesh->GetCoordinateSize();
	}

	ss_normalIndex << "'";

	return CleanString(ss_normalIndex.str());
}

wstring JsonWriter::WriteColor(IShape*& iShape) const {
	wstringstream ss_colors;

	ss_colors << "<Color color='";

	// Write colors for each coordinate point
	for (int i = 0; i < iShape->GetMeshSize(); ++i) {
		Mesh* mesh = iShape->GetMeshAt(i);

		for (int j = 0; j < mesh->GetCoordinateSize(); ++j) {
			const Quantity_ColorRGBA& color = iShape->GetColor(mesh->GetShape());
			ss_colors << NumTool::DoubleToWString(color.GetRGB().Red()) << " ";
			ss_colors << NumTool::DoubleToWString(color.GetRGB().Green()) << " ";
			ss_colors << NumTool::DoubleToWString(color.GetRGB().Blue()) << " ";
		}
	}
	ss_colors << "'></Color>\n";
	return CleanString(ss_colors.str());
}

wstring JsonWriter::WriteNormal(IShape*& iShape) const {
	wstringstream ss_normals;

	ss_normals << "<Normal vector='";

	for (int i = 0; i < iShape->GetMeshSize(); ++i) {
		Mesh* mesh = iShape->GetMeshAt(i);

		for (int j = 0; j < mesh->GetNormalSize(); ++j) {
			const gp_XYZ& normal = mesh->GetNormalAt(j);

			ss_normals << NumTool::DoubleToWString(normal.X()) << " ";
			ss_normals << NumTool::DoubleToWString(normal.Y()) << " ";
			ss_normals << NumTool::DoubleToWString(normal.Z()) << " ";
		}
	}

	ss_normals << "'></Normal>\n";

	return CleanString(ss_normals.str());
}

const wstring JsonWriter::Indent(int level) const {
	wstring indent;
	wstring unit = L" ";	// space or tab

	for (int i = 0; i < level; ++i)
		indent += unit;

	return indent;
}

const wstring JsonWriter::CleanString(wstring str) const {
	wstring from = L" '";
	wstring to = L"'";

	// Remove a blank at the end of '... ' string
	str = StrTool::ReplaceCharacter(str, from, to);

	return str;
}

bool JsonWriter::CheckSameAppearance(const Quantity_Color& diffuseColor, bool isDiffuseOn,
									 const Quantity_Color& emissiveColor, bool isEmissiveOn,
									 const Quantity_Color& specularColor, bool isSpecularOn,
									 double& shininess, bool isShininessOn,
									 double& ambientIntensity, bool isAmbientIntensityOn,
									 double& transparency, bool isTransparencyOn,
									 int& appID) {
	// Search for the same appearance
	for (int i = 0; i < (int)m_appearances.size(); ++i) {
		AppearanceJson app = m_appearances[i];

		if (app.isDiffuseOn == isDiffuseOn
			&& ((isDiffuseOn && app.diffuseColor.IsEqual(diffuseColor))
				|| !isDiffuseOn) &&
			app.isEmissiveOn == isEmissiveOn
			&& ((isEmissiveOn && app.emissiveColor.IsEqual(emissiveColor))
				|| !isEmissiveOn) &&
			app.isSpecularOn == isSpecularOn
			&& ((isSpecularOn && app.specularColor.IsEqual(specularColor))
				|| !isSpecularOn) &&
			app.isShininessOn == isShininessOn
			&& ((isShininessOn && abs(app.shininess - shininess) <= Precision::Confusion())
				|| !isShininessOn) &&
			app.isAmbientIntensityOn == isAmbientIntensityOn
			&& ((isAmbientIntensityOn && abs(app.ambientIntensity - ambientIntensity) <= Precision::Confusion())
				|| !isAmbientIntensityOn) &&
			app.isTransparencyOn == isTransparencyOn
			&& ((isTransparencyOn && abs(app.transparency - transparency) <= Precision::Confusion())
				|| !isTransparencyOn)) {
			appID = i; // Save the order
			return true;
		}
	}

	// Save the current appearance
	AppearanceJson app;
	app.diffuseColor = diffuseColor;
	app.emissiveColor = emissiveColor;
	app.specularColor = specularColor;
	app.shininess = shininess;
	app.ambientIntensity = ambientIntensity;
	app.transparency = transparency;
	app.isDiffuseOn = isDiffuseOn;
	app.isEmissiveOn = isEmissiveOn;
	app.isSpecularOn = isSpecularOn;
	app.isShininessOn = isShininessOn;
	app.isAmbientIntensityOn = isAmbientIntensityOn;
	app.isTransparencyOn = isTransparencyOn;
	m_appearances.push_back(app);

	// Save the latest order
	appID = (int)m_appearances.size() - 1;

	return false;
}

void JsonWriter::CountIndent(int level) {
	// Used for counting indent of 'Transform' and 'Group' nodes
	if (m_indentCountMap.find(level) == m_indentCountMap.end())
		m_indentCountMap.insert({ level, 1 });
	else {
		int count = m_indentCountMap[level];
		count++;
		m_indentCountMap[level] = count;
	}
}

void JsonWriter::PrintIndentCount(void) {
	printf("Indent Count\n");

	for (map<int, int>::iterator it = m_indentCountMap.begin(); it != m_indentCountMap.end(); ++it)
		printf("indent %d - %d\n", it->first, it->second);
}

void JsonWriter::PrintMaterialCount(void) const {
	printf("Number of Materials: %d\n", (int)m_appearances.size());
}

void JsonWriter::Clear(void) {
	m_appearances.clear();
	m_indentCountMap.clear();
}
