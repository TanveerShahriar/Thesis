#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/Support/CommandLine.h>

#include "FunctionCollector.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace fs = std::filesystem;

const std::set<std::string>& functions = FunctionCollector::getInstance().getCollectedFunctions();

class FunctionRewriter : public MatchFinder::MatchCallback, public RecursiveASTVisitor<FunctionRewriter> {
public:
    FunctionRewriter(Rewriter &R) : TheRewriter(R), CurrentFunction(nullptr) {}

    void run(const MatchFinder::MatchResult &Result) override {
        if (const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
            if (functions.find(Func->getNameAsString()) == functions.end())
                return;
            
            bool isMain = (Func->getNameAsString() == "main");
            CurrentFunction = Func;

            std::string suffix;
            if (!isMain){
                // (1) Rewrite return type and function signature
                SourceLocation ReturnTypeStart = Func->getReturnTypeSourceRange().getBegin();
                TheRewriter.ReplaceText(ReturnTypeStart, Func->getReturnType().getAsString().length(), "void");

                std::string originalName = Func->getNameInfo().getName().getAsString();

                for (unsigned i = 0; i < Func->getNumParams(); ++i) {
                    const ParmVarDecl *param = Func->getParamDecl(i);
                    std::string typeStr = param->getType().getAsString();
                    if (!typeStr.empty()) suffix += typeStr[0];
                }

                currentSuffix = suffix;

                std::string newName = originalName + suffix;

                SourceLocation nameLoc = Func->getNameInfo().getBeginLoc();
                TheRewriter.ReplaceText(nameLoc, originalName.length(), newName);

                if (auto FTL = Func->getTypeSourceInfo()->getTypeLoc().getAs<FunctionTypeLoc>()) {
                    SourceLocation LParenLoc = FTL.getLParenLoc();
                    SourceLocation RParenLoc = FTL.getRParenLoc();
                    if (LParenLoc.isValid() && RParenLoc.isValid() && LParenLoc < RParenLoc) {
                        TheRewriter.ReplaceText(SourceRange(LParenLoc, RParenLoc),
                                                "(int thread_idx, int param_index)");
                    }
                }
                
                // Only process the body if it exists.
                if (!Func->hasBody()) {
                    CurrentFunction = nullptr;
                    return;
                }

                // (2) Process return statements if needed.
                if (!Func->getReturnType()->isVoidType()) {
                    if (const CompoundStmt *Body = dyn_cast<CompoundStmt>(Func->getBody())) {
                        for (auto *Stmt : Body->body()) {
                            if (const ReturnStmt *RetStmt = dyn_cast<ReturnStmt>(Stmt)) {
                                SourceLocation RetStart = RetStmt->getBeginLoc();
                                std::string ReturnReplacement = Func->getNameAsString() + suffix + "_params[param_index].return_var = ";
                                TheRewriter.ReplaceText(SourceRange(RetStart, RetStart.getLocWithOffset(6)),
                                                        ReturnReplacement);
                            }
                        }
                    }
                }
            }

            // (3) Traverse function body to rewrite parameter references and record call expressions.
            nonVoidCallees.clear();
            TraverseDecl(const_cast<FunctionDecl *>(Func));

            // (4) Insert the extra lines at the end of the function body.
            if (const CompoundStmt *Body = dyn_cast<CompoundStmt>(Func->getBody())) {
                SourceLocation InsertLoc = Body->getRBracLoc();
                std::string extraCode;
                for (const auto &callee : nonVoidCallees) {
                    extraCode += callee + "_params_index_pool.push(index);\n";
                }
                // If the current function returns a value, mark it done.
                if (!Func->getReturnType()->isVoidType() && !isMain) {
                    extraCode += Func->getNameAsString() + suffix + "_params[param_index]." + Func->getNameAsString() + suffix + "_done = true;\n";
                }
                TheRewriter.InsertTextBefore(InsertLoc, extraCode);
            }

            CurrentFunction = nullptr;
        }
    }

    bool VisitDeclRefExpr(DeclRefExpr *DRE) {
        if (!CurrentFunction)
            return true;

        // (1) Rewrite parameter references.
        if (const ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(DRE->getDecl())) {
            std::string functionName = CurrentFunction->getNameAsString();
            std::string paramName = PVD->getNameAsString();
            std::string replacement = functionName + currentSuffix + "_params[param_index]." + paramName;
            SourceLocation ParamLoc = DRE->getBeginLoc();
            TheRewriter.ReplaceText(ParamLoc, paramName.length(), replacement);
        }

        // (2) Global variable usage rewriting: wrap the entire line in a block with a lock.
        if (const VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
            const SourceManager &SM = TheRewriter.getSourceMgr();
            if (SM.isInSystemHeader(VD->getLocation()))
                return true;

            if (VD->getDeclContext()->isTranslationUnit() || VD->hasGlobalStorage()) {
                SourceLocation loc = DRE->getBeginLoc();
                unsigned line = SM.getSpellingLineNumber(loc);

                // Only process this line once.
                if (processedGlobalLines.find(line) == processedGlobalLines.end()) {
                    processedGlobalLines.insert(line);

                    // Compute the start of the line.
                    unsigned offset = SM.getFileOffset(loc);
                    unsigned colNo = SM.getColumnNumber(SM.getFileID(loc), offset);
                    SourceLocation lineStart = loc.getLocWithOffset(-static_cast<int>(colNo) + 1);

                    FileID fid = SM.getFileID(loc);
                    bool invalid = false;
                    StringRef buffer = SM.getBufferData(fid, &invalid);
                    if (!invalid) {
                        unsigned lineEndOffset = offset;
                        while (lineEndOffset < buffer.size() && buffer[lineEndOffset] != '\n')
                            ++lineEndOffset;
                        SourceLocation lineEnd = SM.getLocForStartOfFile(fid).getLocWithOffset(lineEndOffset);

                        std::string lockBlockStart = "{ unique_lock<mutex> lock(mutexes[thread_idx]);";
                        TheRewriter.InsertText(lineStart, lockBlockStart, true, true);
                        std::string lockBlockEnd = " }";
                        TheRewriter.InsertTextAfterToken(lineEnd, lockBlockEnd);
                    }
                }
            }
        }

        return true;
    }


    // Visitor to handle function calls.
    bool VisitCallExpr(CallExpr *CE) {
        if (!CE->getDirectCallee())
            return true;
        const FunctionDecl *Callee = CE->getDirectCallee();

        std::string functionName = Callee->getNameAsString();
        if (functions.find(functionName) == functions.end()) return true;

        const SourceManager &SM1 = TheRewriter.getSourceMgr();
        std::string argsString;
        for (unsigned i = 0; i < CE->getNumArgs(); ++i) {
            const Expr *arg = CE->getArg(i);
            SourceRange argRange = arg->getSourceRange();
            llvm::StringRef argText = Lexer::getSourceText(CharSourceRange::getTokenRange(argRange), SM1, TheRewriter.getLangOpts());
            if (i > 0) argsString += ", ";
            argsString += argText.str();
        }

        if (const FunctionDecl *Callee = CE->getDirectCallee()) {
            for (unsigned i = 0; i < Callee->getNumParams(); ++i) {
                const ParmVarDecl* paramDecl = Callee->getParamDecl(i);
                QualType paramType = paramDecl->getType();
                functionName += paramType.getAsString()[0];
            }
        }

        std::string pushThreadStmt = "int index; \n { \n unique_lock<mutex> lock(mutexes[thread_idx]);\n if (" +
            functionName + "_params_index_pool.empty()){\n index = " + functionName +
            "_params.size();\n" + functionName + "_params.emplace_back();\n }\n else { \n index = " +
            functionName + "_params_index_pool.front(); \n" + functionName + "_params_index_pool.pop(); \n }\n" +
            functionName + "_params[index] = {" + argsString + "};\n }\n" +
            "pushToThread(" + functionName + "_enumidx);\n";

        if (!Callee->getReturnType()->isVoidType()) {
            pushThreadStmt += "while (!" + functionName + "_params[index]." + functionName +
                "_done) {\n if(!queues[thread_idx].empty()) execute(thread_idx); \n} \n";
            // Record the callee name so that later we add the push statement
            nonVoidCallees.push_back(functionName);
        }

        SourceRange callRange = CE->getSourceRange();
        CharSourceRange charRange = CharSourceRange::getTokenRange(callRange);
        SourceLocation callStart = charRange.getBegin();

        // Find the beginning of the line where the call occurs.
        const SourceManager &SM = TheRewriter.getSourceMgr();
        SourceLocation expansionLoc = SM.getExpansionLoc(callStart);
        FileID fid = SM.getFileID(expansionLoc);
        unsigned offset = SM.getFileOffset(expansionLoc);
        unsigned colNo = SM.getColumnNumber(fid, offset);
        SourceLocation lineStart = callStart.getLocWithOffset(-static_cast<int>(colNo) + 1);

        // Insert the pushThreadStmt before the line.
        TheRewriter.InsertTextBefore(lineStart, pushThreadStmt);

        if (!Callee->getReturnType()->isVoidType()) {
            std::string returnReplacement = functionName + "_params[index]." + functionName + "_return";
            TheRewriter.ReplaceText(charRange, returnReplacement);
        } else {
            SourceLocation callEnd = CE->getEndLoc();
            SourceLocation semiLoc = Lexer::findLocationAfterToken(
                callEnd, tok::semi, SM, TheRewriter.getLangOpts(), false);
            if (semiLoc.isValid()) {
                TheRewriter.RemoveText(SourceRange(callStart, semiLoc));
            } else {
                TheRewriter.RemoveText(charRange);
            }
        }

        return true;
    }

private:
    Rewriter &TheRewriter;
    const FunctionDecl *CurrentFunction;
    std::string currentSuffix;
    std::vector<std::string> nonVoidCallees;
    std::set<unsigned> processedGlobalLines;
};


class FunctionASTConsumer : public ASTConsumer {
public:
    FunctionASTConsumer(Rewriter &R)
        : FuncRewriter(R) {
        Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("function"), &FuncRewriter);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context);
    }

private:
    FunctionRewriter FuncRewriter;
    MatchFinder Matcher;
};

class FunctionFrontendAction : public ASTFrontendAction {
public:
    FunctionFrontendAction() {}

    void EndSourceFileAction() override {
        std::error_code EC;
        llvm::raw_fd_ostream OutFile(SourceFilePath, EC, llvm::sys::fs::OF_Text);
        if (!EC) {
            TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(OutFile);
        }
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        SourceFilePath = file.str();
        return std::make_unique<FunctionASTConsumer>(TheRewriter);
    }

private:
    Rewriter TheRewriter;
    std::string SourceFilePath;
};

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

int main(int argc, const char **argv) {
    if (argc > 1) {
        std::vector<std::string> cppFiles, headerFiles;
        fs::path inputPath(argv[1]);

        for (const auto &entry : fs::recursive_directory_iterator(inputPath)) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension().string();
                if (ext == ".cpp") {
                    cppFiles.push_back(entry.path().string());
                } else if (ext == ".h" || ext == ".hpp") {
                    headerFiles.push_back(entry.path().string());
                }
            }
        }

        for (const auto &file : cppFiles) {
            std::cout << "Processing file: " << file << std::endl;
            FunctionCollector::getInstance().collectFunctions(file);
        }

        std::cout << "Collected functions:\n";
        for (const auto &func : functions) {
            std::cout << " - " << func << "\n";
        }

        const std::set<std::string>& functions_withMangling = FunctionCollector::getInstance().getCollectedFunctions_withMangling();
        std::cout << "Collected functions with mangling:\n";
        for (const auto &func : functions_withMangling) {
            std::cout << " - " << func << "\n";
        }

        auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
        if (!ExpectedParser) {
            return 1;
        }
        CommonOptionsParser &OptionsParser = ExpectedParser.get();

        ClangTool CppTool(OptionsParser.getCompilations(), cppFiles);
        int result = CppTool.run(newFrontendActionFactory<FunctionFrontendAction>().get());
        if (result != 0) {
            std::cerr << "C++ file rewriting failed." << std::endl;
            return result;
        }

        if (!headerFiles.empty()) {
            std::cout << "Processing header files:\n";
            for (const auto &file : headerFiles) {
                std::cout << " - " << file << "\n";
            }
            ClangTool HeaderTool(OptionsParser.getCompilations(), headerFiles);
            result = HeaderTool.run(newFrontendActionFactory<FunctionFrontendAction>().get());
            if (result != 0) {
                std::cerr << "Header file rewriting failed." << std::endl;
                return result;
            }
        }
    }
    return 1;
}