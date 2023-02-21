
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
	wcout << "//  STEP to JSON Translator (STP2X3D) " << "  //" << endl;
	cout << "//////////////////////////////////////////////////" << endl;
	cout << endl;
	cout << "[Usage]" << endl;
	wcout << " " << exe << " option1 value1 option2 value2.." << endl;
	cout << endl;
	cout << "[Options]" << endl;
	cout << " --input      Input STEP file path" << endl;
	cout << " --edge       Boundary edges (1:yes, 0:no) default=" << opt->Edge() << endl;
	cout << " --sketch     Sketch geometry (1:yes, 0:no) default=" << opt->Sketch() << endl;
	cout << " --html       Output file type (1:html, 0:x3d) default=" << opt->Html() << endl;
	cout << " --quality    Mesh quality (1-low to 10-high) default=" << opt->Quality() << endl;
	cout << endl;
	cout << "[Examples]" << endl;
	wcout << " " << exe << " --input Model.stp --edge 1 --quality 7" << endl;
	wcout << " " << exe << " --html 1 --sketch 0 --input Model.step" << endl;
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
	bool batchFlag = false;

	// Set options
	for (int i = 1; i < argc; ++i) {
		string stoken(argv[i]);
		wstring token = StrTool::s2ws(stoken);

		string stoken1(argv[i + 1]);
		wstring token1 = StrTool::s2ws(stoken1);

		if (token != L"--input"
			&& token != L"--output"
			&& token != L"--edge"
			&& token != L"--sketch"
			&& token != L"--html"
			&& token != L"--quality"
			&& token != L"--sfa") {
			wcout << "No such option: " << token << endl;
			return false;
		} else {
			if (token == L"--input") {
				inputFlag = true;
				opt->SetInput(token1);
			} else if (token == L"--output") {
				opt->SetOutput(token1);
			} else if (token == L"--edge") {
				int edge = stoi(token1);
				opt->SetEdge(edge);

				if (edge != 0
					&& edge != 1) {
					cout << "edge must be either 0 or 1." << endl;
					return false;
				}
			} else if (token == L"--sketch") {
				int sketch = stoi(token1);
				opt->SetSketch(sketch);

				if (sketch != 0
					&& sketch != 1) {
					cout << "sketch must be either 0 or 1." << endl;
					return false;
				}
			} else if (token == L"--html") {
				int html = stoi(token1);
				opt->SetHtml(html);

				if (html != 0
					&& html != 1) {
					cout << "html must be either 0 or 1." << endl;
					return false;
				}
			} else if (token == L"--sfa") {
				int sfa = stoi(token1);
				opt->SetSFA(sfa);

				if (sfa != 0
					&& sfa != 1) {
					cout << "sfa must be either 0 or 1." << endl;
					return false;
				}
			} else if (token == L"--quality") {
				double quality = stof(token1);
				opt->SetQuality(quality);

				if (quality < 1.0
					|| quality > 10.0) {
					cout << "quality must be between 1 and 10." << endl;
					return false;
				}
			}

			++i;
		}
	}

	// Check input path
	if (opt->Input().empty()) {
		cout << "Please input a STEP file." << endl;
		return false;
	} else if (opt->OutputDirectory().empty()) {
		cout << "Please input a OTUPUT directory." << endl;
		return false;
	} else if (!fs::is_directory(opt->Input())
			   && !fs::is_regular_file(opt->Input())) {
		wcout << "No such file or directory: " << opt->Input() << endl;
		return false;
	} else if (!fs::is_directory(opt->OutputDirectory())) {
		wcout << "No such directory: " << opt->OutputDirectory() << endl;
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
	
	/** START_JSON **/
	cout << "Writing an Json file.." << endl;
	JsonWriter jw(opt);
	jw.WriteJson(model);
	/** END_JSON **/
	sw.Lap();
	/** START_X3D **/
	cout << "Writing an X3D file.." << endl;
	X3D_Writer xw(opt);
	xw.WriteX3D(model);
	/** END_X3D **/
	//sw.Lap();
	
	
	/// Print results required for SFA
	//if (opt->SFA()) {
	//	StatsPrinter::PrintShapeCount(model);
	//	StatsPrinter::PrintBoundingBox(model, opt);
	//	StatsPrinter::PrintSketchExistence(model);
	//}
	///
	
	cout << "STEP to X3D completed!" << endl;
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
