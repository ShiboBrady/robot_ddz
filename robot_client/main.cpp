#include <iostream>
#include "NetLib.h"

int main(int argc, char** argv)
{
    NetLib netLib("127.0.0.1", 9999, 10);
    netLib.start();
    return 0;
}

