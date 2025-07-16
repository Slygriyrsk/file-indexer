#pragma once
#include "file_indexer.h"
#include<string>
using namespace std;

class CLIInterface {
private:
    FileIndexer indexer_;
    string index_file_path;

    void printHelp() const;
    void printResults(const vector<FileInfo>& results, int max_results = 20) const;
    string formatFileSize(std::uintmax_t size) const;

public:
    CLIInterface();
    void run();
    void processCommand(const string& command);
};