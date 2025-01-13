#ifndef CALL_GRAPH_ANALYZER_H
#define CALL_GRAPH_ANALYZER_H

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <unordered_map>
#include <vector>
#include <string>

class CallGraphAnalyzer {
public:
    CallGraphAnalyzer();
    ~CallGraphAnalyzer() = default;

    // Method to get the call graph as an unordered map
    const std::unordered_map<std::string, std::vector<std::string>> &getCallGraph() const;

    // Method to dump the call graph
    void dumpCallGraph() const;

    // Method to run the analysis
    int runAnalysis(int argc, const char **argv);

private:
    class MyCallHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
    public:
        explicit MyCallHandler(std::unordered_map<std::string, std::vector<std::string>> &graph);
        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

    private:
        std::unordered_map<std::string, std::vector<std::string>> &CallGraph;
    };

    std::unordered_map<std::string, std::vector<std::string>> CallGraph;
};

#endif // CALL_GRAPH_ANALYZER_H