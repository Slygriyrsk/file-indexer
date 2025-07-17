#include "file_indexer.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <sstream>
using namespace std;

void FileIndexer::indexFile(const FileInfo& file_info, size_t index) {
    lock_guard<mutex> lock(index_mutex_);//this lock guard will lock the given mutex(mutual inclusion) s.t only 1 thread can access this protected code
    //index by file name
    string lower_filename = file_info.filename;
    std::transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(),
               [](unsigned char c){ return std::tolower(c); });

    filename_index_[lower_filename].push_back(index);

    //index by extension
    if(!file_info.extension.empty()) {
        string lower_ext = file_info.extension;
        std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(),
               [](unsigned char c){ return std::tolower(c); });
        extension_index_[lower_ext].push_back(index);
    }
}

void FileIndexer::scanDirectoryRecursive(const filesystem::path& dir) {
    try {
        for(const auto& entry : filesystem::recursive_directory_iterator(dir)) {
            if(entry.is_regular_file()) {
                FileInfo file_info(entry.path());
                size_t index = files_.size();
                files_.push_back(file_info);
                indexFile(file_info, index);
                files_processed_++;
            }
        }
    } catch (const filesystem::filesystem_error& e) {
        //cerr is used for sending console error messages
        cerr << "Error scanning directory: " << e.what() << endl;
    }
}

void FileIndexer::buildIndex(const string& root_path) {
    indexing_complete_ = false;
    files_processed_ = 0;
    files_.clear();
    filename_index_.clear();
    extension_index_.clear();

    cout << "Building index for: " << root_path << endl;
    //now how long the work will take time for scanning the dir
    auto start = chrono::high_resolution_clock::now();
    scanDirectoryRecursive(root_path);
    auto end = chrono::high_resolution_clock::now();

    //calculate how much time it took
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Indexing complete! Processed " << files_.size() << " files in " << duration.count() << "ms" << endl;
    
    indexing_complete_ = true;//mark the curr index to be done
}

vector<FileInfo> FileIndexer::searchByName(const string& query) const {
    vector<FileInfo> res;
    string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(),
               [](unsigned char c){ return std::tolower(c); });
    
    for(const auto& file : files_) {
        string lower_filename = file.filename;
        std::transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(),
               [](unsigned char c){ return std::tolower(c); });
        if(lower_filename.find(lower_query) != string::npos) {
            res.push_back(file);
        }
    }
    return res;
}

vector<FileInfo> FileIndexer::searchByExtension(const string& extension) const {
    vector<FileInfo> res;
    string lower_extension = extension;
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(),
               [](unsigned char c){ return std::tolower(c); });
    
    if(lower_extension[0] != '.') {
        lower_extension = '.' + lower_extension; //.txt not txt. so avoid +=
    }

    auto it = extension_index_.find(lower_extension);
    if(it != extension_index_.end()) {
        for(size_t index : it->second) {
            res.push_back(files_[index]);
        }
    }
    return res;
}

vector<FileInfo> FileIndexer::searchByContent(const string& query) const {
    vector<FileInfo> res;
    string lower_content = query;
    std::transform(lower_content.begin(), lower_content.end(), lower_content.begin(),
               [](unsigned char c){ return std::tolower(c); });

    for(const auto& file : files_) {
        //we will only search basic files like text only
        if(file.extension == ".txt" || file.extension == ".cpp" ||
            file.extension == ".h" || file.extension == ".py" ||
            file.extension == ".js" || file.extension == ".html") {

            ifstream infile(file.path); //now we will read the file contents
            //check if its open
            if(infile.is_open()) {
                string line; //we will read each line of the file
                bool found = false; //this is just for the edge cases like we donot repeat it again
                while(getline(infile, line) && !found) {
                    //always transform into lowercase to reduce issue
                    std::transform(line.begin(), line.end(), line.begin(),
               [](unsigned char c){ return std::tolower(c); });
                    if(line.find(lower_content) != string::npos) {
                        res.push_back(file);
                        found = true;
                    }
                }
            }
        }
    }
    return res;
}

//i am making this "const" becz everything gets saved on disk so cannot modified any member vars its called on
//or simple its read-only
void FileIndexer::saveIndex(const string& filename) const {
    //create a new file and save that using ofstream
    ofstream out(filename, ios::binary);//for img, audio anything other than text
    if(!out) return;

    size_t cnt = files_.size();
    out.write(reinterpret_cast<const char*>(&cnt), sizeof(cnt));

    for(const auto& file : files_) {
        size_t path_len = file.path.length();
        out.write(reinterpret_cast<const char*>(&path_len), sizeof(path_len));
        out.write(file.path.c_str(), path_len);
        
        size_t name_len = file.filename.length();
        out.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        out.write(file.filename.c_str(), name_len);
        
        size_t ext_len = file.extension.length();
        out.write(reinterpret_cast<const char*>(&ext_len), sizeof(ext_len));
        out.write(file.extension.c_str(), ext_len);
        
        out.write(reinterpret_cast<const char*>(&file.size), sizeof(file.size));
    }
}

bool FileIndexer::loadIndex(const string& filename) {
    ifstream in(filename, ios::binary);
    if(!in) return false;

    files_.clear();
    filename_index_.clear();
    extension_index_.clear();

    size_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    for (size_t i = 0; i < count; ++i) {
        FileInfo file_info("");
        
        size_t path_len;
        in.read(reinterpret_cast<char*>(&path_len), sizeof(path_len));
        file_info.path.resize(path_len);
        in.read(&file_info.path[0], path_len);
        
        size_t name_len;
        in.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
        file_info.filename.resize(name_len);
        in.read(&file_info.filename[0], name_len);
        
        size_t ext_len;
        in.read(reinterpret_cast<char*>(&ext_len), sizeof(ext_len));
        file_info.extension.resize(ext_len);
        in.read(&file_info.extension[0], ext_len);
        
        in.read(reinterpret_cast<char*>(&file_info.size), sizeof(file_info.size));
        
        files_.push_back(file_info);
        indexFile(file_info, i);
    }
    
    indexing_complete_ = true;
    return true;
}
