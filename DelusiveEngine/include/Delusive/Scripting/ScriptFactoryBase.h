#pragma once
#include <string>
#include <memory>

//Scripts are linked statically for now - the DLL export path was removed for portability

class BehaviourScript;
class UIScript;

class ScriptFactoryBase {
public:
	virtual ~ScriptFactoryBase() = default;

	virtual std::unique_ptr<BehaviourScript> CreateBehaviour(const std::string&) = 0;
	virtual std::unique_ptr<UIScript> CreateUIScript(const std::string& name) = 0;

	virtual const char** ListBehaviourScripts(size_t& count) = 0;
	virtual const char** ListUIScripts(size_t& count) = 0;
};