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
//Data
//****
struct bookmark {
   std::string name;
   std::string path;
   
   std::string to_string() const {
      return "`" + name + "` -> " + path;
   }
};
using bookmark_list = std::list<bookmark>;

namespace bookmark_file {
   static const std::string filepath = std::getenv("HOME") + std::string("/.dogear_store");

   bookmark_list get_bookmark_list() {
      std::ifstream infile(filepath);
      if(!infile.is_open()) return {};
      
      bookmark_list bookmarks;
      
      std::string name, path;
      while(std::getline(infile, name, '=')) {
         if(std::getline(infile, path)) {
            bookmarks.push_back({name, path});
         } else {
            break;
         }
      }
      
      return bookmarks;
   }

   void save_bookmark_list(const bookmark_list& bookmarks) {
      std::ofstream outfile(filepath);
      if(!outfile.is_open()) return; //Todo: Raise error
      
      for(const auto& bookmark : bookmarks) {
         outfile << bookmark.name << '=' << bookmark.path << '\n';
      }
   }
};


//****
//Helpers
//****
bool valid_name(const std::string& name) {
   auto valid_char = [](char ch) {
      return std::isalnum(ch) || ch == '_' || ch == '-' || ch == '.';
   };
   return 1 <= name.size() && name.size() <= 40
      && std::all_of(name.begin(), name.end(), valid_char);
}

void invalid_name_info(const std::string& name) {
   std::cout << "Invalid bookmark name: " << name << "\n\n"
      "Valid bookmark names must be have 1-40 characters\n"
      "and can only contain the following:\n"
      "   [a-zA-z0-9] alphanumeric characters\n"
      "   [_.-] underscores, periods, and dashes\n";
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

`dogear recent`:
Displays the 10 most recently accessed bookmarks.

`dogear fold` (<bookmark name>):
Creates a bookmark of the current working directory.

`dogear unfold`:
Removes the bookmark of the current working directory.

`dogear find` (<bookmark name>):
Returns the path associated with the bookmark or an empty string.

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
   auto bookmarks = bookmark_file::get_bookmark_list();
   const auto path = std::filesystem::current_path();
   
   const auto match_bookmark = [path, name](bookmark bm){ return name == bm.name || path.string() == bm.path; };
   const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_bookmark);
   
   //Case 1: No matching name or paths in bookmark list
   if(match_iter == bookmarks.end()) {
      bookmark new_bookmark = {name, path.string()};
      bookmarks.push_front(new_bookmark);
      bookmark_file::save_bookmark_list(bookmarks);
      std::cout << "Added bookmark to: " << new_bookmark.to_string() << '\n';
   } else if(match_iter->name == name && match_iter->path == path.string()) {
      std::cout << "This directory has already been bookmarked\n";
   } else if(match_iter->name == name) {
   //Case 2: Matching name in bookmark list
      std::cout << "This name already points to another directory:\n"
         << "   " << match_iter->to_string() << "\n\n"
         "Would you like to overwrite it (y/n)? ";
         
      std::string response;
      std::getline(std::cin, response);
      if(response == "y") {
         match_iter->path = path.string();
         bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
         bookmark_file::save_bookmark_list(bookmarks);
         std::cout << "Overwritten to: " << match_iter->to_string() << '\n';
      }
   } else { // match_iter->path == pwd
   //Case 3: Matching path in bookmark list
      std::cout << "This is already bookmarked under another name:\n"
         << "   " << match_iter->to_string() << "\n\n"
         "Would you like to change the name (y/n)? ";
      
      std::string response;
      std::getline(std::cin, response);
      if(response == "y") {
         match_iter->name = name;
         bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
         bookmark_file::save_bookmark_list(bookmarks);
         std::cout << "Overwritten to: " << match_iter->to_string() << '\n';
      }
   }
}

//Remove the bookmark that points to the current directory
void unfold() {
   auto bookmarks = bookmark_file::get_bookmark_list();
   const auto path = std::filesystem::current_path();
   
   const auto match_directory = [path](bookmark bm){ return path.string() == bm.path; };
   const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_directory);
   
   if(match_iter == bookmarks.end()) {
      std::cout << "This directory hasn't been bookmarked\n";
   } else {
      std::cout << "Unfolded bookmark: " << match_iter->name << '\n';
      bookmarks.erase(match_iter);
      bookmark_file::save_bookmark_list(bookmarks);
   }
}

//Finds a bookmark by name or returns an empty string
void find(const std::string& name) {
   auto bookmarks = bookmark_file::get_bookmark_list();

   const auto match_name = [name](bookmark bm){ return name == bm.name; };
   const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_name);
   
   if(match_iter != bookmarks.end()) {
      std::cout << match_iter->path << '\n';
      
      //Only reorder the list if bookmark isn't already at the front.
      if(match_iter != bookmarks.begin()) {
         bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
         bookmark_file::save_bookmark_list(bookmarks);
      }
   }
}

//Returns n recently used bookmarks starting with the most recent
void recent(const int n = 10) {
   const auto bookmarks = bookmark_file::get_bookmark_list();
   std::cout << "Recently Used Bookmarks:\n";

   auto match_iter = bookmarks.begin();
   for(int i = 1; i <= n && match_iter != bookmarks.end(); ++i, ++match_iter) {
      std::cout << i << ") " << match_iter->to_string() << '\n';
   }
}

//Go through bookmarks one by one deleting those that are now unnecessary
void edit() {
   auto bookmarks = bookmark_file::get_bookmark_list();
   std::cout << "Editing bookmarks:\n"
   "-Note: Press (q) at any time to quit\n";
   
   bool did_edit = false;
   for(auto iter = bookmarks.begin(); iter != bookmarks.end();) {
      std::cout << "\nBookmark: " << iter->to_string() << "\n"
         "Delete this bookmark (y/n)? ";
      
      std::string response;
      std::getline(std::cin, response);
      if(response == "y") {
      //Case 1: Yes
         std::cout << "'" << iter->name << "' has been deleted\n";
         iter = bookmarks.erase(iter);
         did_edit = true;
      } else if(response == "q"){
      //Case 2: Quit
         break;
      } else {
      //Case 3: Continue
         ++iter;
      }
   }
   
   if(did_edit) {
      bookmark_file::save_bookmark_list(bookmarks);
   }
}

//Removes all bookmarks with invalid names or that point to nonexistent directories
void clean() {
   auto bookmarks = bookmark_file::get_bookmark_list();
   
   std::error_code err;
   const auto is_invalid = [&err](bookmark bm){
      return !(valid_name(bm.name) && std::filesystem::is_directory(bm.path, err));
   };
   bookmarks.remove_if(is_invalid);
   
   bookmark_file::save_bookmark_list(bookmarks);
   std::cout << "Cleaned invalid bookmarks from bookmark list\n";
}

//****
//main
//****
int main(const int argc, const char * argv[]) {
   if (argc < 2) {
      help();
   } else {
      const std::string cmd = argv[1];
      const std::string name = argc >= 3 ? argv[2] : "";
      
      if(cmd == "help") {
         help();
      } else if(cmd == "fold") {
         if(valid_name(name)) {
            fold(name);
         } else {
            invalid_name_info(name);
         }
      } else if(cmd == "unfold") {
         unfold();
      } else if(cmd == "find") {
         find(name);
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
