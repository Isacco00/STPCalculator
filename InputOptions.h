#pragma once

class InputOptions {
public:
	InputOptions();
	~InputOptions();

	void SetInput(const wstring& input) { m_input = input; }
	void SetOutput(const wstring& output) { m_output = output; }
	void SetEdge(const int& edge);
	void SetSketch(const int& sketch);
	void SetHtml(const int& html);
	void SetQuality(const double& quality) { m_quality = quality; }
	void SetSFA(bool sfa) { m_SFA = sfa; }

	const wstring& Input(void) const { return m_input; }
	const wstring Output(void) const;
	const wstring OutputJson(void) const;
	wstring OutputDirectory(void) { return m_output; }
	bool Edge(void) const { return m_edge; }
	bool Sketch(void) const { return m_sketch; }
	bool Html(void) const { return m_html; }
	double Quality(void) const { return m_quality; }
	bool SFA(void) const { return m_SFA; }

	// Software version (as of Feb 2022)
	const wstring Version(void) const { return L"1.21"; }

private:
	wstring m_input;	// Input file path
	wstring m_output;	// Output file path
	bool m_edge;		// Boundary edge
	bool m_sketch;		// Sketch geometry
	bool m_html;		// Output file type, html or x3d
	double m_quality;	// Mesh quality
	bool m_SFA;			// Specific to SFA
};