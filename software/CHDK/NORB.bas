@title NORB Movie Code
@param x minutes per file
@default x 2
press "shoot_half"  
sleep 2000 
release "shoot_half"
set_backlight(0)
sleep 500
set_backlight(0)
sleep 500

:loop
    print "segment started..."
    press "video" 
    sleep 2000
    release "video"
    s=get_tick_count 
    do 
        sleep 500 
    until x<=((get_tick_count-s)/60000 )
    press "video" 
    sleep 2000
    release "video"
    do 
        sleep 100 
    until get_movie_status=6
    sleep 1000
goto "loop"
end
