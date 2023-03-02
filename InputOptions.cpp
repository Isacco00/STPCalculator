#include "CommonImport.h"
#include "InputOptions.h"

using namespace std;

InputOptions::InputOptions()
	: m_input(L""),
	m_output(L""),
	m_edge(true),
	m_sketch(true),
	m_html(true),
	m_quality(10.0),
	m_SFA(true) {}

InputOptions::~InputOptions() {}

const wstring InputOptions::GetOutput(void) const {
	wstring output = m_input.substr(0, m_input.find_last_of(L"."));

	if (m_html)
		output.append(L".html");
	else
		output.append(L".x3d");

	return output;
}

const wstring InputOptions::GetOutputJson(void) const {
	wstring output = m_output;
	output.append(L"\\calculatedStep.json");
	return output;
}