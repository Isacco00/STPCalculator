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

void InputOptions::SetEdge(const int& edge) {
	if (edge == 0)
		m_edge = false;
	else
		m_edge = true;
}

void InputOptions::SetSketch(const int& sketch) {
	if (sketch == 0)
		m_sketch = false;
	else
		m_sketch = true;
}

void InputOptions::SetHtml(const int& html) {
	if (html == 0)
		m_html = false;
	else
		m_html = true;
}

const wstring InputOptions::Output(void) const {
	wstring output = m_input.substr(0, m_input.find_last_of(L"."));

	if (m_html)
		output.append(L".html");
	else
		output.append(L".x3d");

	return output;
}

const wstring InputOptions::OutputJson(void) const {
	wstring output = m_output;
	output.append(L"\\calculatedStep.json");
	return output;
}