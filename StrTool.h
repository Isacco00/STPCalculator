#pragma once

class StrTool {
public:

	static const wstring ReplaceCharacter(wstring str, wstring from, wstring to) {
		size_t loc = str.find(from);

		while (loc != wstring::npos) {
			str.replace(loc, from.length(), to);
			loc = str.find(from, loc + to.length());
		}

		return str;
	}

	static const wstring str2wstr(string s) {
		wstring ws(s.begin(), s.end());
		return ws;
	}

	// Added by Cabiddu
	static const wstring s2ws(const std::string& s) {
		std::string curLocale = setlocale(LC_ALL, "");
		const char* _Source = s.c_str();
		size_t _Dsize = mbstowcs(NULL, _Source, 0) + 1;
		wchar_t* _Dest = new wchar_t[_Dsize];
		wmemset(_Dest, 0, _Dsize);
		mbstowcs(_Dest, _Source, _Dsize);
		std::wstring result = _Dest;
		delete[]_Dest;
		setlocale(LC_ALL, curLocale.c_str());
		return result;
	}

};
