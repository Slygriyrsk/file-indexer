#include "cli_interface.h"
#include<iostream>
#include<sstream>
#include<iomanip>
#include<filesystem>
using namespace std;

CLIInterface::CLIInterface() {
    //set the home dir to user home dir if avail else stay in the same dir
    string home = getenv("HOME") ? getenv("HOME") : ".";
    index_file_path = home + "/.fastsearch_index.dat";
}

void CLIInterface::printHelp() const {
    //enter the cmds if new user is not able to understand the website
    cout << "\n=== Fast Local Search Tool ===\n";
    cout << "Commands:\n";
    cout << "  index <path>     - Build index for directory\n";
    cout << "  search <query>   - Search files by name\n";
    cout << "  ext <extension>  - Search by file extension\n";
    cout << "  content <query>  - Search file contents (text files only)\n";
    cout << "  load             - Load saved index\n";
    cout << "  save             - Save current index\n";
    cout << "  stats            - Show index statistics\n";
    cout << "  help             - Show this help\n";
    cout << "  quit             - Exit program\n\n";
}

string CLIInterface::formatFileSize(uintmax_t size) const {
    //diff file sizes to make it human readable divide the result always by 1024 so 1536 bytes be 1.50 KB
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    //data size
    double dsize = static_cast<double>(size);//because it may be very big

    while(dsize >= 1024 && unit < 4) {//move to next unit
        dsize /= 1024;
        unit++; //unit must be less than TB assuming 0 based indexing
    }

    //now format the results with just 1 decimal places, even change the setprecision and add the desired unit
    stringstream ss;
    ss<<fixed<<setprecision(1)<<dsize<<" "<<units[unit];
    return ss.str(); //return the converted string
}

void CLIInterface::printResults(const vector<FileInfo>& results, int max_results) const {
    if(results.empty()) {
        cout << "No result found\n";
        return;
    }

    //if found how many
    cout<<"Found " << results.size() << " results";
    if(results.size() > max_results) {
        cout<< " (showing first " << max_results << ")";
    }cout<<":\n\n";

    //now then upto maxresult we will print relevant info about each file, like
    //name, path, size
    for(size_t i = 0; i < min(static_cast<size_t>(max_results), results.size()); ++i) {
        const auto& file = results[i];
        cout << setw(3) << (i + 1) << ". " << file.filename << "\n";
        cout << "     Path: " << file.path << "\n";
        cout << "     Size: " << formatFileSize(file.size) << "\n\n";
    }
}

void CLIInterface::run() {
    cout << "Fast Local Search Tool S3.0\n";
    cout << "Type 'help' for commands or 'quit' to exit.\n\n";

    //try to load existing index
    if(filesystem::exists(index_file_path)) {
        cout << "Loaded existing index...\n";
        if(indexer_.loadIndex(index_file_path)) {
            cout << "Index loaded successfully! (" << indexer_.getTotalFiles() << " files)\n";
        }
    }

    //get ip from user and process the commands entered
    string input;
    while(true) {
        cout << "Search> ";
        getline(cin, input);

        //keep the loop continue/asking user input until user prompts quit or exit
        if(input.empty()) continue;
        if(input == "quit" || input == "exit") break;

        processCommand(input); //now process the command
    }
}

void CLIInterface::processCommand(const string& command) {
    istringstream ss(command);
    string cmd;
    ss >> cmd;

    if(cmd == "help") printHelp();
    else if (cmd == "index") {
        string path;
        ss >> path;
        if(path.empty()) {
            cout << "Usage: index <directory_path>\n";
            return;
        }

        if(!filesystem::exists(path)) {
            //if path doesnot exist the
            cout << "Directory does not exist : " << path << endl;
            return;
        }
        indexer_.buildIndex(path);
    }else if(cmd == "search") {
        string query;
        getline(ss, query);
        if (query.empty() || query == " ") {
            cout << "Usage: search <filename_query>\n";
            return;
        }

        query = query.substr(1);//remove space which is leading
        auto res = indexer_.searchByName(query);
        printResults(res);
    }else if(cmd == "ext") {
        string extension;
        ss >> extension;
        if (extension.empty()) {
            cout << "Usage: ext <file_extension>\n";
            return;
        }
        auto res = indexer_.searchByExtension(extension);
        printResults(res);
    }else if(cmd == "content") {
        string query;
        getline(ss, query);
        if (query.empty() || query == " ") {
            cout << "Usage: content <search_text>\n";
            return;
        }

        query = query.substr(1); // Remove leading space
        
        cout << "Searching file contents (this may take a while)...\n";
        auto res = indexer_.searchByContent(query);
        printResults(res);
    }else if(cmd == "save") {
        indexer_.saveIndex(index_file_path);
        cout << "Index saved to " << index_file_path << "\n";
    }else if(cmd == "load") {
        if (indexer_.loadIndex(index_file_path)) {
            cout << "Index loaded successfully! (" << indexer_.getTotalFiles() << " files)\n";
        } else {
            cout << "Failed to load index from " << index_file_path << "\n";
        }
    }else if(cmd == "stats") {
        cout << "Index Statistics:\n";
        cout << "  Total files: " << indexer_.getTotalFiles() << "\n";
        cout << "  Index file: " << index_file_path << "\n";
        cout << "  Status: " << (indexer_.isIndexingComplete() ? "Ready" : "Building...") << "\n";
    }else {//some unknown command
        cout << "Unknown command: " << cmd << "\n";
        cout << "Type 'help' for available commands.\n";
    }
}