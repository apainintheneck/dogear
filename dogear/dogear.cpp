#include <iostream>
#include <fstream>
#include <cctype>
#include <list>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <system_error>


//****
//Classes
//****
struct bookmark {
   std::string name;
   std::string path;
   
   std::string to_string() const {
      return name + " = " + path;
   }
};
using bookmark_list = std::list<bookmark>;

class bookmark_file {
public:
   bookmark_file() {
      std::filesystem::path path = std::getenv("HOME");
      const std::string filename = ".dogear_store";
      path /= filename;
      file_.open(path, std::ios::in | std::ios::out | std::ios::trunc);
   }
   
   ~bookmark_file() {
      if(is_open()) file_.close();
   }
   
   bool is_open() const { return file_.is_open(); }
   
   void save_bookmark_list(const bookmark_list& bookmarks) {
      for(const auto& bookmark : bookmarks) {
         file_ << bookmark.name << '=' << bookmark.path << '\n';
      }
   }
   
   bookmark_list get_bookmark_list() {
      bookmark_list bookmarks;
      
      std::string name, path;
      while(std::getline(file_, name, '=')) {
         if(std::getline(file_, path)) {
            bookmarks.push_back({name, path});
         } else {
            break;
         }
      }
      
      return bookmarks;
   }
   
private:
   std::fstream file_;
};


//****
//Helpers
//****
bool valid_name(const std::string& bookmark) {
   auto valid_char = [](char ch) {
      return std::isalnum(ch) || ch == '_' || ch == '-' || ch == '.';
   };
   bool is_valid = 1 <= bookmark.size() && bookmark.size() <= 40
      && std::all_of(bookmark.begin(), bookmark.end(), valid_char);
   
   if(is_valid) {
      return true;
   } else {
      std::cout << "Invalid bookmark name: " << bookmark << "\n\n"
         "Valid bookmark names must be have 1-40 characters\n"
         "and can only contain the following:\n"
         "   [a-zA-z0-9] alphanumeric characters\n"
         "   [_.-] underscores, periods, and dashes\n";
      
      return false;
   }
}


//****
//Commands
//****
void help() {
   std::cout <<
R"~(
[dogear]
--------
Bookmark directories for easy access in the future.
   
To change to a bookmarked directory type:
`flipto (<bookmark name>)`
   
   
[dogear subcommands]
--------------------
`dogear` [`help`]:
Displays this page.

`dogear fold` (<bookmark name>):
Creates a bookmark of the current working directory.

`dogear unfold`:
Removes the bookmark of the current working directory.

`dogear find` (<bookmark name>):
Returns the path associated with the bookmark or an empty string.

`dogear recent`:
Displays the 10 most recently accessed bookmarks.

`dogear edit`:
Allows you to edit your bookmarks one by one.

`dogear clean`:
Removes bookmarks pointing to nonexistent directories.
   

[more information]
------------------
Bookmarks are stored in `~/dogear_store` file.
Project can be found here: (Github Link)

)~";
}

//Add a bookmark that points to the current directory
void fold(const std::string& name) {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();
      const auto path = std::filesystem::current_path();
      
      const auto match_bookmark = [path, name](bookmark bm){ return name == bm.name || path.string() == bm.path; };
      const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_bookmark);
      
      //Case 1: No matching name or paths in bookmark list
      if(match_iter == bookmarks.end()) {
         bookmarks.push_front({name, path.string()});
         file.save_bookmark_list(bookmarks);
      } else if(match_iter->name == name) {
      //Case 2: Matching name in bookmark list
         std::cout << "This name already points to another directory:\n"
            << "   " << match_iter->to_string() << "\n"
            "Would you like to overwrite it (y/n)? ";
            
         const char response = std::cin.get();
         if(response == 'y') {
            match_iter->path = path.string();
            bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
            file.save_bookmark_list(bookmarks);
         }
      } else { // match_iter->path == pwd
      //Case 3: Matching path in bookmark list
         std::cout << "This is already bookmarked under another name:\n"
            << "   " << match_iter->to_string() << "\n"
            "Would you like to change the name (y/n)? ";
         
         const char response = std::cin.get();
         if(response == 'y') {
            match_iter->name = name;
            bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
            file.save_bookmark_list(bookmarks);
         }
      }
   } else {
      std::cout << "Couldn't open bookmark list\n";
   }
}

//Remove the bookmark that points to the current directory
void unfold() {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();
      const auto path = std::filesystem::current_path();
      
      const auto match_directory = [path](bookmark bm){ return path.string() == bm.path; };
      const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_directory);
      
      if(match_iter == bookmarks.end()) {
         std::cout << "This directory hasn't been bookmarked\n";
      } else {
         std::cout << "Unfolded bookmark: " << match_iter->name << '\n';
         bookmarks.erase(match_iter);
         file.save_bookmark_list(bookmarks);
      }
   } else {
      std::cout << "Couldn't open bookmark list\n";
   }
}

//Finds a bookmark by name or returns an empty string
void find(const std::string& name) {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();

      const auto match_name = [name](bookmark bm){ return name == bm.name; };
      const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_name);
      
      if(match_iter != bookmarks.end()) {
         std::cout << match_iter->path;
         bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
         file.save_bookmark_list(bookmarks);
      }
   }
   std::cout << '\n';
}

//Returns n recently used bookmarks starting with the most recent
void recent(const int n = 10) {
   bookmark_file file;
   if(file.is_open()) {
      const auto bookmarks = file.get_bookmark_list();
      std::cout << "Recently Used Bookmarks:\n";

      auto match_iter = bookmarks.begin();
      for(size_t i = 1; i <= n && match_iter != bookmarks.end(); ++i, ++match_iter) {
         std::cout << i << ") " << match_iter->to_string() << '\n';
      }
   } else {
      std::cout << "Couldn't open bookmark list\n";
   }
}

//Go through bookmarks one by one deleting those that are now unnecessary
void edit() {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();
      std::cout << "Editing bookmarks:\n"
      "-Note: Press (q) at any time to quit\n";
      
      bool did_edit = false;
      for(auto iter = bookmarks.begin(); iter != bookmarks.end();) {
         std::cout << "\nBookmark: " << iter->to_string() << "\n"
            "Delete this bookmark (y/n)? ";
         
         char response = std::cin.get();
         if(response == 'y') {
         //Case 1: Yes
            iter = bookmarks.erase(iter);
            did_edit = true;
            std::cout << iter->name << " has been deleted\n";
         } else if(response == 'q'){
         //Case 2: Quit
            break;
         } else {
         //Case 3: Continue
            ++iter;
         }
      }
      
      if(did_edit) {
         file.save_bookmark_list(bookmarks);
      }
   }
}

//Removes all bookmarks pointing to nonexistent directories
void clean() {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();
      
      std::error_code err;
      const auto nonexistent_path = [&err](bookmark bm){
         return !std::filesystem::is_directory(bm.path, err);
      };
      bookmarks.remove_if(nonexistent_path);
      
      std::cout << "Cleaned nonexistent directories from bookmark list\n";
   } else {
      std::cout << "Couldn't open bookmark list\n";
   }
}

//****
//main
//****
int main(const int argc, const char * argv[]) {
   if (argc < 2) {
      help();
   } else {
      const std::string cmd = argv[1];
      const std::string bookmark = argc >= 3 ? argv[2] : "";
      
      if(cmd == "help") {
         help();
      } else if(cmd == "fold") {
         if(valid_name(bookmark))
            fold(bookmark);
      } else if(cmd == "unfold") {
         unfold();
      } else if(cmd == "find") {
         find(bookmark);
      } else if(cmd == "recent") {
         recent();
      } else if(cmd == "edit") {
         edit();
      } else if(cmd == "clean") {
         clean();
      } else {
         help();
      }
   }
}
