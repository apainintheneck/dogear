RSpec.describe "integration tests: " do
    # Setup
    before(:all) do
        `make`
        `mv ~/.dogear_store ~/.dogear_store.tmp`
    end

    # Helpers
    def run_script(cmd, args=nil)
        raw_output = nil
        IO.popen("./build/bins/dogear #{cmd}", "r+") do |pipe|
            if args
                args.each do |arg|
                    pipe.puts arg
                rescue Errno::EPIPE
                    break
                end
            end
        
            pipe.close_write
    
            # Read entire output
            raw_output = pipe.gets(nil)
        end

        raw_output ? raw_output.split("\n") : []
    end

    def fill_bookmark_list(bookmarks)
        # Add bookmarks to list
        File.open("#{Dir.home}/.dogear_store", "w") do |file|
            bookmarks.each do |name, path|
                file << "#{name}=#{path}\n"
            end
        end
    end

    # Tests
    describe "usage: " do
        usage = [
            "Example usage:",
            "   dogear recent",
            "   dogear fold [BOOKMARK]",
            "   dogear unfold",
            "   dogear find [BOOKMARK]",
            "   dogear edit",
            "   dogear clean",
            "   dogear help"
        ].freeze

        it "missing command" do
            result = run_script("")
            expect(result).to match_array(usage)
        end

        it "unknown command" do
            result = run_script("unknown-command")
            expect(result).to match_array(usage)
        end
    end

    describe "`recent` command: " do
        it "empty bookmark list" do
            fill_bookmark_list({})
            result = run_script("recent")
            expect(result).to match_array([
                "No bookmarks have been added"
            ])
        end

        it "full bookmark list" do
            fill_bookmark_list({
                first: "first_path",
                second: "second_path",
                third: "third_path"
            })
            result = run_script("recent")
            expect(result).to match_array([
                "Recently Used Bookmarks:",
                "1) `first` -> first_path",
                "2) `second` -> second_path",
                "3) `third` -> third_path"
            ])
        end
    end

    describe "`fold` command: " do
        it "add bookmark" do
            result = run_script("fold nickname")
            expect(result).to match_array([
                "Added bookmark to: `nickname` -> #{Dir.pwd}"
            ])
        end

        it "fail to add bookmarks with invalid names" do
            invalid_name_output = lambda do |name|
                return [
                    "Invalid bookmark name: #{name}",
                    "",
                    "Valid bookmark names must be have 1-40 characters",
                    "and can only contain the following:",
                    "   [a-zA-z0-9] alphanumeric characters",
                    "   [_.-] underscores, periods, and dashes"
                ]
            end

            invalid_names = ["slkdfsd^^^", "", "a" * 45].freeze

            invalid_names.each do |name|
                result = run_script("fold #{name}")
                expect(result).to match_array(
                    invalid_name_output.call(name)
                )
            end
        end

        it "add the same bookmark again" do
            fill_bookmark_list({
                nickname: Dir.pwd
            })
            result = run_script("fold nickname")
            expect(result).to match_array([
                "This directory has already been bookmarked"
            ])
        end

        it "change directory pointed to by bookmark name" do
            fill_bookmark_list({
                nickname: "other_directory"
            })
            result = run_script("fold nickname", ["y"])
            expect(result).to match_array([
                "This name already points to another directory:",
                "   `nickname` -> other_directory",
                "",
                "Would you like to overwrite it (y/n)? " \
                "Overwritten to: `nickname` -> #{Dir.pwd}"
            ])
        end

        it "change bookmark name for current directory" do
            fill_bookmark_list({
                nickname: Dir.pwd
            })
            result = run_script("fold new_name", ["y"])
            expect(result).to match_array([
                "This is already bookmarked under another name:",
                "   `nickname` -> #{Dir.pwd}",
                "",
                "Would you like to change the name (y/n)? " \
                "Overwritten to: `new_name` -> #{Dir.pwd}"
            ])
        end

        it "keep old bookmark name" do
            fill_bookmark_list({
                nickname: Dir.pwd
            })
            result = run_script("fold new_name", ["n"])
            expect(result).to match_array([
                "This is already bookmarked under another name:",
                "   `nickname` -> #{Dir.pwd}",
                "",
                "Would you like to change the name (y/n)? "
            ])
        end
    end

    describe "unfold command: " do
        it "current directory not bookmarked" do
            fill_bookmark_list({})
            result = run_script("unfold")
            expect(result).to match_array([
                "This directory hasn't been bookmarked"
            ])
        end

        it "unfold bookmark" do
            fill_bookmark_list({
                nickname: Dir.pwd
            })
            # Unfold bookmark
            result = run_script("unfold")
            expect(result).to match_array([
                "Unfolded bookmark: nickname"
            ])
            # Check that bookmark list is empty
            result = run_script("recent")
            expect(result).to match_array([
                "No bookmarks have been added"
            ])
        end
    end

    describe "find command: " do
        it "no bookmark with that name exists" do
            fill_bookmark_list({})
            result = run_script("find first")
            expect(result).to match_array([])
        end

        it "named bookmark does exist" do
            fill_bookmark_list({
                first: "first_path",
                second: "second_path",
                third: "third_path"
            })
            result = run_script("find first")
            expect(result).to match_array(["first_path"])
        end
    end

    describe "edit command: " do
        it "no bookmarks to edit" do
            fill_bookmark_list({})
            result = run_script("edit")
            expect(result).to match_array([
                "There are no bookmarks to edit"
            ])
        end

        it "don't delete anything" do
            fill_bookmark_list({
                first: "first_path",
                second: "second_path",
                third: "third_path"
            })
            # Look at but don't delete any bookmarks
            result = run_script("edit", [
                "n",
                "sdfkjlsdjfsdkfj",
                "y     "
            ])
            expect(result).to match_array([
                "Editing Bookmarks:",
                "-Note: Press (q) at any time to quit",
                "",
                "Bookmark: `first` -> first_path",
                "Delete this bookmark (y/n)? ",
                "Bookmark: `second` -> second_path",
                "Delete this bookmark (y/n)? ",
                "Bookmark: `third` -> third_path",
                "Delete this bookmark (y/n)? "
            ])
            # Check to see that they're all still there
            result = run_script("recent")
            expect(result).to match_array([
                "Recently Used Bookmarks:",
                "1) `first` -> first_path",
                "2) `second` -> second_path",
                "3) `third` -> third_path"
            ])
        end

        it "quit early" do
            fill_bookmark_list({
                first: "first_path",
                second: "second_path",
                third: "third_path"
            })
            # Quit editing session immediately
            run_script("edit", ["q"])
            # Check to see that they're all still there
            result = run_script("recent")
            expect(result).to match_array([
                "Recently Used Bookmarks:",
                "1) `first` -> first_path",
                "2) `second` -> second_path",
                "3) `third` -> third_path"
            ])
        end

        it "delete all of them" do
            fill_bookmark_list({
                first: "first_path",
                second: "second_path",
                third: "third_path"
            })
            # Delete all of them
            result = run_script("edit", ["y"] * 3)
            expect(result).to match_array([
                "Editing Bookmarks:",
                "-Note: Press (q) at any time to quit",
                "",
                "Bookmark: `first` -> first_path",
                "Delete this bookmark (y/n)? " \
                "`first` has been deleted",
                "",
                "Bookmark: `second` -> second_path",
                "Delete this bookmark (y/n)? " \
                "`second` has been deleted",
                "",
                "Bookmark: `third` -> third_path",
                "Delete this bookmark (y/n)? " \
                "`third` has been deleted",
            ])
            # Check to see that they're all gone
            result = run_script("recent")
            expect(result).to match_array([
                "No bookmarks have been added"
            ])
        end
    end

    describe "clean command: " do
        it "empty bookmark list" do
            fill_bookmark_list({})
            result = run_script("clean")
            expect(result).to match_array([
                "No bookmarks to clean"
            ])
        end

        it "clean list of bookmarks to invalid directories" do
            fill_bookmark_list({
                first: "first_path",
                second: "second_path",
                third: "third_path"
            })
            # Remove invalid bookmarks
            result = run_script("clean")
            expect(result).to match_array([
                "Cleaned invalid bookmarks from bookmark list"
            ])
            # Check to see that they're all gone
            result = run_script("recent")
            expect(result).to match_array([
                "No bookmarks have been added"
            ])
        end
    end

    # Cleanup
    after(:all) do
        `mv -f ~/.dogear_store.tmp ~/.dogear_store`
    end
end