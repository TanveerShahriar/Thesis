#include "CallGraphAnalyzer.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CommandLine.h>

using namespace clang;
using namespace clang::ast_matchers;

static llvm::cl::OptionCategory ToolCategory("call-graph-analyzer options");

CallGraphAnalyzer::CallGraphAnalyzer() {}

const std::unordered_map<std::string, std::vector<std::string>> &CallGraphAnalyzer::getCallGraph() const {
    return CallGraph;
}

void CallGraphAnalyzer::dumpCallGraph() const {
    for (const auto &kv : CallGraph) {
        llvm::outs() << kv.first << " calls:\n";
        for (const auto &callee : kv.second) {
            llvm::outs() << "  " << callee << "\n";
        }
    }
}

CallGraphAnalyzer::MyCallHandler::MyCallHandler(std::unordered_map<std::string, std::vector<std::string>> &graph)
    : CallGraph(graph) {}

void CallGraphAnalyzer::MyCallHandler::run(const MatchFinder::MatchResult &Result) {
    const auto *Call = Result.Nodes.getNodeAs<CallExpr>("callSite");
    const auto *Caller = Result.Nodes.getNodeAs<FunctionDecl>("caller");
    const auto *Callee = Result.Nodes.getNodeAs<FunctionDecl>("callee");

    if (!Call || !Caller || !Callee)
        return;

    const SourceManager &SM = *Result.SourceManager;
    if (Callee->isImplicit() || SM.isInSystemHeader(Callee->getLocation()) || !Callee->isUserProvided()) {
        return;
    }

    std::string callerName = Caller->getQualifiedNameAsString();
    std::string calleeName = Callee->getQualifiedNameAsString();
    CallGraph[callerName].push_back(calleeName);
}

int CallGraphAnalyzer::runAnalysis(int argc, const char **argv) {
    auto ExpectedParser =
        clang::tooling::CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << "Error creating CommonOptionsParser: "
                     << llvm::toString(ExpectedParser.takeError()) << "\n";
        return 1;
    }

    clang::tooling::CommonOptionsParser &OptionsParser = ExpectedParser.get();

    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                   OptionsParser.getSourcePathList());

    MyCallHandler Handler(CallGraph);
    MatchFinder Finder;

    Finder.addMatcher(
        callExpr(
            callee(functionDecl().bind("callee")),
            hasAncestor(functionDecl(
                isDefinition(),
                unless(isImplicit()),
                unless(isExpansionInSystemHeader())
            ).bind("caller"))
        ).bind("callSite"),
        &Handler
    );

    return Tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());
}