#include "CommonImport.h"
#include "StepReader.h"
#include "Component.h"
#include "IShape.h"

StepReader::StepReader(InputOptions* opt)
	: m_opt(opt) {}

StepReader::~StepReader(void) {}

bool StepReader::ReadSTEP(Model* model) {
	IFSelect_ReturnStatus status;
	wstring filePath = m_opt->Input();
	OSD::SetSignal(false);
	try {
		model->Clear();
		// Read a STEP file
		STEPControl_Reader reader;
		TCollection_AsciiString aFileName((const wchar_t*)filePath.c_str());
		status = reader.ReadFile(aFileName.ToCString());
		//status = reader.ReadFile(filePath.c_str());
		if (!CheckReturnStatus(status)) {
			return false;
		}
		if (reader.TransferRoot()) {
			const TopoDS_Shape& shape = reader.Shape();
			IShape* iShape = new IShape(shape);
			Component* rootComp = new Component(shape);
			rootComp->AddIShape(iShape);
			model->AddRootComponent(rootComp);
		}
	} catch (...) {
		// Unknown failure
		return CheckReturnStatus(IFSelect_RetFail);
	}
	model->Update();
	// Terminate if the root comp is empty
	if (model->IsEmpty()) {
		return CheckReturnStatus(IFSelect_RetVoid);
	}
	return true;
}

bool StepReader::CheckReturnStatus(const IFSelect_ReturnStatus& status) const {
	bool isDone = false;
	if (status == IFSelect_RetDone) {
		isDone = true;
	} else {
		switch (status) {
		case IFSelect_RetError:
			printf("Not a valid STEP file.\n");
			break;
		case IFSelect_RetFail:
			printf("Reading has failed.\n");
			break;
		case IFSelect_RetVoid:
			printf("Nothing to translate.\n");
			break;
		case IFSelect_RetStop:
			printf("Reading has stopped.\n");
			break;
		}
	}
	return isDone;
}