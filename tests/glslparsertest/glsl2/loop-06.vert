/* PASS - the while loop has a single statement: the if block. */
uniform bool condition;
void main()
{
    while (condition)
        if (condition)
            continue;
}
