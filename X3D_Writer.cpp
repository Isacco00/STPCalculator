#include "CommonImport.h"
#include "X3D_Writer.h"
#include "Component.h"
#include "IShape.h"
#include "Mesh.h"

X3D_Writer::X3D_Writer(InputOptions* opt)
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

	// Attribute for IndexedFaceSet required for X3DOM webvis
	m_creaseAngle = 0.2;
}

X3D_Writer::~X3D_Writer(void) {
	Clear();
}

void X3D_Writer::WriteX3D(Model*& model) {
	wstringstream ss_x3d;

	// Initial indent level
	int level = 0;

	// Open header
	ss_x3d << OpenHeader();

	// Write viewpoint
	ss_x3d << WriteViewpoint(model, level + 1);

	// Write model
	ss_x3d << WriteModel(model, level + 1);

	// Close header
	ss_x3d << CloseHeader();

	// Write X3D file
	wstring filePath = m_opt->GetOutput();

	wofstream wof;

	// This line is required to write Unicode characters.
	wof.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t, 0x10ffff, generate_header>));

	wof.open(filePath.c_str());
	wof << ss_x3d.str().c_str();
	wof.close();

	/// Print results required for SFA
	if (m_opt->GetSFA()) {
		PrintIndentCount();
		PrintMaterialCount();
	}
	///

	ss_x3d.clear();
}

wstring X3D_Writer::OpenHeader(void) const {
	wstringstream ss_hd;

	if (m_opt->GetHtml()) {
		ss_hd << "<html>\n";
		ss_hd << "<head>\n";
		ss_hd << " <link rel='stylesheet' type='text/css' href='https://www.x3dom.org/x3dom/release/x3dom.css'/>\n";
		ss_hd << " <script type='text/javascript' src='https://www.x3dom.org/x3dom/release/x3dom.js'></script>\n";
		ss_hd << "</head>\n";
		ss_hd << "<body>\n";
	} else {
		ss_hd << "<?xml version='1.0' encoding='UTF-8'?>\n";
	}

	ss_hd << "<X3D version='3.3'>\n";
	ss_hd << "<head>\n";
	ss_hd << "</head>\n";
	ss_hd << "<Scene>\n";

	return ss_hd.str();
}

wstring X3D_Writer::CloseHeader(void) const {
	wstringstream ss_hd;

	ss_hd << "</Scene>\n";
	ss_hd << "</X3D>";

	if (m_opt->GetHtml()) {
		ss_hd << "\n";
		ss_hd << "</body>\n";
		ss_hd << "</html>";
	}

	return ss_hd.str();
}

wstring X3D_Writer::WriteViewpoint(Model*& model, int level) const {
	if (!m_opt->GetHtml())
		return L"";

	wstringstream ss_vp;

	Bnd_Box bndBox = model->GetBoundingBox(m_opt->GetSketch());
	assert(!bndBox.IsVoid());

	double X_min = 0.0, Y_min = 0.0, Z_min = 0.0;
	double X_max = 0.0, Y_max = 0.0, Z_max = 0.0;

	bndBox.Get(X_min, Y_min, Z_min, X_max, Y_max, Z_max);

	double X_mean = (X_min + X_max) / 2;
	double Y_mean = (Y_min + Y_max) / 2;
	double Z_mean = (Z_min + Z_max) / 2;

	double X_pos = X_mean;
	double Y_pos = Y_mean;
	double Z_pos = Z_mean;

	double X_ori = 1.0;
	double Y_ori = 0.0;
	double Z_ori = 0.0;
	double R_ori = PI / 2;

	double X_gap = X_max - X_min;
	double Y_gap = Y_max - Y_min;
	double Z_gap = Z_max - Z_min;

	if (X_gap >= Y_gap
		&& X_gap >= Z_gap)
		Y_pos = (-2) * X_gap;
	else if (Y_gap >= X_gap
			 && Y_gap >= Z_gap)
		Y_pos = (-2) * Y_gap;
	else if (Z_gap >= X_gap
			 && Z_gap >= Y_gap)
		Y_pos = (-2) * Z_gap;

	ss_vp << Indent(level);
	ss_vp << "<Viewpoint";

	ss_vp << " position='";
	ss_vp << NumTool::DoubleToWString(X_pos) << " ";
	ss_vp << NumTool::DoubleToWString(Y_pos) << " ";
	ss_vp << NumTool::DoubleToWString(Z_pos) << "'";

	ss_vp << " orientation='";
	ss_vp << NumTool::DoubleToWString(X_ori) << " ";
	ss_vp << NumTool::DoubleToWString(Y_ori) << " ";
	ss_vp << NumTool::DoubleToWString(Z_ori) << " ";
	ss_vp << NumTool::DoubleToWString(R_ori) << "'";

	ss_vp << " centerOfRotation='";
	ss_vp << NumTool::DoubleToWString(X_mean) << " ";
	ss_vp << NumTool::DoubleToWString(Y_mean) << " ";
	ss_vp << NumTool::DoubleToWString(Z_mean) << "'";

	ss_vp << "></Viewpoint>\n";

	return ss_vp.str();
}

wstring X3D_Writer::WriteModel(Model*& model, int level) {
	wstringstream ss_model;

	if (model->GetComponentSize() >= 2) {
		ss_model << Indent(level);
		ss_model << "<Group>\n";
		CountIndent(level);
	} else
		level--;

	// Write root components
	for (int i = 0; i < model->GetComponentSize(); ++i) {
		Component* rootComp = model->GetComponentAt(i);

		if (m_opt->GetSFA() // SFA-specific
			&& rootComp->GetIShapeSize() == 1
			&& rootComp->GetIShapeAt(0)->IsSketchGeometry()) {
			IShape* shape = rootComp->GetIShapeAt(0);
			ss_model << WriteSketchGeometry(shape, level + 1);
		} else {
			ss_model << Indent(level + 1);
			ss_model << "<Group";

			ss_model << " DEF='" << rootComp->GetName() << "'>\n";
			CountIndent(level + 1);

			ss_model << WriteComponent(rootComp, level + 1);

			ss_model << Indent(level + 1);
			ss_model << "</Group>\n";
		}
	}

	if (model->GetComponentSize() >= 2) {
		ss_model << Indent(level);
		ss_model << "</Group>\n";
	}

	return ss_model.str();
}

wstring X3D_Writer::WriteComponent(Component*& comp, int level) {
	wstringstream ss_comp;

	// Write shape nodes
	for (int i = 0; i < comp->GetIShapeSize(); ++i) {
		IShape* iShape = comp->GetIShapeAt(i);

		try {
			ss_comp << WriteShape(iShape, level + 1);
		} catch (...) {
			wcout << "Writing X3D has failed on Shape: " << iShape->GetName() << endl;
		}
	}

	return ss_comp.str();
}

wstring X3D_Writer::WriteTransformAttributes(const gp_Trsf& trsf) const {
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

wstring X3D_Writer::WriteShape(IShape*& iShape, int level) {
	wstringstream ss_shape;

	if (iShape->IsFaceSet()) {
		wstring shapeId = iShape->GetUniqueName();

		ss_shape << Indent(level);
		ss_shape << "<Shape";

		if (m_opt->GetSFA())
			ss_shape << " id='" << shapeId << "'";

		ss_shape << " DEF='" << iShape->GetName() << "'";
		ss_shape << ">\n";

		ss_shape << WriteIndexedFaceSet(iShape, level + 1);

		ss_shape << Indent(level);
		ss_shape << "</Shape>\n";

		if (m_opt->GetEdge()) // Boundary edges
		{
			ss_shape << Indent(level);
			ss_shape << "<Shape";

			if (m_opt->GetSFA())
				ss_shape << " id='" << shapeId << "'";

			ss_shape << " DEF='" << iShape->GetName() << "_edges'";
			ss_shape << ">\n";

			ss_shape << WriteIndexedLineSet(iShape, level + 1);

			ss_shape << Indent(level);
			ss_shape << "</Shape>\n";
		}
	} else // Sketch geometry
	{
		ss_shape << Indent(level);
		ss_shape << "<Shape";

		if (!m_opt->GetSFA())
			ss_shape << " DEF='" << iShape->GetName() << "'";

		ss_shape << ">\n";

		ss_shape << WriteIndexedLineSet(iShape, level + 1);

		ss_shape << Indent(level);
		ss_shape << "</Shape>\n";
	}

	return ss_shape.str();
}

wstring X3D_Writer::WriteIndexedFaceSet(IShape*& iShape, int level) {
	wstringstream ss_ifs;

	double transparency = m_transparency;

	// Write Appearance node
	ss_ifs << Indent(level);

	Quantity_ColorRGBA color(m_diffuseColor);

	ss_ifs << WriteAppearance(iShape, color.GetRGB(), true,
							  m_emissiveColor, false,
							  m_specularColor, true,
							  m_shininess, true,
							  m_ambientIntensity, false,
							  transparency, false);

	// Open IndexedFaceSet
	ss_ifs << Indent(level);
	ss_ifs << "<IndexedFaceSet";

	ss_ifs << " creaseAngle='" << NumTool::DoubleToWString(m_creaseAngle) << "'";

	ss_ifs << " solid='false'";

	ss_ifs << WriteCoordinateIndex(iShape, true);

	ss_ifs << ">\n";

	// Write coordinates
	ss_ifs << Indent(level + 1);
	ss_ifs << WriteCoordinate(iShape, false);

	// Close IndexedFaceSet
	ss_ifs << Indent(level);
	ss_ifs << "</IndexedFaceSet>\n";

	return ss_ifs.str();
}

wstring X3D_Writer::WriteIndexedLineSet(IShape*& iShape, int level) {
	wstringstream ss_ils;

	// Write Appearance node
	if (iShape->IsSketchGeometry()) {
		Quantity_ColorRGBA color(m_emissiveColor);
		ss_ils << Indent(level);
		ss_ils << WriteAppearance(iShape, m_diffuseColor, false,
								  color.GetRGB(), true,
								  m_specularColor, false,
								  m_shininess, false,
								  m_ambientIntensity, false,
								  m_transparency, false);
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

	// Close IndexedLineSet
	ss_ils << Indent(level);
	ss_ils << "</IndexedLineSet>\n";

	return ss_ils.str();
}

wstring X3D_Writer::WriteAppearance(IShape*& iShape, const Quantity_Color& diffuseColor, bool isDiffuseOn,
									const Quantity_Color& emissiveColor, bool isEmissiveOn,
									const Quantity_Color& specularColor, bool isSpecularOn,
									double& shininess, bool isShininessOn,
									double& ambientIntensity, bool isAmbientIntensityOn,
									double& transparency, bool isTransparencyOn) {
	wstringstream ss_app;

	int appID = 0;

	//if (!m_opt->SFA()
	//	|| (m_opt->SFA() 
	//		&& iShape->IsFaceSet()))
	//{
	if (CheckSameAppearance(diffuseColor, isDiffuseOn,
							emissiveColor, isEmissiveOn,
							specularColor, isSpecularOn,
							shininess, isShininessOn,
							ambientIntensity, isAmbientIntensityOn,
							transparency, isTransparencyOn,
							appID)) {
		ss_app << "<Appearance USE='app" << to_wstring(appID) << "'></Appearance>\n";

		return ss_app.str();
	}
	//}

	// Write Appearance node
	ss_app << "<Appearance";

	//if (!m_opt->SFA()
	//	|| (m_opt->SFA() 
	//		&& iShape->IsFaceSet()))
	ss_app << " DEF='app" << to_wstring(appID) << "'";

	ss_app << "><Material";

	if (m_opt->GetSFA()
		//&& iShape->IsFaceSet()
		)
		ss_app << " id='mat" << to_wstring(appID) << "'";

	if (isDiffuseOn) {
		ss_app << " diffuseColor='";
		ss_app << NumTool::DoubleToWString(diffuseColor.Red()) << " ";
		ss_app << NumTool::DoubleToWString(diffuseColor.Green()) << " ";
		ss_app << NumTool::DoubleToWString(diffuseColor.Blue()) << "'";
	}

	if (isEmissiveOn) {
		ss_app << " emissiveColor='";
		ss_app << NumTool::DoubleToWString(emissiveColor.Red()) << " ";
		ss_app << NumTool::DoubleToWString(emissiveColor.Green()) << " ";
		ss_app << NumTool::DoubleToWString(emissiveColor.Blue()) << "'";
	}

	if (isSpecularOn) {
		ss_app << " specularColor='";
		ss_app << NumTool::DoubleToWString(specularColor.Red()) << " ";
		ss_app << NumTool::DoubleToWString(specularColor.Green()) << " ";
		ss_app << NumTool::DoubleToWString(specularColor.Blue()) << "'";
	}

	if (isShininessOn) {
		ss_app << " shininess='";
		ss_app << NumTool::DoubleToWString(shininess) << "'";
	}

	if (isAmbientIntensityOn) {
		ss_app << " ambientIntensity='";
		ss_app << NumTool::DoubleToWString(ambientIntensity) << "'";
	}

	if (isTransparencyOn) {
		ss_app << " transparency='";
		ss_app << NumTool::DoubleToWString(transparency) << "'";
	}

	ss_app << "></Material></Appearance>\n";

	return ss_app.str();
}

wstring X3D_Writer::WriteCoordinate(IShape*& iShape, bool isBoundaryEdges) const {
	wstringstream ss_coords;

	ss_coords << "<Coordinate";

	if (!isBoundaryEdges) {
		if (m_opt->GetEdge()
			&& iShape->IsFaceSet())
			ss_coords << " DEF='c" << to_wstring(iShape->GetGlobalIndex()) << "'";

		ss_coords << " point='";

		for (int i = 0; i < iShape->GetMeshSize(); ++i) {
			Mesh* mesh = iShape->GetMeshAt(i);

			for (int j = 0; j < mesh->GetCoordinateSize(); ++j) {
				const gp_XYZ& coord = mesh->GetCoordinateAt(j);

				ss_coords << NumTool::DoubleToWString(coord.X()) << " ";
				ss_coords << NumTool::DoubleToWString(coord.Y()) << " ";
				ss_coords << NumTool::DoubleToWString(coord.Z()) << " ";
			}
		}
	} else {
		ss_coords << " USE='c" << to_wstring(iShape->GetGlobalIndex());
	}

	if (m_opt->GetSFA())
		ss_coords << "'></Coordinate>\n";
	else
		ss_coords << "'/>\n";

	return CleanString(ss_coords.str());
}

wstring X3D_Writer::WriteCoordinateIndex(IShape*& iShape, bool faceMesh) const {
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
		} else { // Edge mesh (Boundary edges, sketch geometry)

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

wstring X3D_Writer::WriteNormalIndex(IShape*& iShape) const {
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

wstring X3D_Writer::WriteColor(IShape*& iShape) const {
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

wstring X3D_Writer::WriteNormal(IShape*& iShape) const {
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

const wstring X3D_Writer::Indent(int level) const {
	wstring indent;
	wstring unit = L" ";	// space or tab

	for (int i = 0; i < level; ++i)
		indent += unit;

	return indent;
}

const wstring X3D_Writer::CleanString(wstring str) const {
	wstring from = L" '";
	wstring to = L"'";

	// Remove a blank at the end of '... ' string
	str = StrTool::ReplaceCharacter(str, from, to);

	return str;
}

bool X3D_Writer::CheckSameAppearance(const Quantity_Color& diffuseColor, bool isDiffuseOn,
									 const Quantity_Color& emissiveColor, bool isEmissiveOn,
									 const Quantity_Color& specularColor, bool isSpecularOn,
									 double& shininess, bool isShininessOn,
									 double& ambientIntensity, bool isAmbientIntensityOn,
									 double& transparency, bool isTransparencyOn,
									 int& appID) {
	// Search for the same appearance
	for (int i = 0; i < (int)m_appearances.size(); ++i) {
		Appearance app = m_appearances[i];

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
	Appearance app;
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

wstring X3D_Writer::WriteSketchGeometry(IShape*& iShape, int level) {
	wstringstream ss_sg;

	ss_sg << Indent(level);
	ss_sg << "<Shape>\n";

	// Write Appearance node
	Quantity_Color color;
	color = m_emissiveColor;
	ss_sg << Indent(level + 1);
	ss_sg << "<Appearance><Material";
	ss_sg << " emissiveColor='";
	ss_sg << NumTool::DoubleToWString(color.Red()) << " ";
	ss_sg << NumTool::DoubleToWString(color.Green()) << " ";
	ss_sg << NumTool::DoubleToWString(color.Blue()) << "'";
	ss_sg << "></Material></Appearance>\n";

	// Open IndexedLineSet
	ss_sg << Indent(level + 1);
	ss_sg << "<IndexedLineSet";
	ss_sg << WriteCoordinateIndex(iShape, false) << ">\n";

	// Write coordinates
	ss_sg << Indent(level + 2);
	ss_sg << WriteCoordinate(iShape, false);

	// Close IndexedLineSet
	ss_sg << Indent(level + 1);
	ss_sg << "</IndexedLineSet>\n";

	ss_sg << Indent(level);
	ss_sg << "</Shape>\n";

	return ss_sg.str();
}

wstring X3D_Writer::WriteHiddenGeometry(Component*& comp, int level) {
	wstringstream ss_hg;

	if (m_opt->GetSFA()) // SFA-specific
	{
		ss_hg << "<!--composites-->\n";
		ss_hg << Indent(level) << "<Switch whichChoice='0' id='swComposites1'><Group>\n";
	} else
		level--;

	// Write shape nodes
	for (int i = 0; i < comp->GetIShapeSize(); ++i) {
		IShape* iShape = comp->GetIShapeAt(i);

		try {
			ss_hg << WriteShape(iShape, level + 1);
		} catch (...) {
			wcout << "Writing X3D has failed on Shape: " << iShape->GetName() << endl;
		}
	}

	if (m_opt->GetSFA()) // SFA-specific
	{
		ss_hg << Indent(level) << "</Group></Switch>\n";
	}

	return ss_hg.str();
}

void X3D_Writer::CountIndent(int level) {
	// Used for counting indent of 'Transform' and 'Group' nodes
	if (m_indentCountMap.find(level) == m_indentCountMap.end())
		m_indentCountMap.insert({ level, 1 });
	else {
		int count = m_indentCountMap[level];
		count++;
		m_indentCountMap[level] = count;
	}
}

void X3D_Writer::PrintIndentCount(void) {
	printf("Indent Count\n");

	for (map<int, int>::iterator it = m_indentCountMap.begin(); it != m_indentCountMap.end(); ++it)
		printf("indent %d - %d\n", it->first, it->second);
}

void X3D_Writer::PrintMaterialCount(void) const {
	printf("Number of Materials: %d\n", (int)m_appearances.size());
}

void X3D_Writer::Clear(void) {
	m_appearances.clear();
	m_indentCountMap.clear();
}
