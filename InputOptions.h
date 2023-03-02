#pragma once

class InputOptions {
public:
	InputOptions();
	~InputOptions();

	void SetInput(const wstring& input) { m_input = input; }
	void SetOutput(const wstring& output) { m_output = output; }

	const wstring& GetInput(void) const { return m_input; }
	const wstring GetOutput(void) const;
	const wstring GetOutputJson(void) const;
	wstring GetOutputDirectory(void) { return m_output; }
	bool GetEdge(void) const { return m_edge; }
	bool GetSketch(void) const { return m_sketch; }
	bool GetHtml(void) const { return m_html; }
	double GetQuality(void) const { return m_quality; }
	bool GetSFA(void) const { return m_SFA; }

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