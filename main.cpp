#include "GrammerAnalyzer.h"
#include <vector>
#include <algorithm>
#include "Optimizer.h"
#include "MipsGenerator.h"

using namespace std;

bool runOptimize = false;

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
    for (auto &code:intermediateCodes) {
        fprintf(fp_intermediateCodes_no_optimization, "%s\n", code.print().c_str());
    }
    fclose(fp_intermediateCodes_no_optimization);

    vector<IntermediateCmd> intermediateCodes_optimized;
    if (runOptimize) {
        Optimizer optimizer{grammarAnalyzer.getIntermediateCodes()};
        intermediateCodes_optimized = optimizer.getIntermediateCodes();
        FILE *fp_intermediateCodes_optimized = fopen("17376108_王慎执_优化后中间代码.txt", "w");
        for (auto &code:intermediateCodes_optimized) {
            fprintf(fp_intermediateCodes_optimized, "%s\n", code.print().c_str());
        }
        fclose(fp_intermediateCodes_optimized);
    }

    vector<MipsCmd> mipsCodes;
    MipsGenerator mipsGenerator{((runOptimize) ? intermediateCodes_optimized : intermediateCodes), &mipsCodes};
    FILE *fp_mips = fopen("mips.txt", "w");
    for (auto &e:mipsCodes) {
        fprintf(fp_mips, "%s\n", e.print().c_str());
    }
    fclose(fp_mips);

    return 0;
}
