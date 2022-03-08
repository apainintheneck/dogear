RSpec.describe "integration tests: " do
    # Setup
    before(:all) do
        `make`
        `mv ~/.dogear_store ~/.dogear_store.tmp`
    end

    # Helpers
    def run_script(cmd, args=nil)
        raw_output = nil
        IO.popen("./build/apps/dogear #{cmd}", "r+") do |pipe|
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
        raw_output.split("\n")
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
    describe "unknown or missing command: " do
        help_output = [
            "",
            "[dogear]",
            "--------",
            "Bookmark directories for easy access in the future.",
            "",
            "To change to a bookmarked directory type:",
            "`flipto (<bookmark name>)`",
            "",
            "",
            "[dogear subcommands]",
            "--------------------",
            "`dogear` [`help`]:",
            "Displays this page.",
            "",
            "`dogear recent`:",
            "Displays the 10 most recently accessed bookmarks.",
            "",
            "`dogear fold` (<bookmark name>):",
            "Creates a bookmark of the current working directory.",
            "",
            "`dogear unfold`:",
            "Removes the bookmark of the current working directory.",
            "",
            "`dogear find` (<bookmark name>):",
            "Returns the path associated with the bookmark or an empty string.",
            "",
            "`dogear edit`:",
            "Allows you to edit your bookmarks one by one.",
            "",
            "`dogear clean`:",
            "Removes bookmarks pointing to nonexistent directories.",
            "",
            "",
            "[more information]",
            "------------------",
            "Bookmarks are stored in `~/dogear_store` file.",
            "Project can be found here: (Github Link)"
        ]

        it "no command" do
            result = run_script("")
            expect(result).to match_array(help_output)
        end 

        it "unknown command" do
            result = run_script("unknown-command")
            expect(result).to match_array(help_output)
        end
    end

    describe "`recent` command: " do
        it "empty bookmark list" do
            result = run_script("recent")
            expect(result).to match_array(["Recently Used Bookmarks:"])
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

    # Cleanup
    after(:all) do
        `mv -f ~/.dogear_store.tmp ~/.dogear_store`
    end
end