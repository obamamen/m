#include <stdio.h>
#include <string.h>

#include <m/m.h>
#include <mtest/mtest.h>
#include <mtest/mtest_terminal.h>


int main(void)
{
    mtest_terminal_ctx ctx;
    mtest t = mtest_new(mtest_terminal_new(&ctx, MTEST_TERMINAL_DEFAULT));

    return mtest_terminal_summary(&t);
}