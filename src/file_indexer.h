#pragma once //either use it or use 
/* #ifndef _MYHEADER_H
#define _MYHEADER_H
... WRITE THE LOGIC HERE
#endif */

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
using namespace std;

struct FileInfo {
    //theses are the most common thigns to included
    string path;
    string filename;
    string extension;
    uintmax_t size;
    filesystem::file_time_type last_modified;

    FileInfo(const filesystem::path& p) : path(p.string()), filename(p.filename().string()), extension(p.extension().string()), size(0) {
        try {
            if(filesystem::exists(p)) {
                size = filesystem::file_size(p);
                last_modified = filesystem::last_write_time(p);
            }
        } catch (...) {
            size = 0;
        }
    }
};

class FileIndexer {
private:
    vector<FileInfo> files_;
    unordered_map<string, vector<size_t>> filename_index_;
    unordered_map<string, vector<size_t>> extension_index_;
    mutex index_mutex_; //we made it mututally exclusive so that multiple thread doesnot colide with this
    atomic<bool> indexing_complete_{false};
    atomic<size_t> files_processed_{0};

    void indexFile(const FileInfo& file_info, size_t index);
    void scanDirectoryRecursive(const filesystem::path& dir);

public:
    void buildIndex(const string& root_path);
    vector<FileInfo> searchByName(const string& query) const;
    vector<FileInfo> searchByExtension(const string& query) const;
    vector<FileInfo> searchByContent(const string& query) const;

    bool isIndexingComplete() const { return indexing_complete_; }
    size_t getFilesProcessed() const { return files_processed_; }
    size_t getTotalFiles() const { return files_.size(); }
    
    void saveIndex(const std::string& filename) const;
    bool loadIndex(const std::string& filename);
};