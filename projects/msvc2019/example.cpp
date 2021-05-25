#include <dnyScriptInterpreter.h>

//dnyScriptInterpreter example code
//=================================

void dnyStandardOutputHandler(const std::wstring& wszOutputText)
{
	//Standard output handler function

	std::wcout << wszOutputText << std::endl;
}

//Example datatype handlers
bool DT_MyDataType_DeclareVar(const std::wstring& wszName, dnyScriptInterpreter::CVarManager::ICVar<dnyScriptInterpreter::dnyCustom>* pCVar)
{
	std::wcout << "DT_MyDataType_DeclareVar: " << wszName << L"|0x" << pCVar << std::endl;
	return true;
}

bool DT_MyDataType_AssignVarValue(const std::wstring& wszName, dnyScriptInterpreter::CVarManager::ICVar<dnyScriptInterpreter::dnyCustom>* pCVar, const dnyScriptInterpreter::CVarManager::ICustomVarValue& rCustomVarValue, bool bIsConst)
{
	std::wcout << "DT_MyDataType_AssignVarValue: " << wszName << L"|0x" << pCVar << L"|" << rCustomVarValue.QueryAsDnyString() << L"|" << bIsConst << std::endl;
	pCVar->AcquireCustomSpace(rCustomVarValue.QueryAsDnyString().length() * 2 + 2); //Only once on declaration or every time a value is assigned
	wcscpy_s((wchar_t*)pCVar->CustomData(), rCustomVarValue.QueryAsDnyString().length() * 2, rCustomVarValue.QueryAsDnyString().c_str());
	return true;
}

dnyScriptInterpreter::dnyString DT_MyDataType_GetReplacerString(const std::wstring& wszName, dnyScriptInterpreter::CVarManager::ICVar<dnyScriptInterpreter::dnyCustom>* pCVar)
{
	std::wcout << "DT_MyDataType_GetReplacerString: " << wszName << L"|0x" << pCVar << std::endl;
	return dnyScriptInterpreter::dnyString((wchar_t*)pCVar->CustomData());
}

void DT_MyDataType_RemoveVar(const std::wstring& wszName, dnyScriptInterpreter::CVarManager::ICVar<dnyScriptInterpreter::dnyCustom>* pCVar)
{
	//Base memory cleanup is done by the CVar handler object
	std::wcout << "DT_MyDataType_RemoveVar: " << wszName << L"|0x" << pCVar << std::endl;
}

dnyScriptInterpreter::CVarManager::custom_cvar_type_s::cvar_type_event_table_s g_oMyDataTypeCVarEventTable = {
	&DT_MyDataType_DeclareVar, &DT_MyDataType_AssignVarValue, &DT_MyDataType_GetReplacerString, &DT_MyDataType_RemoveVar
};

//Float-returning command handler class
class IMyFloatCommandInterface : public dnyScriptInterpreter::CCommandManager::IResultCommandInterface<dnyScriptInterpreter::dnyFloat> {
public:
	virtual bool CommandCallback(void* pCodeContext, void* pInterfaceObject)
	{
		dnyScriptInterpreter::ICodeContext* pContext = (dnyScriptInterpreter::ICodeContext*)pCodeContext;

		pContext->ReplaceAllVariables(pInterfaceObject);

		dnyScriptInterpreter::dnyFloat flValue = pContext->GetPartFloat(1);
		this->SetResult(flValue * 2);

		return true;
	}
} oTestCommandValue;

//Void command handler class
class IMyVoidCommandInterface : public dnyScriptInterpreter::CCommandManager::IVoidCommandInterface {
public:
	virtual bool CommandCallback(void* pCodeContext, void* pInterfaceObject)
	{
		dnyScriptInterpreter::ICodeContext* pContext = (dnyScriptInterpreter::ICodeContext*)pCodeContext;

		pContext->ReplaceAllVariables(pInterfaceObject);

		std::wcout << pContext->GetPartString(1) << std::endl;

		return true;
	}
} oTestCommandVoid;

int main(int argc, char* argv[])
{
	//Print about information
	std::wcout << dnyScriptInterpreter::dnyProduct() << L" v" << dnyScriptInterpreter::dnyVersion() << L" developed by " << dnyScriptInterpreter::dnyAuthor() << L" (" << dnyScriptInterpreter::dnycontact() << L")" << std::endl;

	//As user you have to instantiate a CScriptingInterface object
	dnyScriptInterpreter::CScriptingInterface oScriptingInterface;

	//You can use a base directory or leave it blank to use absolute paths
	oScriptingInterface.SetScriptBaseDirectory(L"");
	//Set standard output handler function if needed
	oScriptingInterface.SetStandardOutput(&dnyStandardOutputHandler);

	//Example data type registrations
	oScriptingInterface.RegisterDataType(L"MyDataType", &g_oMyDataTypeCVarEventTable);
	
	//Example command registrations
	oScriptingInterface.RegisterCommand(L"mycommand2", &oTestCommandValue, dnyScriptInterpreter::CVarManager::CT_FLOAT); //Float-returning command
	oScriptingInterface.RegisterCommand(L"mycommand", &oTestCommandVoid, dnyScriptInterpreter::CVarManager::CT_VOID); //Void command

	//Execute script file
	bool bResult = oScriptingInterface.ExecuteScript(L"example.dnys");

	//Handle operation status
	if (dnyScriptInterpreter::GetErrorInformation().GetErrorCode() != dnyScriptInterpreter::SET_NO_ERROR) {
		std::wcout << L"Error during script parsing:" << std::endl;
		std::wcout << dnyScriptInterpreter::GetErrorInformation().GetErrorCode() << std::endl;
		std::wcout << dnyScriptInterpreter::GetErrorInformation().GetErrorText() << std::endl;
	}

	//Call a function from the previously executed script file
	if (oScriptingInterface.BeginFunctionCall(L"callme", dnyScriptInterpreter::CVarManager::CT_INT)) {
		oScriptingInterface.PushFunctionParam((dnyScriptInterpreter::dnyInteger)2020);

		oScriptingInterface.ExecuteFunction();

		std::wcout << oScriptingInterface.QueryFunctionResultAsInteger() << std::endl;
		
		oScriptingInterface.EndFunctionCall();
	}

	std::wcout << L"Result: " << bResult << std::endl;

	//Handle operation status
	if (dnyScriptInterpreter::GetErrorInformation().GetErrorCode() != dnyScriptInterpreter::SET_NO_ERROR) {
		std::wcout << L"Error during function call:" << std::endl;
		std::wcout << dnyScriptInterpreter::GetErrorInformation().GetErrorCode() << std::endl;
		std::wcout << dnyScriptInterpreter::GetErrorInformation().GetErrorText() << std::endl;
	}

	system("pause");

	return 0;
}