g++ -std=c++20 -Ofast -flto -fno-exceptions -static -s -mpopcnt -"msse4.2" -mbmi2 -DUSE_BMI2 src/Chess.cpp -o build/Jimbo-win-bmi2.exe
g++ -std=c++20 -Ofast -flto -fno-exceptions -static -s -mpopcnt -"msse4.2" src/Chess.cpp -o build/Jimbo-win-popcnt.exe
g++ -std=c++20 -Ofast -flto -fno-exceptions -static -s src/Chess.cpp -o build/Jimbo-win-x64.exe