mkdir build
cd build
clang++ ../main.cpp ../types.h ../bitboard.h ../position.h ../position.cpp ../board.h ../board.cpp -O3
.\a.exe
cd ../