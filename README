# CMocka-branches:
See test_forks for unittests/example using the API.
CMocka forks is inspired by https://github.com/philsquared/Catch framework for c++.
The cmocka forks feature adds support for running a unittest multiple times to simplify testing of branch points in the code for example due to user input or error handling.
### Example
(loosely based on the bluetooth 5 phy change procedure at https://www.bluetooth.org/DocMan/handlers/DownloadDoc.ashx?doc_id=421043 page 2759):
all tests that uses branches must use wrapped macros, for example cmocka_unit_test_teardown_twigs (see cmocka_branches.h). 
See tests/test_branches.c for a complete example and test.


```c
/* Example/demo unittest from documentation, does not actually assert anything*/
static void phy_change_test(void **state)
{
    static char const * const  ch_rsp_names[] = {"change_to_2mbps", "change to coded", "no change"};
    static char const * const  ch_rsp_error_code_2m[]  = {"success",
                                                          "destination phy not supported"};
    static char const * const  ch_rsp_error_code_cd[]  = {"success",
                                                          "destination phy not supported",
                                                          "not enough time",
                                                          "<not entered>"};
    static char const * const  ch_rsp_error_code_dle[] = {"larger packets",
                                                          "smaller packets",
                                                          "same size packets",
                                                          "no packet size change",
                                                          "<not entered>"};
    unsigned int ch_rsp_phy, ch_rsp_errcode, dle_rsp_errcode;
    ch_rsp_phy = branch_start_count("change_response_phy", 3, ch_rsp_names);
    /* Set default values here for use by print later */
    ch_rsp_errcode = 3;
    dle_rsp_errcode = 4;
    switch(ch_rsp_phy)
    {
        case 0: /* change_to_2mbps */
            ch_rsp_errcode = branch_start_count("ch_rsp_error_code", 2, ch_rsp_error_code_2m);
            /* Do some validation here */
            branch_end_named("ch_rsp_error_code");
            dle_rsp_errcode = branch_start_count("data length procedure response error code", 4, ch_rsp_error_code_dle);
            /* Do some other validation here */
            branch_end_named("data length procedure response error code");
            break;
        case 1: /* change_to_coded */
            ch_rsp_errcode = branch_start_count("ch_rsp_error_code_cd", 3, ch_rsp_error_code_cd);
            /* Do some validation here */
            branch_end_named("ch_rsp_error_code_cd");
            break;
        case 2: /* no_change */
            ch_rsp_errcode = branch_start_count("ch_rsp_error_code", 2, NULL);
            /* Do some validation here */
            branch_end_named("ch_rsp_error_code");
            break;
    }
    branch_end_named("change_response_phy");
    printf("Phy change test: Phy: %s (%u),\tCode: %s (%u), \tDle: %s (%u)\n",
            ch_rsp_names[ch_rsp_phy], ch_rsp_phy,
            ch_rsp_error_code_cd[ch_rsp_errcode], ch_rsp_errcode,
            ch_rsp_error_code_dle[dle_rsp_errcode], dle_rsp_errcode);
    (void)state;
}
```

In this test case the test will be ran 13 times:

| Phy                  | Code                                | DLE                       |
|----------------------|-------------------------------------|---------------------------|
| change_to_2mbps (0), |  success (0),                       | larger packets (0)        |
| change_to_2mbps (0), |  success (0),                       | smaller packets (1)       |
| change_to_2mbps (0), |  success (0),                       | same size packets (2)     |
| change_to_2mbps (0), |  success (0),                       | no packet size change (3) |
| change_to_2mbps (0), |  destination phy not supported (1), | larger packets (0)        |
| change_to_2mbps (0), |  destination phy not supported (1), | smaller packets (1)       |
| change_to_2mbps (0), |  destination phy not supported (1), | same size packets (2)     |
| change_to_2mbps (0), |  destination phy not supported (1), | no packet size change (3) |
| change to coded (1), |  success (0),                       | <not entered> (4)         |
| change to coded (1), |  destination phy not supported (1), | <not entered> (4)         |
| change to coded (1), |  not enough time (2),               | <not entered> (4)         |
| no change (2),       |  success (0),                       | <not entered> (4)         |
| no change (2),       |  destination phy not supported (1), | <not entered> (4)         |

All combinations of fork values will be ran and thus tested. The test runner will change the values of the forks from the innermost fork to the outermost, from the first to the last fork (in case there are multiple forks in the same nesting level). The test setup and teardown function will only be ran once, setup before the first run of the test and teardown after the last. If one fork results in an error the whole test will be aborted.

