#include "fuji-sandbox.h"
#include "fuji.h"
#include "fmt/core.h"



void fuji_sandbox(){
    #ifdef NDEBUG
    fmt::print("fuji-sandbox/0.1: Release Build\n");
    #else
    fmt::print("fuji-sandbox/0.1: Debug Build\n");
    #endif

    fuji();
}
