#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include <iostream>
#include <string>

using namespace std;

int failures = 0;

template <typename T, typename U>
void check(const string &name, const T &got, const U &want){
    bool ok = got == want;
    cout << (ok ? "PASS " : "FAIL ") << name << endl;
    if(!ok){
        cout << "  got:  [" << got << "]" << endl;
        cout << "  want: [" << want << "]" << endl;
        failures++;
    }
}

int finish(){
    cout << (failures == 0 ? "All tests passed" : to_string(failures) + " test(s) failed") << endl;
    return failures == 0 ? 0 : 1;
}
#endif
