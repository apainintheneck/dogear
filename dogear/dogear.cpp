#include <iostream>
#include <fstream>
#include <cctype>
#include <list>
#include <algorithm>
#include <string>
#include <cstdlib>


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
      std::string filename = "/.dogear_store";
      auto fullpath = std::getenv("HOME") + filename;
      file_.open(fullpath, std::ios::in | std::ios::out | std::ios::trunc);
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
   auto valid_char = [](char ch) { return std::isalnum(ch) || ch == '_' || ch == '-' || ch == '.'; };
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

void fold(const std::string& name) {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();
      const std::string pwd = std::getenv("PWD");
      
      auto match_bookmark = [pwd, name](bookmark bm){ return name == bm.name || pwd == bm.path; };
      const auto iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_bookmark);
      
      if(iter == bookmarks.end()) {
         bookmarks.push_front({name, pwd});
         file.save_bookmark_list(bookmarks);
      } else if(iter->name == name) {
         std::cout << "This bookmark name already exists:\n"
            << "   " << iter->to_string() << "\n"
            "Would you like to overwrite it (y/n)? ";
            
         const char response = std::cin.get();
         if(response == 'y') {
            iter->path = pwd;
            bookmarks.splice(bookmarks.begin(), bookmarks, iter);
            file.save_bookmark_list(bookmarks);
         }
      } else { // iter->path == pwd
         std::cout << "This is already bookmarked under another name:\n"
            << "   " << iter->to_string() << "\n"
            "Would you like to change the name (y/n)? ";
         
         const char response = std::cin.get();
         if(response == 'y') {
            iter->name = name;
            bookmarks.splice(bookmarks.begin(), bookmarks, iter);
            file.save_bookmark_list(bookmarks);
         }
      }
   } else {
      std::cout << "Couldn't open bookmark list\n";
   }
}

void unfold() {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();
      const std::string pwd = std::getenv("PWD");
      
      auto match_directory = [pwd](bookmark bm){ return pwd == bm.path; };
      const auto iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_directory);
      
      if(iter == bookmarks.end()) {
         std::cout << "This directory hasn't been bookmarked\n";
      } else {
         std::cout << "Unfolded bookmark: " << iter->to_string() << '\n';
         bookmarks.erase(iter);
         file.save_bookmark_list(bookmarks);
      }
   } else {
      std::cout << "Couldn't open bookmark list\n";
   }
}

void find(const std::string& name) {
   bookmark_file file;
   if(file.is_open()) {
      auto bookmarks = file.get_bookmark_list();

      auto match_name = [name](bookmark bm){ return name == bm.name; };
      const auto iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_name);
      
      if(iter != bookmarks.end()) {
         std::cout << iter->name;
         bookmarks.splice(bookmarks.begin(), bookmarks, iter);
         file.save_bookmark_list(bookmarks);
      }
   }
   std::cout << '\n';
}

void recent(const int n = 10) {
   bookmark_file file;
   if(file.is_open()) {
      const auto bookmarks = file.get_bookmark_list();
      std::cout << "Recently Used Bookmarks:\n";

      auto iter = bookmarks.begin();
      for(size_t i = 1; i <= n && iter != bookmarks.end(); ++i, ++iter) {
         std::cout << i << ") " << iter->to_string() << '\n';
      }
   } else {
      std::cout << "Couldn't open bookmark list\n";
   }
}

void edit() {
//   bookmark_file file;
//   if(file.is_open()) {
//      auto bookmarks = file.get_bookmark_list();
//
//      auto match_name = [name](bookmark bm){ return name == bm.name; };
//      const auto iter = std::find(bookmarks.begin(), bookmarks.end(), match_name);
//      if(iter != bookmarks.end()) {
//         std::cout << "Unknown bookmark: " << name << '\n';
//      } else if(flip(iter->path)) {
//         std::cout << "Flipped the page to: " << iter->to_string() << '\n';
//      } else {
//         std::cout << "Unable to flip to: " << iter->to_string() << '\n';
//
//         std::cout << "Delete this bookmark (y/n)? ";
//         char response = std::cin.get();
//         if(response == 'y') {
//            bookmarks.erase(iter);
//            file.save_bookmark_list(bookmarks);
//            std::cout << iter->name << " has been deleted\n";
//         }
//      }
//   }
}

void clean() {
   
}

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
