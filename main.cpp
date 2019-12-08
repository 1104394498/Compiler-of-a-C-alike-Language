#include "GrammerAnalyzer.h"
#include "Error_GrammarAnalyzer.h"
#include <vector>
#include <algorithm>
#include "Optimizer.h"
#include "MipsGenerator.h"
#include <fstream>
#include <iostream>

using namespace std;
bool runOptimize = true;
bool error_handle = true;

int main() {
    vector<Error> allErrors;
    if (error_handle) {
        // Error Handle
        FILE *fp_error = fopen("error.txt", "w");

        Error_GrammarAnalyzer error_grammarAnalyzer{"testfile.txt", &allErrors};

        sort(allErrors.begin(), allErrors.end());
        for (auto &e : allErrors) {
            fprintf(fp_error, "%d %c\n", e.getLineNO(), e.getErrorType());
        }

        fclose(fp_error);
        /*
        if (!allErrors.empty())
            exit(0);
        */
    }

    allErrors.clear();
    // Normal Handle
    vector<IntermediateCmd> intermediateCodes;

    GrammarAnalyzer grammarAnalyzer{"testfile.txt", &allErrors, &intermediateCodes};
    grammarAnalyzer.print("output.txt");

    sort(allErrors.begin(), allErrors.end());

    /*
    FILE *fp_error = fopen("error.txt", "w");
    for (auto &e : allErrors) {
        fprintf(fp_error, "%d %c\n", e.getLineNO(), e.getErrorType());
    }
    fclose(fp_error);
     */


    FILE *fp_intermediateCodes_no_optimization = fopen("17376108_王慎执_优化前中间代码.txt", "w");
    // FILE *fp_intermediateCodes_no_optimization = fopen("17376108_Shenzhi_Wang_noOptimization.txt", "w");
    for (auto &code:intermediateCodes) {
        fprintf(fp_intermediateCodes_no_optimization, "%s\n", code.print().c_str());
    }
    fclose(fp_intermediateCodes_no_optimization);

    vector<IntermediateCmd> intermediateCodes_optimized;
    if (runOptimize) {
        Optimizer optimizer{grammarAnalyzer.getIntermediateCodes(), grammarAnalyzer.getTempVarGenerator()};
        intermediateCodes_optimized = optimizer.getIntermediateCodes();
        FILE *fp_intermediateCodes_optimized = fopen("17376108_王慎执_优化后中间代码.txt", "w");
        // FILE *fp_intermediateCodes_optimized = fopen("17376108_Shenzhi_Wang_Optimization.txt", "w");
        for (auto &code:intermediateCodes_optimized) {
            fprintf(fp_intermediateCodes_optimized, "%s\n", code.print().c_str());
        }
        fclose(fp_intermediateCodes_optimized);
    }

    vector<MipsCmd> mipsCodes;
    MipsGenerator mipsGenerator{((runOptimize) ? intermediateCodes_optimized : intermediateCodes), &mipsCodes};
    ofstream mips_file{"mips.txt"};
    for (auto &e:mipsCodes) {
        mips_file << e.print() << endl;
        // cout << e.print() << endl;
    }
    mips_file.close();

    return 0;
}
