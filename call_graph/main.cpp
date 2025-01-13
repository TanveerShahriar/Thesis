#include "CallGraphAnalyzer.h"

int main(int argc, const char **argv) {
    CallGraphAnalyzer analyzer;

    // Run the analysis
    if (analyzer.runAnalysis(argc, argv) != 0) {
        return 1;
    }

    // Dump the call graph
    analyzer.dumpCallGraph();

    return 0;
}