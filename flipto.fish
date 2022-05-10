# Used with the dogear app to allow you to switch
# to bookmarked directories.
#
# For more information: 
# https://github.com/apainintheneck/dogear

function flipto
    if test (count $argv) -lt 1; or test $argv[1] = "help"
        echo "Usage: flipto <bookmark> [subdirectory]"
        echo "For more information type:"
        echo "   dogear help"
    else
        set bookmark $(dogear find "$argv[1]")
        if test "$bookmark"
            cd "$bookmark"
            if test $status = 0 ; and test (count $argv) -gt 1
                cd "$argv[2]"
            end
        else
            echo "Unable to flip to bookmark: $argv[1]"
        end
    end
end
