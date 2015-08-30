/*
 * accessor_comp.cpp
 *
 *  Created on: 13 juil. 2015
 *      Author: gorinje
 */

#include "llvm/ADT/Triple.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Host.h"

#include "accessor_comp.h"

using namespace llvm;
using namespace std;

typedef struct Acc_Comp_s
{
	Module* Mod;
	ExecutionEngine *EE;
}Acc_Comp;

//===----------------------------------------------------------------------===//
// Object cache
//
// This object cache implementation writes cached objects to disk to the
// directory specified by CacheDir, using a filename provided in the module
// descriptor. The cache tries to load a saved object using that path if the
// file exists. CacheDir defaults to "", in which case objects are cached
// alongside their originating bitcodes.
//
class LLIObjectCache : public ObjectCache {
public:
	LLIObjectCache(const std::string& CacheDir) : CacheDir(CacheDir) {
		// Add trailing '/' to cache dir if necessary.
		if (!this->CacheDir.empty() &&
			this->CacheDir[this->CacheDir.size() - 1] != '/')
			this->CacheDir += '/';
	}
	virtual ~LLIObjectCache() {}

	void notifyObjectCompiled(const Module *M, MemoryBufferRef Obj) override {
		const std::string ModuleID = M->getModuleIdentifier();
		std::string CacheName;
		if (!getCacheFilename(ModuleID, CacheName))
			return;
		if (!CacheDir.empty()) { // Create user-defined cache dir.
			SmallString<128> dir(CacheName);
			sys::path::remove_filename(dir);
			sys::fs::create_directories(Twine(dir));
		}
		std::error_code EC;
		raw_fd_ostream outfile(CacheName, EC, sys::fs::F_None);
		outfile.write(Obj.getBufferStart(), Obj.getBufferSize());
		outfile.close();
	}

	std::unique_ptr<MemoryBuffer> getObject(const Module* M) override {
		const std::string ModuleID = M->getModuleIdentifier();
		std::string CacheName;
		if (!getCacheFilename(ModuleID, CacheName))
			return nullptr;
		// Load the object from the cache filename
		ErrorOr<std::unique_ptr<MemoryBuffer>> IRObjectBuffer =
			MemoryBuffer::getFile(CacheName.c_str(), -1, false);
		// If the file isn't there, that's OK.
		if (!IRObjectBuffer)
			return nullptr;
		// MCJIT will want to write into this buffer, and we don't want that
		// because the file has probably just been mmapped.  Instead we make
		// a copy.  The filed-based buffer will be released when it goes
		// out of scope.
		return MemoryBuffer::getMemBufferCopy(IRObjectBuffer.get()->getBuffer());
	}

private:
	std::string CacheDir;

	bool getCacheFilename(const std::string &ModID, std::string &CacheName) {
		std::string Prefix("file:");
		size_t PrefixLength = Prefix.length();
		if (ModID.substr(0, PrefixLength) != Prefix)
			return false;
		std::string CacheSubdir = ModID.substr(PrefixLength);
#if defined(_WIN32)
		// Transform "X:\foo" => "/X\foo" for convenience.
		if (isalpha(CacheSubdir[0]) && CacheSubdir[1] == ':') {
			CacheSubdir[1] = CacheSubdir[0];
			CacheSubdir[0] = '/';
		}
#endif
		CacheName = CacheDir + CacheSubdir+ ".o";
		//size_t pos = CacheName.rfind('.');
		//CacheName.replace(pos, CacheName.length() - pos, ".o");
		return true;
	}
};


static LLIObjectCache *CacheManager = nullptr;

MDString* getFirstMDString(Module* mod, const char* attribute){
	NamedMDNode* ND = mod->getNamedMetadata(attribute);
	if (ND) {
		MDNode* node = ND->getOperand(0);
		if (node)
			return cast<MDString>(node->getOperand(0));
	}

	return NULL;
}

Function* getFirstMDFunction(Module* mod, const char* attribute){
	NamedMDNode* ND = mod->getNamedMetadata(attribute);
	if (ND) {
		MDNode* node = ND->getOperand(0);
		if (node){
			Value* metavalue = cast<ValueAsMetadata>(node->getOperand(0))->getValue();
			if (isa<Function>(metavalue)) return cast<Function>(metavalue);
		}
	}

	return NULL;
}

void* getFn(Acc_Comp_s* comp, const char * name){
	Function* fn = NULL;

	if (comp && comp->EE)
		fn = getFirstMDFunction(comp->Mod, name);

	if (!fn) return NULL;
	return comp->EE->getPointerToFunction(fn);
}

int compile(Acc_Comp_s** comp, const char* llvm_code, unsigned int length, const char* cachedir){
	bool enableCacheManager = false;
	LLVMContext &Context = getGlobalContext();
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();

	// Create compiler
	Acc_Comp_s* tmp_comp;
	Module* Mod;

	// Parse code
	SMDiagnostic Err;
	unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(StringRef(llvm_code, length));
	unique_ptr<Module> Owner = parseIR(Buf.get()->getMemBufferRef(), Err, Context);

	if (!Owner.get()) {
		Err.print("Bevara module : ", errs());

		//Parsing error
		return 1;
	}

	Mod = Owner.get();


	//Prepare cache name
	string CacheName("file:");
	MDString* libname = getFirstMDString(Mod, "LIBNAME");
	if (libname && cachedir){
		MDString* libversion = getFirstMDString(Mod, "LIBVERSION");
		enableCacheManager = true;
		CacheName.append(libname->getString());
		if (libversion){
			CacheName.append("-");
			CacheName.append(libname->getString());
		}
		Mod->setModuleIdentifier(CacheName);
	}

	// Build engine
	std::string ErrorMsg;
	EngineBuilder builder(std::move(Owner));
	builder.setErrorStr(&ErrorMsg);
	builder.setOptLevel(CodeGenOpt::Aggressive);

	string TargetTriple = sys::getProcessTriple();
#ifdef WIN32
	TargetTriple.append("-elf");
	// Temporary LLVM fix for windows
#endif
	Mod->setTargetTriple(Triple::normalize(TargetTriple));

	// Enable MCJIT if desired.
	RTDyldMemoryManager *RTDyldMM = nullptr;
	RTDyldMM = new SectionMemoryManager();

	// Deliberately construct a temp std::unique_ptr to pass in. Do not null out
	// RTDyldMM: We still use it below, even though we don't own it.
	builder.setMCJITMemoryManager(
			std::unique_ptr<RTDyldMemoryManager>(RTDyldMM));

	ExecutionEngine* EE = builder.create();
	if (!EE) {
		if (!ErrorMsg.empty())
			errs() << "Accessor compilation error : error creating EE: " << ErrorMsg << "\n";
		else
			errs() << "Accessor compilation error : unknown error creating EE!\n";
		return 1;
	}

	if (enableCacheManager) {
		CacheManager = new LLIObjectCache(cachedir);
		EE->setObjectCache(CacheManager);
	}

	// The following functions have no effect if their respective profiling
	// support wasn't enabled in the build configuration.
	EE->RegisterJITEventListener(
		JITEventListener::createOProfileJITEventListener());
	EE->RegisterJITEventListener(
		JITEventListener::createIntelJITEventListener());

	// Give MCJIT a chance to apply relocations and set page permissions.
	EE->finalizeObject();
	EE->runStaticConstructorsDestructors(false);



	tmp_comp = new Acc_Comp_s();
	tmp_comp->Mod = Mod;
	tmp_comp->EE = EE;
	*comp = tmp_comp;

	return 0;
}




