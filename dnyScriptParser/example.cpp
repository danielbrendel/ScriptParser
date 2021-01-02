#include "dnyScriptParser.h"

//dnyScriptParser example code
//============================

void dnyStandardOutputHandler(const std::wstring& wszOutputText)
{
	//Standard output handler function

	std::wcout << wszOutputText << std::endl;
}

//Example datatype handlers
bool DT_MyDataType_DeclareVar(const std::wstring& wszName, dnyScriptParser::CVarManager::ICVar<dnyScriptParser::dnycustom>* pCVar)
{
	std::wcout << "DT_MyDataType_DeclareVar: " << wszName << L"|0x" << pCVar << std::endl;
	return true;
}

bool DT_MyDataType_AssignVarValue(const std::wstring& wszName, dnyScriptParser::CVarManager::ICVar<dnyScriptParser::dnycustom>* pCVar, const dnyScriptParser::CVarManager::ICustomVarValue& rCustomVarValue, bool bIsConst)
{
	std::wcout << "DT_MyDataType_AssignVarValue: " << wszName << L"|0x" << pCVar << L"|" << rCustomVarValue.QueryAsDnyString() << L"|" << bIsConst << std::endl;
	pCVar->AcquireCustomSpace(rCustomVarValue.QueryAsDnyString().length() * 2 + 2); //Only once on declaration or every time a value is assigned
	wcscpy_s((wchar_t*)pCVar->CustomData(), rCustomVarValue.QueryAsDnyString().length() * 2, rCustomVarValue.QueryAsDnyString().c_str());
	return true;
}

dnyScriptParser::dnyString DT_MyDataType_GetReplacerString(const std::wstring& wszName, dnyScriptParser::CVarManager::ICVar<dnyScriptParser::dnycustom>* pCVar)
{
	std::wcout << "DT_MyDataType_GetReplacerString: " << wszName << L"|0x" << pCVar << std::endl;
	return dnyScriptParser::dnyString((wchar_t*)pCVar->CustomData());
}

void DT_MyDataType_RemoveVar(const std::wstring& wszName, dnyScriptParser::CVarManager::ICVar<dnyScriptParser::dnycustom>* pCVar)
{
	//Base memory cleanup is done by the CVar handler object
	std::wcout << "DT_MyDataType_RemoveVar: " << wszName << L"|0x" << pCVar << std::endl;
}

dnyScriptParser::CVarManager::custom_cvar_type_s::cvar_type_event_table_s g_oMyDataTypeCVarEventTable = {
	&DT_MyDataType_DeclareVar, &DT_MyDataType_AssignVarValue, &DT_MyDataType_GetReplacerString, &DT_MyDataType_RemoveVar
};

//Float-returning command handler class
class IMyFloatCommandInterface : public dnyScriptParser::CCommandManager::IResultCommandInterface<dnyScriptParser::dnyFloat> {
public:
	virtual bool CommandCallback(void* pCodeContext, void* pInterfaceObject)
	{
		dnyScriptParser::ICodeContext* pContext = (dnyScriptParser::ICodeContext*)pCodeContext;

		pContext->ReplaceAllVariables(pInterfaceObject);

		dnyScriptParser::dnyFloat flValue = pContext->GetPartFloat(1);
		this->SetResult(flValue * 2);

		return true;
	}
} oTestCommandValue;

//Void command handler class
class IMyVoidCommandInterface : public dnyScriptParser::CCommandManager::IVoidCommandInterface {
public:
	virtual bool CommandCallback(void* pCodeContext, void* pInterfaceObject)
	{
		dnyScriptParser::ICodeContext* pContext = (dnyScriptParser::ICodeContext*)pCodeContext;

		pContext->ReplaceAllVariables(pInterfaceObject);

		std::wcout << pContext->GetPartString(1) << std::endl;

		return true;
	}
} oTestCommandVoid;

int main(int argc, char* argv[])
{
	//Print about information
	std::wcout << dnyScriptParser::dnyProduct() << L" v" << dnyScriptParser::dnyVersion() << L" developed by " << dnyScriptParser::dnyAuthor() << L" (" << dnyScriptParser::dnycontact() << L")" << std::endl;

	//As user you have to instantiate a CScriptingInterface object
	dnyScriptParser::CScriptingInterface oScriptingInterface;

	//You can use a base directory or leave it blank to use absolute paths
	oScriptingInterface.SetScriptBaseDirectory(L"");
	//Set standard output handler function if needed
	oScriptingInterface.SetStandardOutput(&dnyStandardOutputHandler);

	//Example data type registrations
	oScriptingInterface.RegisterDataType(L"MyDataType", &g_oMyDataTypeCVarEventTable);
	
	//Example command registrations
	oScriptingInterface.RegisterCommand(L"mycommand2", &oTestCommandValue, dnyScriptParser::CVarManager::CT_FLOAT); //Float-returning command
	oScriptingInterface.RegisterCommand(L"mycommand", &oTestCommandVoid, dnyScriptParser::CVarManager::CT_VOID); //Void command

	//Execute script file
	bool bResult = oScriptingInterface.ExecuteScript(L"scriptdemo.dnys");

	//Handle operation status
	if (dnyScriptParser::GetErrorInformation().GetErrorCode() != dnyScriptParser::SET_NO_ERROR) {
		std::wcout << L"Error during script parsing:" << std::endl;
		std::wcout << dnyScriptParser::GetErrorInformation().GetErrorCode() << std::endl;
		std::wcout << dnyScriptParser::GetErrorInformation().GetErrorText() << std::endl;
	}

	//Call a function from the previously executed script file
	if (oScriptingInterface.BeginFunctionCall(L"callme", dnyScriptParser::CVarManager::CT_INT)) {
		oScriptingInterface.PushFunctionParam((dnyScriptParser::dnyInteger)2020);

		oScriptingInterface.ExecuteFunction();

		std::wcout << oScriptingInterface.QueryFunctionResultAsInteger() << std::endl;
		
		oScriptingInterface.EndFunctionCall();
	}

	std::wcout << L"Result: " << bResult << std::endl;

	//Handle operation status
	if (dnyScriptParser::GetErrorInformation().GetErrorCode() != dnyScriptParser::SET_NO_ERROR) {
		std::wcout << L"Error during function call:" << std::endl;
		std::wcout << dnyScriptParser::GetErrorInformation().GetErrorCode() << std::endl;
		std::wcout << dnyScriptParser::GetErrorInformation().GetErrorText() << std::endl;
	}

	system("pause");

	return 0;
}