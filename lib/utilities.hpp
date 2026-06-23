#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <iostream>
#include <regex>
#include <fstream>
#include <windows.h>  // Windows only library

using namespace std;


string exec(string cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = _popen(cmd.data(), "r");
    if (!pipe) throw runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        _pclose(pipe);
        throw;
    }
    _pclose(pipe);

    // Remove newline
    regex newline_re("\n$");
    result = regex_replace(result, newline_re, "");

    return result;
}

vector<string> split_string(const string& s, char delimiter) {
  vector<string> tokens;
  size_t start = 0, end = 0;
  while ((end = s.find(delimiter, start)) != string::npos) {
    tokens.push_back(s.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(s.substr(start));
  return tokens;
}

string getAbsoluteDirectory(const string& relativePath){
    char absolutePath[MAX_PATH];
    GetFullPathNameA(relativePath.c_str(), MAX_PATH, absolutePath, nullptr);

    string path = string(absolutePath);
    size_t pos = path.find_last_of("\\");
    if (pos != string::npos){
        path.erase(pos + 1);
    }

    return path;
}

string getAbsolutePath(const string& baseDir, const string& relativePath) {
    char absolutePath[MAX_PATH];
    GetFullPathNameA((baseDir + "\\" + relativePath).c_str(), MAX_PATH, absolutePath, nullptr);
    return string(absolutePath);
}
#endif
