:Loop

adb reverse tcp:8081 tcp:8081
call npm run start

goto :Loop
