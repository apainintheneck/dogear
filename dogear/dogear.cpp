#include <iostream>
#include <fstream>
#include <cctype>
#include <list>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include "sysexits.h"

int exit_code = EXIT_SUCCESS;

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

bookmark_list get() {
   std::ifstream infile(filepath);
   if(not infile.is_open()) return {};
   
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

void save(const bookmark_list& bookmarks) {
   std::ofstream outfile(filepath);
   if(not outfile.is_open()) {
      exit_code = EX_CANTCREAT;
      return;
   }
   
   for(const auto& bookmark : bookmarks) {
      outfile << bookmark.name << '=' << bookmark.path << '\n';
   }
}

} //namespace bookmark_file


//****
//Helpers
//****
bool is_valid_name(const std::string& name) {
   auto valid_char = [](char ch) {
      return std::isalnum(ch) or ch == '_' or ch == '-' or ch == '.';
   };
   return 1 <= name.size() and name.size() <= 40
      and std::all_of(name.begin(), name.end(), valid_char);
}

void invalid_name_info(const std::string& name) {
   std::cout << "Invalid bookmark name: " << name << "\n\n"
      "Valid bookmark names must be have 1-40 characters\n"
      "and can only contain the following:\n"
      "   [a-zA-z0-9] alphanumeric characters\n"
      "   [_.-] underscores, periods, and dashes\n";
}

//A case insensitive alternative to std::string.find()
bool includes(const std::string& to_search, const std::string& search_term) {
   auto cmp_lowercase = [](char ch1, char ch2) {
      return std::tolower(ch1) == std::tolower(ch2);
   };
   
   auto iter = std::search(to_search.begin(), to_search.end(),
                           search_term.begin(), search_term.end(),
                           cmp_lowercase);
   return iter != to_search.end();
}

void usage() {
   std::cout <<
R"~(Example usage:
   dogear recent
   dogear fold <bookmark>
   dogear unfold
   dogear find <bookmark>
   dogear like <search term>
   dogear edit
   dogear clean
   dogear help
)~";
}


//****
//Commands
//****
//Returns n recently used bookmarks starting with the most recent
void recent(const int n = 10) {
   const auto bookmarks = bookmark_file::get();

   if(bookmarks.empty()) {
      std::cout << "No bookmarks have been added\n";
   } else {
      std::cout << "Recently Used Bookmarks:\n";

      auto match_iter = bookmarks.begin();
      for(int i = 1; i <= n and match_iter != bookmarks.end(); ++i, ++match_iter) {
         std::cout << i << ") " << match_iter->to_string() << '\n';
      }
   }
}

//Add a bookmark that points to the current directory
void fold(const std::string& name) {
   if(not is_valid_name(name)) {
      invalid_name_info(name);
      exit_code = EX_USAGE;
      return;
   }
   
   auto bookmarks = bookmark_file::get();
   const auto path = std::filesystem::current_path();
   
   const auto match_bookmark = [&path, &name](bookmark bm){ return name == bm.name or path.string() == bm.path; };
   const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_bookmark);
   
   if(match_iter == bookmarks.end()) {
      //Case 1: No matching name or paths in bookmark list
      bookmark new_bookmark = {name, path.string()};
      bookmarks.push_front(new_bookmark);
      bookmark_file::save(bookmarks);
      std::cout << "Added bookmark to: " << new_bookmark.to_string() << '\n';
   } else if(match_iter->name == name and match_iter->path == path.string()) {
      //Case 2: This exact bookmark already exists
      std::cout << "This directory has already been bookmarked\n";
      bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
      bookmark_file::save(bookmarks);
   } else if(match_iter->name == name) {
      //Case 3: Matching name in bookmark list
      std::cout << "This name already points to another directory:\n"
         << "   " << match_iter->to_string() << "\n\n"
         "Would you like to overwrite it (y/n)? ";
         
      std::string response;
      std::getline(std::cin, response);
      if(response == "y") {
         match_iter->path = path.string();
         bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
         bookmark_file::save(bookmarks);
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
         bookmark_file::save(bookmarks);
         std::cout << "Overwritten to: " << match_iter->to_string() << '\n';
      }
   }
}

//Remove the bookmark that points to the current directory
void unfold() {
   auto bookmarks = bookmark_file::get();
   const auto path = std::filesystem::current_path();
   
   const auto match_directory = [&path](bookmark bm){ return path.string() == bm.path; };
   const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_directory);
   
   if(match_iter == bookmarks.end()) {
      std::cout << "This directory hasn't been bookmarked\n";
      exit_code = EXIT_FAILURE;
   } else {
      std::cout << "Unfolded bookmark: " << match_iter->name << '\n';
      bookmarks.erase(match_iter);
      bookmark_file::save(bookmarks);
   }
}

//Finds a bookmark by name or returns an empty string.
void find(const std::string& name) {
   auto bookmarks = bookmark_file::get();

   const auto match_name = [&name](bookmark bm){ return name == bm.name; };
   const auto match_iter = std::find_if(bookmarks.begin(), bookmarks.end(), match_name);
   
   if(match_iter != bookmarks.end()) {
      std::cout << match_iter->path << '\n';
      
      //Only reorder the list if bookmark isn't already at the front.
      if(match_iter != bookmarks.begin()) {
         bookmarks.splice(bookmarks.begin(), bookmarks, match_iter);
         bookmark_file::save(bookmarks);
      }
      return;
   }

   exit_code = EXIT_FAILURE;
}

//Lists all bookmarks that contain the given string in order of most recently
//accessed. It also ignores case and looks at names and directory paths.
void like(const std::string& search_term) {
   if(search_term.empty()) {
      std::cout << "Missing a valid search term\n";
      exit_code = EX_USAGE;
      return;
   }
   
   auto bookmarks = bookmark_file::get();
   std::cout << "Bookmarks like `" << search_term << "`:\n";

   bool is_match = false;
   for(const auto& bookmark: bookmarks) {
      if(includes(bookmark.name, search_term) or includes(bookmark.path, search_term)) {
         std::cout << "*) " << bookmark.to_string() << '\n';
         is_match = true;
      }
   }

   if(not is_match) exit_code = EXIT_FAILURE;
}

//Go through bookmarks one by one deleting those that are now unnecessary
void edit() {
   auto bookmarks = bookmark_file::get();

   if(bookmarks.empty()) {
      std::cout << "There are no bookmarks to edit\n";
   } else {
      std::cout << "Editing Bookmarks:\n"
      "-Note: Press (q) at any time to quit\n";
      
      bool edit_flag = false;
      for(auto iter = bookmarks.begin(); iter != bookmarks.end();) {
         std::cout << "\nBookmark: " << iter->to_string() << "\n"
            "Delete this bookmark (y/n)? ";
         
         std::string response;
         std::getline(std::cin, response);
         if(response == "y") {
         //Case 1: Yes
            std::cout << "`" << iter->name << "` has been deleted\n";
            iter = bookmarks.erase(iter);
            edit_flag = true;
         } else if(response == "q"){
         //Case 2: Quit
            break;
         } else {
         //Case 3: Continue
            ++iter;
         }
      }
      
      if(edit_flag) {
         bookmark_file::save(bookmarks);
      }
   }
}

//Removes all bookmarks with invalid names or that point to nonexistent directories
void clean() {
   auto bookmarks = bookmark_file::get();
   
   if(bookmarks.empty()) {
      std::cout << "No bookmarks to clean\n";
   } else {
      std::error_code err; //Used for noexcept is_directory call
      const auto is_invalid = [&err](bookmark bm){
         return not (is_valid_name(bm.name) and std::filesystem::is_directory(bm.path, err));
      };
      bookmarks.remove_if(is_invalid);
      
      bookmark_file::save(bookmarks);
      std::cout << "Cleaned invalid bookmarks from bookmark list\n";
   }
}

void help() {
   std::cout <<
R"~(
[dogear]
--------
Bookmark directories for easy access in the future.
Dogear manages bookmarks while flipto allows you to move to them.


[flipto command]
-----------------
`flipto <bookmark>`
Flips to bookmarked directory if it exists.

`flipto <bookmark> [subdirectory]`
Flips to bookmarked directory first and then
attempts to flip to the subdirectory.


[dogear subcommands]
--------------------
`dogear recent`:
Displays the 10 most recently accessed bookmarks.

`dogear fold` <bookmark>:
Creates a bookmark of the current working directory.

`dogear unfold`:
Removes the bookmark of the current working directory.

`dogear find` <bookmark>:
Returns the path associated with the bookmark or an empty string.

`dogear like` <search term>:
Returns a list of bookmarks containing the search term.

`dogear edit`:
Allows you to edit your bookmarks one by one.

`dogear clean`:
Removes bookmarks pointing to nonexistent directories.


[more information]
------------------
Bookmarks are stored in `~/dogear_store` file.
Project can be found here: https://github.com/apainintheneck/dogear

)~";
}

//****
//main
//****
int main(const int argc, const char * argv[]) {
   if (argc < 2) {
      usage();
      exit_code = EX_USAGE;
   } else {
      const std::string cmd = argv[1];
      const std::string arg = argc >= 3 ? argv[2] : "";
      
      if(cmd == "help") {
         help();
      } else if(cmd == "fold") {
         fold(arg);
      } else if(cmd == "unfold") {
         unfold();
      } else if(cmd == "find") {
         find(arg);
      } else if(cmd == "like") {
         like(arg);
      } else if(cmd == "recent") {
         recent();
      } else if(cmd == "edit") {
         edit();
      } else if(cmd == "clean") {
         clean();
      } else {
         usage();
         exit_code = EX_USAGE;
      }
   }

   return exit_code;
}
