
# Used with the dogear app to allow you to switch
# to bookmarked directories.
#
# For more information: 
# Github link

flipto () {
    if [ "$#" -lt 1 ] || [ "$1" = "help" ]
    then
        echo "Usage: flipto <bookmark>"
        echo "For more information type:"
        echo "   dogear help"
    else
        bookmark=$(dogear find "$1")
        if [ "$bookmark" ]
        then
            cd "$bookmark" || exit
        else
            echo "Unable to flip to bookmark: $1"
        fi
    fi
}