
# Used with the dogear app to allow you to switch
# to bookmarked directories.
#
# For more information: 
# https://github.com/apainintheneck/dogear

flipto () {
    if [ "$#" -lt 1 ] || [ "$1" = "help" ]
    then
        echo "Usage: flipto <bookmark> [subdirectory]"
        echo "For more information type:"
        echo "   dogear help"
    else
        bookmark=$(dogear find "$1")
        if [ "$bookmark" ]
        then
            cd "$bookmark"
            if [ "$?" = 0 ] && [ "$#" -gt 1 ]
            then
                cd "$2"
            fi
        else
            echo "Unable to flip to bookmark: $1"
        fi
    fi
}