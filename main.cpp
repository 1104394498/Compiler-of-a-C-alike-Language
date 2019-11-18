#include "GrammerAnalyzer.h"
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<Error> allErrors;

    vector<IntermediateCmd> intermediateCodes;

    GrammarAnalyzer grammarAnalyzer{"testfile.txt", &allErrors, &intermediateCodes};

    sort(allErrors.begin(), allErrors.end());

    FILE *fp_error = fopen("error.txt", "w");
    for (auto &e : allErrors) {
        fprintf(fp_error, "%d %c\n", e.getLineNO(), e.getErrorType());
    }
    fclose(fp_error);

    FILE *fp_intermediateCodes_no_optimization = fopen("17376108_王慎执_优化前中间代码.txt", "w");
    for (auto& code:intermediateCodes) {
        fprintf(fp_error, "%s\n", code.print().c_str());
    }
    fclose(fp_intermediateCodes_no_optimization);

    grammarAnalyzer.print("output.txt");

    return 0;
}
