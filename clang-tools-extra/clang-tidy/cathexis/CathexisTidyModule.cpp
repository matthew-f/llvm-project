//===------- CathexisTidyModule.cpp - clang-tidy
//----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "AbseilWrappersCheck.h"
#include "BannedCFunctionsCheck.h"
#include "BugproneBufDataCheck.h"
#include "NamingConventionsCheck.h"
#include "PackedStructMissingSizeCheckCheck.h"
#include "ProtectedClassMemberCheck.h"
#include "QtNoDateTimeToStringCheck.h"
#include "QtNoQfiledialogStaticMethodsCheck.h"
#include "QtNolatin1Check.h"
#include "QtTrWithoutQobjectCheck.h"
#include "QualityConstCastCheck.h"
#include "QualityContainersCheck.h"
#include "QualityDeprecatedCheck.h"
#include "QualityEnumCheck.h"
#include "QualityLoggingCheck.h"
#include "QualityNoNakedNewCheck.h"
#include "QualityPackedStructuresCheck.h"
#include "QualityQtAvoidQdialogExecCheck.h"
#include "QualityReviewCheck.h"
#include "QualityUseGbufviewCheck.h"
#include "QualityUseNodiscardCheck.h"
#include "QualityXmlSuspiciousFindoraddCheck.h"
#include "RefactorXmlConvertFindoraddToUpdateCheck.h"
#include "RefactorXmlRenameUpdateCheck.h"
#include "RefactorXmlUseFindoraddCheck.h"
//#include "StringFindStartswithCheck.h"
#include "QualityEnumToStringCheck.h"
#include "UseAnonymousDefCheck.h"
#include "UseNodiscardCheck.h"
#include "WrapperCheck.h"

namespace clang {
namespace tidy {
namespace cathexis {

class CathexisModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<AbseilWrappersCheck>(
        "cathexis-abseil-wrappers");
    CheckFactories.registerCheck<BannedCFunctionsCheck>(
        "cathexis-banned-c-functions");
    CheckFactories.registerCheck<BugproneBufDataCheck>(
        "cathexis-bugprone-buf-data");
    CheckFactories.registerCheck<NamingConventionsCheck>(
        "cathexis-naming-conventions");
    CheckFactories.registerCheck<PackedStructMissingSizeCheckCheck>(
        "cathexis-packed-struct-missing-size-check");
    CheckFactories.registerCheck<ProtectedClassMemberCheck>(
        "cathexis-protected-class-member");
    CheckFactories.registerCheck<QtNoDateTimeToStringCheck>(
        "cathexis-qt-no-date-time-to-string");
    CheckFactories.registerCheck<QtNolatin1Check>("cathexis-qt-nolatin1");
    CheckFactories.registerCheck<QtNoQfiledialogStaticMethodsCheck>(
        "cathexis-qt-no-qfiledialog-static-methods");
    CheckFactories.registerCheck<QtTrWithoutQobjectCheck>(
        "cathexis-qt-tr-without-qobject");
    CheckFactories.registerCheck<QualityConstCastCheck>(
        "cathexis-quality-const-cast");
    CheckFactories.registerCheck<QualityContainersCheck>(
        "cathexis-quality-containers");
    CheckFactories.registerCheck<QualityDeprecatedCheck>(
        "cathexis-quality-deprecated");
    CheckFactories.registerCheck<QualityEnumCheck>("cathexis-quality-enum");
    CheckFactories.registerCheck<QualityLoggingCheck>(
        "cathexis-quality-logging");
    CheckFactories.registerCheck<QualityNoNakedNewCheck>(
        "cathexis-quality-no-naked-new");
    CheckFactories.registerCheck<QualityPackedStructuresCheck>(
        "cathexis-quality-packed-structures");
    CheckFactories.registerCheck<QualityQtAvoidQdialogExecCheck>(
        "cathexis-quality-qt-avoid-qdialog-exec");
    CheckFactories.registerCheck<QualityReviewCheck>("cathexis-quality-review");
    CheckFactories.registerCheck<QualityUseGbufviewCheck>(
        "cathexis-quality-use-gbufview");
    CheckFactories.registerCheck<QualityUseNodiscardCheck>(
        "cathexis-quality-use-nodiscard");
    CheckFactories.registerCheck<QualityXmlSuspiciousFindoraddCheck>(
        "cathexis-quality-xml-suspicious-findoradd");
    CheckFactories.registerCheck<RefactorXmlConvertFindoraddToUpdateCheck>(
        "cathexis-refactor-xml-convert-findoradd-to-update");
    CheckFactories.registerCheck<RefactorXmlRenameUpdateCheck>(
        "cathexis-refactor-xml-rename-update");
    CheckFactories.registerCheck<RefactorXmlUseFindoraddCheck>(
        "cathexis-refactor-xml-use-findoradd");
    //    CheckFactories.registerCheck<StringFindStartswithCheck>(
    //   "cathexis-string-find-startswith");
    CheckFactories.registerCheck<UseAnonymousDefCheck>(
        "cathexis-use-anonymous-def");
    CheckFactories.registerCheck<UseNodiscardCheck>("cathexis-use-nodiscard");
    CheckFactories.registerCheck<WrapperCheck>("cathexis-wrapper");
    CheckFactories.registerCheck<QualityEnumToStringCheck>(
        "cathexis-quality-enum-to-string");
  }
};

// Register the CathexisModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<CathexisModule> X("cathexis-module",
                                                      "Add Cathexis checks.");

} // namespace cathexis

// This anchor is used to force the linker to link in the generated object file
// and thus register the CathexisModule.
volatile int CathexisModuleAnchorSource = 0;

} // namespace tidy
} // namespace clang
