#pragma once

class Model;

class StepReader {
public:
	StepReader(InputOptions* opt);
	~StepReader(void);

	bool ReadSTEP(Model* model);

protected:
	bool CheckReturnStatus(const IFSelect_ReturnStatus& status) const;

private:
	InputOptions* m_opt;
};
