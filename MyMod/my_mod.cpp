#include "my_mod.h"

int MyMod::requireAdmin(){
    return 0;
}

void MyMod::start(){
    wprintf(L"Starting MyMod\n");
}

void MyMod::kill(){
    wprintf(L"Killing MyMod\n");
}