# **dogear** - a command line directory bookmark utility

## Requirements
A *nix system with a C++17 compiler.

## Reason for existing
A lot of the time I find that I want to jump directly to a specific folder or subdirectory and the cd command requires me to write out the whole path which is a bit time consuming and the path can be hard to remember at times.

## Solution
A command line utility where you can bookmark directories and then access them using that nickname. It also saves the most recently accessed directories. Essentially the whole bookmark list is ordered by the most recently accessed bookmark. There is also a shell function used to change directories which calls the main program.

### Note
1. Where I'm from we call the act of folding the corner of a page in a book to save your place dogearing it because it looks a bit like a dog's ear.
2. A simpler version of this written as an all-in-one Fish shell script is available [here](https://gist.github.com/apainintheneck/e43fbcf078097667213d1c8dfe39b21c).

## Example Usage
```shell
$ pwd
/Users/username
$ cd Desktop/projects/
$ dogear fold projects
Added bookmark to: `projects` -> /Users/username/Desktop/projects
$ cd
$ pwd
/Users/username
$ flipto projects
$ pwd
/Users/username/Desktop/projects
```

## Commands

### Flipto
#### A shell function that changes to the bookmarked directory.
- `flipto <bookmark>` --- Flips to directory if bookmark exists.
- `flipto <bookmark> [subdirectory]` --- Flips to directory first and then changes to the subdirectory.
- `flipto help` --- Small man page.

### Dogear
#### The utility that manages all of the bookmarks.
- `dogear recent` --- Displays ten most recently accessed bookmarks.
- `dogear fold <bookmark>` --- Bookmark the current working directory. If nickname already in use, ask to overwrite. If directory already exists, ask to change nickname.
- `dogear unfold` --- Fails if current working directory not found in bookmarks. Prints bookmark removed otherwise.
- `dogear find <bookmark>` --- Returns the directory associated with the bookmark or nothing. This is used by the flipto function.
- `dogear like <search term>` --- Prints a list of bookmarks that include the search term in either the name or directory path.
- `dogear edit` --- Goes through list of bookmarks and allows users to edit them one by one.
- `dogear clean` --- Removes bookmarks with invalid names or that point to nonexistent directories.
- `dogear help` --- Man page that explains how this little utility works.

## How it works
The Dogear program manages the `~/.dogear_store` file where all of the bookmarks are stored. The Flipto shell function calls Dogear and then changes to that directory using the `cd` command.

## Installation
Clone the repository, type `make install` and follow the instuctions.

## Technical Details
- Rspec is used for integration tests which can be found in the `spec/` folder.
- Run tests by typing `make test`.
