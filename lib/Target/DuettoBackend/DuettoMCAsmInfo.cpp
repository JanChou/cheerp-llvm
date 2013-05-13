//===-- DuettoBackend.cpp - MCAsmInfo for Duetto ---------------===//
//
//	Copyright 2013 Leaning Technlogies
//===----------------------------------------------------------------------===//

#include "DuettoTargetMachine.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/TargetRegistry.h"

namespace llvm {
  class Triple;

  class DuettoMCAsmInfo : public MCAsmInfo {
    virtual void anchor();
  public:
    explicit DuettoMCAsmInfo(const Triple &Triple);
  };

void DuettoMCAsmInfo::anchor() { }

DuettoMCAsmInfo::DuettoMCAsmInfo(const Triple &T) {

  // Debug Information
  SupportsDebugInformation = false;
}

static MCAsmInfo *createDuettoMCAsmInfo(const MCRegisterInfo &T, StringRef TT) {
  Triple TheTriple(TT);
  return new DuettoMCAsmInfo(TheTriple);
}

extern "C" void LLVMInitializeDuettoBackendTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn A(TheDuettoBackendTarget, createDuettoMCAsmInfo);
}

} // namespace llvm
