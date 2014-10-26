// Demonstate constant data
char *ReturnOne(void)
{
    return "One";
}


// Demonstate non-constant initialized data
static char two[] = "Two0";
char *ReturnTwo(void)
{
    two[3]++;
    return two;
}
