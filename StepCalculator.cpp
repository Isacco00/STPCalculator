
#include "CommonImport.h"
#include "StepReader.h"
#include "Tessellator.h"
#include "X3D_Writer.h"
#include "JsonWriter.h"
#include "Component.h"
#include <fstream>
//-----------------------------------------------------------------------------

namespace fs = std::filesystem;
// Print out the usage
void PrintUsage(wstring exe, InputOptions* opt) {
	cout << endl;
	cout << "//////////////////////////////////////////////////" << endl;
	wcout << "//  STEP to JSON Translator (STP2JSON) " << "  //" << endl;
	cout << "//////////////////////////////////////////////////" << endl;
	cout << endl;
	cout << "[Usage]" << endl;
	wcout << " " << exe << " option1 value1 option2 value2.." << endl;
	cout << endl;
	cout << "[Options]" << endl;
	cout << " --input      Input STEP file path" << endl;
	cout << " --output     Output JSON path default=" << opt->GetOutputJson().c_str() << endl;
	cout << endl;
	cout << "[Examples]" << endl;
	wcout << " " << exe << " --input Model.stp --Output C:\\Desktop" << endl;
	cout << endl;
}

// Set option values
bool SetOption(int argc, char* argv[], InputOptions* opt) {
	// Print out usage
	if (argc < 2) {
		string a = argv[0];
		wstring aw = StrTool::s2ws(a);

		PrintUsage(aw, opt);
		//cout << "WRONG USAGE" << std::endl;
		return false;
	}
	bool inputFlag = false;

	// Set options
	for (int i = 1; i < argc; ++i) {
		string stoken(argv[i]);
		wstring token = StrTool::s2ws(stoken);

		string stoken1(argv[i + 1]);
		wstring token1 = StrTool::s2ws(stoken1);

		if (token != L"--input" && token != L"--output") {
			wcout << "No such option: " << token << endl;
			return false;
		} else {
			if (token == L"--input") {
				inputFlag = true;
				opt->SetInput(token1);
			} else if (token == L"--output") {
				opt->SetOutput(token1);
			}
			++i;
		}
	}

	// Check input path
	if (opt->GetInput().empty()) {
		cout << "Please input a STEP file." << endl;
		return false;
	} else if (opt->GetOutputDirectory().empty()) {
		cout << "Please input a OTUPUT directory." << endl;
		return false;
	} else if (!fs::is_directory(opt->GetInput())
			   && !fs::is_regular_file(opt->GetInput())) {
		wcout << "No such file or directory: " << opt->GetInput() << endl;
		return false;
	} else if (!fs::is_directory(opt->GetOutputDirectory())) {
		wcout << "No such directory: " << opt->GetOutputDirectory() << endl;
		return false;
	}

	return true;
}
int RunSTP2X3D(InputOptions* opt) {
	Model* model = new Model();

	StopWatch sw;
	sw.Start();

	/** START_STEP **/
	cout << "Reading a STEP file.." << endl;
	StepReader sr(opt);
	if (!sr.ReadSTEP(model)) {
		delete model;
		return -1;
	}
	/** END_STEP **/
	//sw.Lap();

	/** START_TESSELLATION **/
	cout << "Tessellating.." << endl;
	Tessellator* ts = new Tessellator(opt);
	ts->Tessellate(model);
	delete ts;
	/** END_TESSELLATION **/
	sw.Lap();
	
	///** START_JSON **/
	//cout << "Writing an Json file.." << endl;
	//JsonWriter jw(opt);
	//jw.WriteJson(model);
	///** END_JSON **/
	//sw.Lap();
	///** START_X3D **/
	//cout << "Writing an X3D file.." << endl;
	//X3D_Writer xw(opt);
	//xw.WriteX3D(model);
	///** END_X3D **/
	////sw.Lap();


	/// Print results required for SFA
	//if (opt->SFA()) {
	//	StatsPrinter::PrintShapeCount(model);
	//	StatsPrinter::PrintBoundingBox(model, opt);
	//	StatsPrinter::PrintSketchExistence(model);
	//}
	///

	cout << "STEP to JSON completed!" << endl;
	sw.End();

	delete model;

	return 0;
}

int main(int argc, char** argv) {
	InputOptions opt; // Option for STEP to X3D translator

	int status = -1; // Translation status

#if _DEBUG
	opt.SetInput(L"C:\\Users\\Isacco\\Desktop\\test.stp");
	opt.SetOutput(L"C:\\Users\\Isacco\\Desktop\\STPCalculator");
#else
	if (!SetOption(argc, argv, &opt))
		return status;
#endif

	status = RunSTP2X3D(&opt);
	return status;
}
