#include "GrammerAnalyzer.h"
#include <vector>
#include <algorithm>
#include "Optimizer.h"

using namespace std;

bool runOptimize = true;

int main() {
    vector<Error> allErrors;

    vector<IntermediateCmd> intermediateCodes;

    GrammarAnalyzer grammarAnalyzer{"testfile.txt", &allErrors, &intermediateCodes};
    grammarAnalyzer.print("output.txt");

    sort(allErrors.begin(), allErrors.end());

    FILE *fp_error = fopen("error.txt", "w");
    for (auto &e : allErrors) {
        fprintf(fp_error, "%d %c\n", e.getLineNO(), e.getErrorType());
    }
    fclose(fp_error);

    FILE *fp_intermediateCodes_no_optimization = fopen("17376108_王慎执_优化前中间代码.txt", "w");
    for (auto& code:intermediateCodes) {
        fprintf(fp_intermediateCodes_no_optimization, "%s\n", code.print().c_str());
    }
    fclose(fp_intermediateCodes_no_optimization);

    if (runOptimize) {
        Optimizer optimizer{grammarAnalyzer.getIntermediateCodes()};
        vector<IntermediateCmd> intermediateCodes_optimized = optimizer.getIntermediateCodes();
        FILE* fp_intermediateCodes_optimized = fopen("17376108_王慎执_优化后中间代码.txt", "w");
        for (auto& code:intermediateCodes_optimized) {
            fprintf(fp_intermediateCodes_optimized, "%s\n", code.print().c_str());
        }
        fclose(fp_intermediateCodes_optimized);

    }

    return 0;
}
