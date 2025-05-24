#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

bool recursive = false;
// Функция для обработки и включения файлов
bool ProcessIncludes(const path& current_file, const vector<path>& include_directories, ofstream& out_stream) {
    ifstream in_stream(current_file);
    if (!in_stream.is_open()) { 
        return false; // Не удалось открыть текущий файл 
    }
    
    string line;
    int line_number = 0;
                           
    regex include_dquotes(R"(\s*#\s*include\s*"([^"]*)\"\s*)");
    regex include_angle  (R"(\s*#\s*include\s*<([^>]*)>\s*)");
    
   while (getline(in_stream, line)) {
        line_number++;
        
        smatch match;
       // check for the presence of the directive #include "..."
        if (regex_match(line, match, include_dquotes)) {
            path included_file = current_file.parent_path() / match[1].str(); // path relative to the current file
            ifstream included_stream(included_file);
            
            if (!included_stream.is_open()) {
                // If it was not possible to open, continue to search in include_directories
                for (const auto& dir : include_directories) {
                    included_file = dir / match[1].str();
                    included_stream.open(included_file);
                    if (included_stream.is_open()) {
                        break; // file found
                    }
                }
            }
            
            if (!included_stream.is_open()) {
                cout << "unknown include file " << match[1].str() 
                     << " at file " << current_file.string() 
                     << " at line " << line_number << endl;
                return false; // notify of the error and exit
            }
            
            // recording the contents of the included file
            string included_line;
            while (getline(included_stream, included_line)) {
                ProcessIncludes(included_file, include_directories, out_stream);
                break;
                
            }
            included_stream.close();
            continue; // go to the next line of the file
        }

        // check for the directive #include <...>
        else if (regex_match(line, match, include_angle)) {
            path included_file = current_file.parent_path() / match[1].str(); // path relative to the current file
            ifstream included_stream(included_file);
            
            if (!included_stream.is_open()) {
				//If it was not possible to open, continue to search in include_directories
                for (const auto& dir : include_directories) {
                    included_file = dir / match[1].str();
                    included_stream.open(included_file);
                    if (included_stream.is_open()) {                        
                        break; // file found
                    }
                }
            }
            
            if (!included_stream.is_open()) {
                cout << "unknown include file " << match[1].str() 
                     << " at file " << current_file.string() 
                     << " at line " << line_number << endl;
                return false; // notify of the error and exit
            }
             
            // recording the contents of the included file
            string included_line;
            while (getline(included_stream, included_line)) {
                ProcessIncludes(included_file, include_directories, out_stream);
                 break;
                
            }
            included_stream.close();
            continue; // go to the next line of the file
        }
        
       else{
        // write a line if it is not a directive #include 
        out_stream << line << endl;
       }
    }

    in_stream.close();
    return true; // all files have been processed successfully
}


// напишите эту функцию
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
    ofstream out_stream(out_file, ios::ate);
    if (!out_stream.is_open()) {
        return false; // Не удалось открыть файл для записи
    }
    return ProcessIncludes(in_file, include_directories, out_stream);
}
