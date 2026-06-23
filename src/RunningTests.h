#pragma once
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_COLORS_ANSI
#include "doctest/doctest.h"

int runTests(int argc, char** argv, bool &out_shouldExit)
{
    out_shouldExit = false;

    doctest::Context ctx;
    ctx.setOption("abort-after", 5);    // default - stop after 5 failed asserts
    ctx.applyCommandLine(argc, argv);   // apply command line - argc / argv
    ctx.setOption("no-breaks", true); // override - don't break in the debugger
    int res = ctx.run();                // run test cases unless with --no-run

    if(ctx.shouldExit())                // query flags (and --exit) rely on this
    {
        out_shouldExit = true;
    }

    return res; // + your_program_res
}