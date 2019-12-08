#include "GrammerAnalyzer.h"
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    FILE *fp_error = fopen("error.txt", "w");
    vector<Error> allErrors;

    GrammarAnalyzer grammarAnalyzer{"testfile.txt", &allErrors};

    sort(allErrors.begin(), allErrors.end());
    for (auto &e : allErrors) {
        fprintf(fp_error, "%d %c\n", e.getLineNO(), e.getErrorType());
    }

    fclose(fp_error);

    grammarAnalyzer.print("output.txt");

    return 0;
}
