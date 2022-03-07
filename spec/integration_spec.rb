RSpec.describe "integration tests" do
    before do
        `make`
        `mv ~/.dogear_store ~/.dogear_store.tmp`
    end

    def run_script(cmd, args=nil)
        raw_output = nil
        IO.popen("./build/apps/dogear recent", "r+") do |pipe|
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

    it "test `recent` on empty bookmark list" do
        result = run_script("recent")
        expect(result).to match_array(["Recently Used Bookmarks:"])
    end

    after do
        `mv -f ~/.dogear_store.tmp ~/.dogear_store`
    end
end