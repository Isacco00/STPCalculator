#pragma once

// OpenCascade includes
#include <BinXCAFDrivers.hxx>
#include <TDocStd_Application.hxx>

#include <Poly.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

#include <STEPControl_Reader.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPConstruct_Styles.hxx>

#include <TDocStd_Document.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ViewTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_View.hxx>
#include <XCAFDoc_ViewTool.hxx>
#include <XCAFDoc_Note.hxx>
#include <XCAFDoc_NotesTool.hxx>
#include <XCAFDoc_Datum.hxx>
#include <XCAFDoc_Dimension.hxx>
#include <XCAFDoc_DimTol.hxx>
#include <XCAFDoc_DimTolTool.hxx>
#include <XCAFDoc_GeomTolerance.hxx>
#include <XCAFDimTolObjects_Tool.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <XCAFDimTolObjects_GeomToleranceObject.hxx>
#include <XCAFDimTolObjects_DatumObject.hxx>
#include <XCAFView_Object.hxx>

#include <StdPrs_ShadedShape.hxx>

#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepTools.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp_Face.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Builder.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <TopOpeBRepBuild_Tools.hxx>

#include <TDataStd_Name.hxx>
#include <Standard_NumericError.hxx>
#include <gp_Quaternion.hxx>

#include <XSControl_WorkSession.hxx>
#include <XSControl_TransferReader.hxx>
#include <Transfer_TransientProcess.hxx>
#include <Transfer_FinderProcess.hxx>
#include <TransferBRep.hxx>
#include <StepRepr_RepresentationItem.hxx>
#include <StepVisual_Colour.hxx>
#include <StepVisual_ColourRgb.hxx>

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>

#include <Prs3d.hxx>
#include <Prs3d_Drawer.hxx>

#include <OSD.hxx>